// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "mdt_dialout_core.h"
#include "../utils/peer_parser.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <vector>
#include <zmq.hpp>


std::unordered_map<std::string, std::vector<std::string>> label_map;
std::shared_mutex                                         label_map_mutex;
std::atomic<bool>                                         label_map_reload_pending{false};

namespace {
std::mutex            g_servers_mutex;
std::vector<Srv *>    g_active_servers;
}  // namespace

void register_server(Srv *srv)
{
    std::lock_guard<std::mutex> lk(g_servers_mutex);
    g_active_servers.push_back(srv);
}

void unregister_server(Srv *srv)
{
    std::lock_guard<std::mutex> lk(g_servers_mutex);
    auto it = std::find(g_active_servers.begin(), g_active_servers.end(), srv);
    if (it != g_active_servers.end()) {
        g_active_servers.erase(it);
    }
}

void initiate_shutdown()
{
    // Snapshot then release: Server::Shutdown blocks for the drain deadline.
    std::vector<Srv *> snapshot;
    {
        std::lock_guard<std::mutex> lk(g_servers_mutex);
        snapshot = g_active_servers;
    }
    for (Srv *s : snapshot) {
        s->Shutdown();
    }
}

void Srv::Shutdown()
{
    if (shutdown_done_.exchange(true)) {
        return;
    }
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
    switch (active_vendor_) {
    case Vendor::Cisco:
        if (cisco_server_) cisco_server_->Shutdown(deadline);
        if (cisco_cq_)     cisco_cq_->Shutdown();
        break;
    case Vendor::Juniper:
        if (juniper_server_) juniper_server_->Shutdown(deadline);
        if (juniper_cq_)     juniper_cq_->Shutdown();
        break;
    case Vendor::Nokia:
        if (nokia_server_) nokia_server_->Shutdown(deadline);
        if (nokia_cq_)     nokia_cq_->Shutdown();
        break;
    case Vendor::Huawei:
        if (huawei_server_) huawei_server_->Shutdown(deadline);
        if (huawei_cq_)     huawei_cq_->Shutdown();
        break;
    case Vendor::Unset:
        break;
    }
}

void CustomSocketMutator::log_socket_options(int fd) {
    int type, keepalive, reuseport;
    socklen_t len = sizeof(type);

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) == 0) {
        std::string type_str;
        switch (type) {
            case SOCK_STREAM:
                type_str = "SOCK_STREAM (TCP)";
                break;
            case SOCK_DGRAM:
                type_str = "SOCK_DGRAM (UDP)";
                break;
            case SOCK_RAW:
                type_str = "SOCK_RAW (Raw)";
                break;
            default:
                type_str = "Unknown Type";
                break;
        }
        spdlog::get("multi-logger")->debug("Socket Type: {}", type_str);
    } else {
        spdlog::get("multi-logger")->error("getsockopt(SO_TYPE) failed");
    }

    if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, &len) == 0) {
        spdlog::get("multi-logger")->
            debug("SO_KEEPALIVE: {}", keepalive ? "Enabled" : "Disabled");
    } else {
        spdlog::get("multi-logger")->error("getsockopt(SO_KEEPALIVE) failed");
    }

    if (getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuseport, &len) == 0) {
        spdlog::get("multi-logger")->
            debug("SO_REUSEPORT: {}", reuseport ? "Enabled" : "Disabled");
    } else {
        spdlog::get("multi-logger")->error("getsockopt(SO_REUSEPORT) failed");
    }
}

bool CustomSocketMutator::bindtodevice_socket_mutator(int fd)
{
    int type;
    socklen_t len = sizeof(type);

    const std::string iface = main_cfg_parameters.at("iface");
    const std::string sbc = main_cfg_parameters.at("so_bindtodevice_check");

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) != 0) {
        spdlog::get("multi-logger")->
            error("[CustomSocketMutator()]: Unable to get the "
            "interface type");
        std::abort();
    }

    if (sbc.compare("true") == 0) {
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
            iface.c_str(), strlen(iface.c_str())) != 0) {
            spdlog::get("multi-logger")->
                error("[CustomSocketMutator()]: Unable to bind [{}] "
                "on the configured socket(s)", iface);
            std::abort();
        }
    } else {
        spdlog::get("multi-logger")->
            warn("[CustomSocketMutator()]: SO_BINDTODEVICE "
            "check disabled");
    }

    log_socket_options(fd);

    return true;
}

#define GPR_ICMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))
int custom_socket_compare(grpc_socket_mutator *mutator1,
    grpc_socket_mutator *mutator2)
{
    return GPR_ICMP(mutator1, mutator2);
}

void custom_socket_destroy(grpc_socket_mutator *mutator)
{
    // Pairs with placement new in ServerBuilderOptionImpl::UpdateArguments.
    static_cast<CustomSocketMutator *>(mutator)->~CustomSocketMutator();
    gpr_free(mutator);
}

bool custom_socket_mutator_fd(int fd, grpc_socket_mutator *mutator0)
{
    CustomSocketMutator *csm = (CustomSocketMutator *) mutator0;
    return csm->bindtodevice_socket_mutator(fd);
}

const grpc_socket_mutator_vtable custom_socket_mutator_vtable =
    grpc_socket_mutator_vtable {
        custom_socket_mutator_fd,
        custom_socket_compare,
        custom_socket_destroy,
        nullptr
    };

CustomSocketMutator::CustomSocketMutator()
{
    spdlog::get("multi-logger")->debug("constructor: CustomSocketMutator()");
    grpc_socket_mutator_init(this, &custom_socket_mutator_vtable);
}

void ServerBuilderOptionImpl::UpdateArguments(
    grpc::ChannelArguments *custom_args)
{
    // gpr_malloc + placement new so custom_socket_destroy's gpr_free matches.
    auto *csm_ = static_cast<CustomSocketMutator *>(
        gpr_malloc(sizeof(CustomSocketMutator)));
    new (csm_) CustomSocketMutator();
    custom_args->SetSocketMutator(csm_);
}

namespace {
std::string slurp(const std::string &path)
{
    std::ifstream f(path);
    if (!f) {
        // cfg_handler validates exists() at startup; reaching here means
        // the file became unreadable between validation and load.
        spdlog::get("multi-logger")->error(
            "[slurp] cannot open '{}'", path);
        std::abort();
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::shared_ptr<grpc::ServerCredentials> make_server_credentials()
{
    if (main_cfg_parameters.at("enable_tls") != "true") {
        return grpc::InsecureServerCredentials();
    }
    grpc::SslServerCredentialsOptions opts;
    opts.pem_key_cert_pairs.push_back({
        slurp(main_cfg_parameters.at("tls_key_path")),
        slurp(main_cfg_parameters.at("tls_cert_path")),
    });
    return grpc::SslServerCredentials(opts);
}

void log_thread_id(const char *tag)
{
    auto tid = std::this_thread::get_id();
    std::stringstream stid;
    stid << tid;
    spdlog::get("multi-logger")->debug("{} - Thread-ID: {}", tag, stid.str());
}

// Per-vendor FsmCtrl plumbing: kafka + zmq sides constructed once, the
// active one chosen by data_delivery_method. Lives on the FsmCtrl stack
// so its handles outlive every spawned Stream.
struct VendorPlumbing {
    DataManipulation data_manipulation;
    DataWrapper      data_wrapper;
    KafkaDelivery    kafka_delivery;
    ZmqPush          zmq_pusher;
    std::string      zmq_uri;
    std::optional<kafka::clients::KafkaProducer> kafka_producer;
    std::optional<zmq::socket_t>                 zmq_sock;
    DeliveryMethod   delivery;
    bool             label_encode;
    std::string      writer_id;

    VendorPlumbing()
        : zmq_uri(zmq_pusher.get_zmq_transport_uri()),
          writer_id(main_cfg_parameters.at("writer_id"))
    {
        const std::string ddm = main_cfg_parameters.at("data_delivery_method");
        if (ddm == "kafka") {
            kafka_producer.emplace(kafka_delivery.get_properties());
            delivery = DeliveryMethod::Kafka;
        } else if (ddm == "zmq") {
            zmq_sock.emplace(zmq_pusher.get_zmq_ctx(), zmq::socket_type::push);
            zmq_sock->connect(zmq_uri);
            delivery = DeliveryMethod::Zmq;
        } else {
            spdlog::get("multi-logger")->error(
                "[VendorPlumbing] unknown data_delivery_method: {}", ddm);
            std::abort();
        }
        label_encode =
            data_manipulation_cfg_parameters.at(
                "enable_label_encode_as_map") == "true" ||
            data_manipulation_cfg_parameters.at(
                "enable_label_encode_as_map_ptm") == "true";
    }
};

// Drive a vendor's FSM: arm the first handler, then loop over completion
// queue events forever (until cq is shut down).
template <typename Stream, typename Service>
void RunFsm(Service *svc, grpc::ServerCompletionQueue *cq,
    typename Stream::Context *ctx, const char *log_tag)
{
    Stream::Spawn(svc, cq, ctx);
    void *tag {nullptr};
    bool ok {false};
    while (true) {
        if (!cq->Next(&tag, &ok)) {
            spdlog::get("multi-logger")->info(
                "[{}] completion queue shut down, exiting", log_tag);
            break;
        }
        static_cast<Stream *>(tag)->Proceed(ok);
    }
}

// Per-message delivery: builds the envelope (kafka path) or wraps the
// raw body (zmq path) and dispatches via the active producer/socket.
// Templated on the per-vendor Context — duck-typed access to the
// shared field set (delivery, writer_id, label_encode, kafka_*, zmq_*,
// data_manipulation, data_wrapper, kafka_delivery, zmq_pusher).
template <typename CtxT>
void deliver_payload(CtxT *ctx,
    std::string &body,
    const std::string &peer_ip,
    const std::string &peer_port,
    std::unordered_map<std::string, std::vector<std::string>> &label_map)
{
    if (ctx->delivery == DeliveryMethod::Kafka) {
        std::string envelope;
        if (!ctx->data_manipulation->BuildEnvelope(body, peer_ip, peer_port,
                ctx->label_encode ? &label_map : nullptr,
                ctx->writer_id, envelope)) {
            return;
        }
        ctx->kafka_delivery->AsyncKafkaProducer(*ctx->kafka_producer,
            peer_ip, envelope);
    } else if (ctx->delivery == DeliveryMethod::Zmq) {
        ctx->data_wrapper->BuildDataWrapper(
            "gRPC", "json_string", ctx->writer_id,
            peer_ip, peer_port, body);
        ctx->zmq_pusher->ZmqPusher(*ctx->data_wrapper, *ctx->zmq_sock,
            ctx->zmq_pusher->get_zmq_transport_uri());
    }
}
}  // namespace

void Srv::CiscoBind(std::string cisco_srv_socket)
{
    active_vendor_ = Vendor::Cisco;
    grpc::ServerBuilder cisco_builder;
    std::unique_ptr<ServerBuilderOptionImpl>
        csbo(new ServerBuilderOptionImpl());
    cisco_builder.SetOption(std::move(csbo));
    cisco_builder.RegisterService(&cisco_service_);
    cisco_builder.AddListeningPort(cisco_srv_socket,
        make_server_credentials());
    cisco_cq_ = cisco_builder.AddCompletionQueue();
    cisco_server_ = cisco_builder.BuildAndStart();

    register_server(this);
    Srv::CiscoFsmCtrl();
    unregister_server(this);
}

void Srv::JuniperBind(std::string juniper_srv_socket)
{
    active_vendor_ = Vendor::Juniper;
    grpc::ServerBuilder juniper_builder;
    std::unique_ptr<ServerBuilderOptionImpl>
        jsbo(new ServerBuilderOptionImpl());
    juniper_builder.SetOption(std::move(jsbo));
    juniper_builder.RegisterService(&juniper_service_);
    juniper_builder.AddListeningPort(juniper_srv_socket,
        make_server_credentials());
    juniper_cq_ = juniper_builder.AddCompletionQueue();
    juniper_server_ = juniper_builder.BuildAndStart();

    register_server(this);
    Srv::JuniperFsmCtrl();
    unregister_server(this);
}

void Srv::NokiaBind(std::string nokia_srv_socket)
{
    active_vendor_ = Vendor::Nokia;
    grpc::ServerBuilder nokia_builder;
    std::unique_ptr<ServerBuilderOptionImpl>
        jsbo(new ServerBuilderOptionImpl());
    nokia_builder.SetOption(std::move(jsbo));
    nokia_builder.RegisterService(&nokia_service_);
    nokia_builder.AddListeningPort(nokia_srv_socket,
        make_server_credentials());
    nokia_cq_ = nokia_builder.AddCompletionQueue();
    nokia_server_ = nokia_builder.BuildAndStart();

    register_server(this);
    Srv::NokiaFsmCtrl();
    unregister_server(this);
}

void Srv::HuaweiBind(std::string huawei_srv_socket)
{
    active_vendor_ = Vendor::Huawei;
    grpc::ServerBuilder huawei_builder;
    std::unique_ptr<ServerBuilderOptionImpl>
        hsbo(new ServerBuilderOptionImpl());
    huawei_builder.SetOption(std::move(hsbo));
    huawei_builder.RegisterService(&huawei_service_);
    huawei_builder.AddListeningPort(huawei_srv_socket,
        make_server_credentials());
    huawei_cq_ = huawei_builder.AddCompletionQueue();
    huawei_server_ = huawei_builder.BuildAndStart();

    register_server(this);
    Srv::HuaweiFsmCtrl();
    unregister_server(this);
}

void Srv::CiscoFsmCtrl()
{
    log_thread_id("Srv::CiscoFsmCtrl()");
    VendorPlumbing pl;
    CiscoStream::Context ctx{
        &label_map, &pl.data_manipulation, &pl.data_wrapper,
        &pl.kafka_delivery,
        pl.kafka_producer ? &*pl.kafka_producer : nullptr,
        &pl.zmq_pusher,
        pl.zmq_sock ? &*pl.zmq_sock : nullptr,
        pl.zmq_uri,
        std::stoi(main_cfg_parameters.at("replies_cisco")),
        pl.delivery, pl.writer_id, pl.label_encode,
        data_manipulation_cfg_parameters.at(
            "enable_cisco_gpbkv2json") == "true",
        data_manipulation_cfg_parameters.at(
            "enable_cisco_message_to_json_string") == "true",
    };
    RunFsm<CiscoStream>(&cisco_service_, cisco_cq_.get(), &ctx,
        "CiscoFsmCtrl");
}

void Srv::JuniperFsmCtrl()
{
    log_thread_id("Srv::JuniperFsmCtrl()");
    VendorPlumbing pl;
    JuniperStream::Context ctx{
        &label_map, &pl.data_manipulation, &pl.data_wrapper,
        &pl.kafka_delivery,
        pl.kafka_producer ? &*pl.kafka_producer : nullptr,
        &pl.zmq_pusher,
        pl.zmq_sock ? &*pl.zmq_sock : nullptr,
        pl.zmq_uri,
        std::stoi(main_cfg_parameters.at("replies_juniper")),
        pl.delivery, pl.writer_id, pl.label_encode,
    };
    RunFsm<JuniperStream>(&juniper_service_, juniper_cq_.get(), &ctx,
        "JuniperFsmCtrl");
}

void Srv::NokiaFsmCtrl()
{
    log_thread_id("Srv::NokiaFsmCtrl()");
    VendorPlumbing pl;
    NokiaStream::Context ctx{
        &label_map, &pl.data_manipulation, &pl.data_wrapper,
        &pl.kafka_delivery,
        pl.kafka_producer ? &*pl.kafka_producer : nullptr,
        &pl.zmq_pusher,
        pl.zmq_sock ? &*pl.zmq_sock : nullptr,
        pl.zmq_uri,
        std::stoi(main_cfg_parameters.at("replies_nokia")),
        pl.delivery, pl.writer_id, pl.label_encode,
    };
    RunFsm<NokiaStream>(&nokia_service_, nokia_cq_.get(), &ctx,
        "NokiaFsmCtrl");
}

void Srv::HuaweiFsmCtrl()
{
    log_thread_id("Srv::HuaweiFsmCtrl()");
    VendorPlumbing pl;
    HuaweiStream::Context ctx{
        &label_map, &pl.data_manipulation, &pl.data_wrapper,
        &pl.kafka_delivery,
        pl.kafka_producer ? &*pl.kafka_producer : nullptr,
        &pl.zmq_pusher,
        pl.zmq_sock ? &*pl.zmq_sock : nullptr,
        pl.zmq_uri,
        std::stoi(main_cfg_parameters.at("replies_huawei")),
        pl.delivery, pl.writer_id, pl.label_encode,
    };
    RunFsm<HuaweiStream>(&huawei_service_, huawei_cq_.get(), &ctx,
        "HuaweiFsmCtrl");
}

Srv::CiscoStream::CiscoStream(
    mdt_dialout::gRPCMdtDialout::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx) :
        svc_       {svc},
        cq_        {cq},
        ctx_       {ctx},
        resp_      {&server_ctx_},
        state_     {State::Create},
        replies_   {0}
{
    spdlog::get("multi-logger")->debug("constructor: CiscoStream()");
    svc_->RequestMdtDialout(&server_ctx_, &resp_, cq_, cq_, this);
}

void Srv::CiscoStream::Spawn(
    mdt_dialout::gRPCMdtDialout::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx)
{
    new CiscoStream(svc, cq, ctx);
}

Srv::JuniperStream::JuniperStream(
    Subscriber::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx) :
        svc_     {svc},
        cq_      {cq},
        ctx_     {ctx},
        resp_    {&server_ctx_},
        state_   {State::Create},
        replies_ {0}
{
    spdlog::get("multi-logger")->debug("constructor: JuniperStream()");
    svc_->RequestDialOutSubscriber(&server_ctx_, &resp_, cq_, cq_, this);
}

void Srv::JuniperStream::Spawn(
    Subscriber::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx)
{
    new JuniperStream(svc, cq, ctx);
}

Srv::NokiaStream::NokiaStream(
    Nokia::SROS::DialoutTelemetry::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx) :
        svc_     {svc},
        cq_      {cq},
        ctx_     {ctx},
        resp_    {&server_ctx_},
        state_   {State::Create},
        replies_ {0}
{
    spdlog::get("multi-logger")->debug("constructor: NokiaStream()");
    svc_->RequestPublish(&server_ctx_, &resp_, cq_, cq_, this);
}

void Srv::NokiaStream::Spawn(
    Nokia::SROS::DialoutTelemetry::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx)
{
    new NokiaStream(svc, cq, ctx);
}

Srv::HuaweiStream::HuaweiStream(
    huawei_dialout::gRPCDataservice::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx) :
        svc_     {svc},
        cq_      {cq},
        ctx_     {ctx},
        resp_    {&server_ctx_},
        state_   {State::Create},
        replies_ {0}
{
    spdlog::get("multi-logger")->debug("constructor: HuaweiStream()");
    svc_->RequestdataPublish(&server_ctx_, &resp_, cq_, cq_, this);
}

void Srv::HuaweiStream::Spawn(
    huawei_dialout::gRPCDataservice::AsyncService *svc,
    grpc::ServerCompletionQueue *cq,
    Context *ctx)
{
    new HuaweiStream(svc, cq, ctx);
}

void Srv::CiscoStream::Proceed(bool ok)
{
    switch (state_) {
    case State::Create:
        if (!ok) {
            delete this;
            return;
        }
        Spawn(svc_, cq_, ctx_);
        state_ = State::Read;
        resp_.Read(&request_, this);
        return;

    case State::Read:
        if (!ok) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        HandleMessage();
        ++replies_;
        if (ctx_->max_replies > 0 && replies_ >= ctx_->max_replies) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        resp_.Read(&request_, this);
        return;

    case State::Finishing:
    case State::Done:
        delete this;
        return;
    }
}

void Srv::CiscoStream::HandleMessage()
{
    std::shared_lock<std::shared_mutex> _label_lock(label_map_mutex);

    auto &label_map         = *ctx_->label_map;
    auto &data_manipulation = *ctx_->data_manipulation;
    auto &cisco_tlm         = tlm_;
    auto &cisco_stream      = request_;

    std::string stream_data_in;
    std::string stream_data_in_normalization;
    const PeerEndpoint peer = ParsePeer(server_ctx_.peer());
    const std::string &peer_ip   = peer.ip;
    const std::string &peer_port = peer.port;
    // ParseFromString returns true for GPB-KV/GPB, false for JSON payloads.
    bool parsing_str = cisco_tlm.ParseFromString(cisco_stream.data());

    stream_data_in = cisco_stream.data();

    if (stream_data_in.empty()) {
        spdlog::get("multi-logger")->
            info("[CiscoStream::Start()] {} handling empty data", peer_ip);
    } else if (!cisco_tlm.data_gpbkv().empty() && parsing_str) {
        spdlog::get("multi-logger")->
            info("[CiscoStream::Start()] {} handling GPB-KV data", peer_ip);
        if (ctx_->cisco_gpbkv2json) {
            if (data_manipulation.CiscoGpbkv2Json(cisco_tlm,
                stream_data_in_normalization)) {
                spdlog::get("multi-logger")->
                    info("[CiscoStream::Start()] {} enable_cisco_gpbkv2json, "
                    "data-normalization successful", peer_ip);
                deliver_payload(ctx_, stream_data_in_normalization,
                    peer_ip, peer_port, label_map);
            } else {
                spdlog::get("multi-logger")->
                    error("[CiscoStream::Start()] {} enable_cisco_gpbkv2json, "
                    "data-normalization failure", peer_ip);
            }
        } else if (ctx_->cisco_msg_to_json) {
            // MessageToJsonString appends to existing string.
            stream_data_in.clear();
            google::protobuf::util::JsonPrintOptions opt;
            opt.add_whitespace = false;
            google::protobuf::util::MessageToJsonString(
                cisco_tlm, &stream_data_in, opt);
            deliver_payload(ctx_, stream_data_in, peer_ip, peer_port, label_map);
        } else {
            // both data-manipulation toggles false: TBD, just alert.
            spdlog::get("multi-logger")->
                error("[CiscoStream::Start()] {} general data-manipulation "
                "failure: conflicting manipulation functions", peer_ip);
        }
    } else if (cisco_tlm.has_data_gpb() && parsing_str) {
        spdlog::get("multi-logger")->
            info("[CiscoStream::Start()] {} handling GPB data", peer_ip);
        deliver_payload(ctx_, stream_data_in, peer_ip, peer_port, label_map);
    } else if (!parsing_str) {
        spdlog::get("multi-logger")->
            info("[CiscoStream::Start()] {} handling JSON data", peer_ip);
        deliver_payload(ctx_, stream_data_in, peer_ip, peer_port, label_map);
    }
}

void Srv::JuniperStream::Proceed(bool ok)
{
    switch (state_) {
    case State::Create:
        if (!ok) { delete this; return; }
        Spawn(svc_, cq_, ctx_);
        state_ = State::Read;
        resp_.Read(&request_, this);
        return;
    case State::Read:
        if (!ok) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        HandleMessage();
        ++replies_;
        if (ctx_->max_replies > 0 && replies_ >= ctx_->max_replies) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        resp_.Read(&request_, this);
        return;
    case State::Finishing:
    case State::Done:
        delete this;
        return;
    }
}

void Srv::JuniperStream::HandleMessage()
{
    std::shared_lock<std::shared_mutex> _label_lock(label_map_mutex);

    auto &label_map           = *ctx_->label_map;
    auto &data_manipulation   = *ctx_->data_manipulation;
    auto &juniper_stream      = request_;
    auto &juniper_tlm_hdr_ext = tlm_hdr_ext_;

    std::string json_str_out;
    const PeerEndpoint peer = ParsePeer(server_ctx_.peer());
    const std::string &peer_ip   = peer.ip;
    const std::string &peer_port = peer.port;

    Json::Value root;

    if (data_manipulation.JuniperExtension(juniper_stream,
        juniper_tlm_hdr_ext, root) == true &&
        data_manipulation.JuniperUpdate(juniper_stream, json_str_out,
            root) == true) {
        spdlog::get("multi-logger")->
            info("[JuniperStream::Start()] {} "
            "JuniperExtension, parsing successful", peer_ip);
    } else {
        spdlog::get("multi-logger")->
            error("[JuniperStream::Start()] {} "
            "JuniperExtension, parsing failure", peer_ip);
    }

    deliver_payload(ctx_, json_str_out, peer_ip, peer_port, label_map);
}

void Srv::NokiaStream::Proceed(bool ok)
{
    switch (state_) {
    case State::Create:
        if (!ok) { delete this; return; }
        Spawn(svc_, cq_, ctx_);
        state_ = State::Read;
        resp_.Read(&request_, this);
        return;
    case State::Read:
        if (!ok) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        HandleMessage();
        ++replies_;
        if (ctx_->max_replies > 0 && replies_ >= ctx_->max_replies) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        resp_.Read(&request_, this);
        return;
    case State::Finishing:
    case State::Done:
        delete this;
        return;
    }
}

void Srv::NokiaStream::HandleMessage()
{
    std::shared_lock<std::shared_mutex> _label_lock(label_map_mutex);

    auto &label_map         = *ctx_->label_map;
    auto &data_manipulation = *ctx_->data_manipulation;
    auto &nokia_stream      = request_;

    std::string json_str_out;
    const PeerEndpoint peer = ParsePeer(server_ctx_.peer());
    const std::string &peer_ip   = peer.ip;
    const std::string &peer_port = peer.port;

    Json::Value root;

    if (data_manipulation.NokiaUpdate(nokia_stream, json_str_out,
            root) == true) {
        spdlog::get("multi-logger")->
            info("[NokiaStream::Start()] {} "
            "NokiaExtension, parsing successful", peer_ip);
    } else {
        spdlog::get("multi-logger")->
            error("[NokiaStream::Start()] {} "
            "NokiaExtension, parsing failure", peer_ip);
    }

    deliver_payload(ctx_, json_str_out, peer_ip, peer_port, label_map);
}

void Srv::HuaweiStream::Proceed(bool ok)
{
    switch (state_) {
    case State::Create:
        if (!ok) { delete this; return; }
        Spawn(svc_, cq_, ctx_);
        state_ = State::Read;
        resp_.Read(&request_, this);
        return;
    case State::Read:
        if (!ok) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        HandleMessage();
        ++replies_;
        if (ctx_->max_replies > 0 && replies_ >= ctx_->max_replies) {
            state_ = State::Finishing;
            resp_.Finish(grpc::Status::OK, this);
            return;
        }
        resp_.Read(&request_, this);
        return;
    case State::Finishing:
    case State::Done:
        delete this;
        return;
    }
}

void Srv::HuaweiStream::HandleMessage()
{
    std::shared_lock<std::shared_mutex> _label_lock(label_map_mutex);

    auto &label_map         = *ctx_->label_map;
    auto &data_manipulation = *ctx_->data_manipulation;
    auto &huawei_tlm        = tlm_;
    auto &oc_if             = oc_if_;
    auto &huawei_stream     = request_;

    std::string stream_data_in;
    std::string json_str_out;
    const PeerEndpoint peer = ParsePeer(server_ctx_.peer());
    const std::string &peer_ip   = peer.ip;
    const std::string &peer_port = peer.port;

    bool parsing_str = huawei_tlm.ParseFromString(huawei_stream.data());
    stream_data_in = huawei_stream.data();

    if (stream_data_in.empty()) {
        spdlog::get("multi-logger")->
            info("[HuaweiStream::Start()] {} handling empty data", peer_ip);
    } else if (huawei_tlm.has_data_gpb() && parsing_str &&
        huawei_tlm.proto_path() == "openconfig_interfaces.Interfaces") {
        spdlog::get("multi-logger")->
            info("[HuaweiStream::Start()] {} handling GPB data", peer_ip);

        if (data_manipulation.HuaweiGpbOpenconfigInterface(
            huawei_tlm, oc_if, json_str_out)) {
            spdlog::get("multi-logger")->
                info("[HuaweiStream::Start()] {} "
                "HuaweiGpbOpenconfigInterface, parsing successful", peer_ip);
        } else {
            spdlog::get("multi-logger")->
                error("[HuaweiStream::Start()] {} "
                "HuaweiGpbOpenconfigInterface, parsing failure", peer_ip);
        }

        deliver_payload(ctx_, json_str_out, peer_ip, peer_port, label_map);
    }

    stream_data_in = huawei_stream.data_json();
    if (stream_data_in.empty()) {
        spdlog::get("multi-logger")->
            info("[HuaweiStream::Start()] {} handling empty data_json", peer_ip);
    } else {
        spdlog::get("multi-logger")->
            info("[HuaweiStream::Start()] {} handling JSON data_json", peer_ip);
        deliver_payload(ctx_, stream_data_in, peer_ip, peer_port, label_map);
    }
}

