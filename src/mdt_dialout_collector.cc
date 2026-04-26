// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include <exception>
#include <memory>
#include <thread>
#include <csignal>
#include <fstream>
#include <unistd.h>
#include <zmq.hpp>
#include "csv/rapidcsv.h"
#include "core/mdt_dialout_core.h"


void *ZmqSingleThreadPoller(ZmqPull &zmq_poller,
    zmq::socket_t &zmq_sock);
void *VendorThread(const std::string &vendor);
void LoadThreads(std::vector<std::thread> &workers_vec,
    const std::string &socket_str,
    const std::string &replies_str,
    const std::string &workers_str);
void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv);
void LoadLabelMapPreTagStyle(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv);
bool DumpCorePid(int &core_pid, const std::string &core_pid_path);
void ReloadLabelMaps();
std::string ReadVersionFromFile(const std::string &filePath);

int main(int argc, char *argv[])
{
    std::string cfg_path;

    int opt;
    int f_option_flag = 0;
    int v_option_flag = 0;

    while ((opt = getopt(argc, argv, "Vf:")) != -1) {
        switch (opt) {
        case 'f':
            f_option_flag = 1;
            cfg_path = optarg;
            break;
        case 'V':
            v_option_flag = 1;
            break;
        default:
            std::cout << "Usage: mdt_dialout_collector [-f cfg_path] | [-V]\n";
            return EXIT_FAILURE;
        }
    }

    if (cfg_path.empty() == true) {
        cfg_path = "/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf";
    }

    if ((f_option_flag == 1 and v_option_flag == 1) or (argc > 3)) {
        std::cout << "Usage: mdt_dialout_collector [-f cfg_path] | [-V]\n";
        return EXIT_FAILURE;
    }

    if ((v_option_flag == 1) and (argc == 2)) {
        std::string version = ReadVersionFromFile("../VERSION");
        std::cout << "gRPC dial-out collector, mdt-dialout-collector "
                  << version << "\n";
        return EXIT_SUCCESS;
    } else if ((v_option_flag == 1) and (argc > 2)) {
        std::cout << "Usage: mdt_dialout_collector [-f cfg_path] | [-V]\n";
        return EXIT_FAILURE;
    }

    LogsHandler logs_handler;
    spdlog::get("multi-logger-boot")->debug("main: main()");

    if (f_option_flag == 1 or cfg_path.empty() == false) {
        CfgHandler cfg_handler;
        cfg_handler.set_cfg_path(cfg_path);

        LogsCfgHandler logs_cfg_handler;
        if (logs_cfg_handler.lookup_logs_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_logs_parameters()) == false) {
            // failing here means the destructor logger isn't ready (segfault)
            std::exit(EXIT_FAILURE);
        } else {
            logs_cfg_parameters = cfg_handler.get_logs_parameters();
            logs_handler.set_spdlog_sinks();
        }

        MainCfgHandler main_cfg_handler;
        if (main_cfg_handler.lookup_main_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_main_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            main_cfg_parameters = cfg_handler.get_main_parameters();
        }

        DataManipulationCfgHandler data_manipulation_cfg_handler;
        if (data_manipulation_cfg_handler.lookup_data_manipulation_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_data_manipulation_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            data_manipulation_cfg_parameters =
                cfg_handler.get_data_manipulation_parameters();
        }

        KafkaCfgHandler kafka_cfg_handler;
        if (kafka_cfg_handler.lookup_kafka_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_kafka_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            kafka_delivery_cfg_parameters = cfg_handler.get_kafka_parameters();
        }

        ZmqCfgHandler zmq_cfg_handler;
        if (zmq_cfg_handler.lookup_zmq_parameters(
            "",
            cfg_handler.get_zmq_parameters()) == false) {
            std::exit(EXIT_FAILURE);
        } else {
            zmq_delivery_cfg_parameters = cfg_handler.get_zmq_parameters();
        }
    }

    int core_pid = getpid();
    const std::string core_pid_folder =
        main_cfg_parameters.at("core_pid_folder");
    const std::string core_pid_file = main_cfg_parameters.at("writer_id");
    const std::string core_pid_path = core_pid_folder + "/" + core_pid_file;

    if (DumpCorePid(core_pid, core_pid_path) == false) {
        spdlog::get("multi-logger")->error(
            "unable to dump PID {} to {}", core_pid, core_pid_path);
        return EXIT_FAILURE;
    }

    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map").compare("true") == 0) {
        spdlog::get("multi-logger")->info("Loading label-map-csv data ...");
        LoadLabelMap(label_map,
            data_manipulation_cfg_parameters.at("label_map_csv_path"));
    }
    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map_ptm").compare("true") == 0) {
        spdlog::get("multi-logger")->info("Loading label-map-ptm data ...");
        LoadLabelMapPreTagStyle(label_map,
            data_manipulation_cfg_parameters.at("label_map_ptm_path"));
    }
    spdlog::get("multi-logger")->debug(
        "sockets found in config:\n socket_cisco: {},\n socket_juniper: {},\n socket_nokia: {},\n socket_huawei: {}\n",
        main_cfg_parameters.at("socket_cisco"),
        main_cfg_parameters.at("socket_juniper"),
        main_cfg_parameters.at("socket_nokia"),
        main_cfg_parameters.at("socket_huawei")
    );
    if (main_cfg_parameters.at("socket_cisco").empty() == true &&
        main_cfg_parameters.at("socket_juniper").empty() == true &&
        main_cfg_parameters.at("socket_nokia").empty() == true &&
        main_cfg_parameters.at("socket_huawei").empty() == true) {
            spdlog::get("multi-logger")->
                error("[socket_*] configuration issue: "
                "unable to find at least one valid IPv4 socket where to bind "
                "the daemon");
            return EXIT_FAILURE;
    }

    ZmqPull zmq_poller;
    zmq::socket_t sock_pull(zmq_poller.get_zmq_ctx(),
        zmq::socket_type::pull);
    sock_pull.bind(zmq_poller.get_zmq_transport_uri());
    std::thread zmq_single_thread_poller(
        &ZmqSingleThreadPoller,
        std::ref(zmq_poller),
        std::ref(sock_pull));

    // pthread_sigmask is unreliable here: gRPC spawns threads with reset
    // masks. Self-pipe + watcher is the only portable option.
    static int shutdown_pipe[2];
    if (pipe(shutdown_pipe) != 0) {
        spdlog::get("multi-logger")->error("pipe() for shutdown failed");
        return EXIT_FAILURE;
    }
    auto signal_pipe_handler = [](int sig) {
        // async-signal-safe: only write() — watcher dispatches in normal context.
        unsigned char c = static_cast<unsigned char>(sig);
        ssize_t n = write(shutdown_pipe[1], &c, 1);
        (void)n;
    };
    struct sigaction sa = {};
    sa.sa_handler = signal_pipe_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGUSR1, &sa, nullptr);

    std::vector<std::thread> cisco_workers;
    std::vector<std::thread> juniper_workers;
    std::vector<std::thread> nokia_workers;
    std::vector<std::thread> huawei_workers;

    LoadThreads(cisco_workers, "socket_cisco", "replies_cisco",
        "cisco_workers");
    LoadThreads(juniper_workers, "socket_juniper", "replies_juniper",
        "juniper_workers");
    LoadThreads(nokia_workers, "socket_nokia", "replies_nokia",
        "nokia_workers");
    LoadThreads(huawei_workers, "socket_huawei", "replies_huawei",
        "huawei_workers");

    std::thread signal_watcher([&zmq_poller]() {
        while (true) {
            unsigned char buf;
            ssize_t n = read(shutdown_pipe[0], &buf, 1);
            if (n != 1) {
                return;
            }
            int sig = static_cast<int>(buf);
            if (sig == SIGUSR1) {
                spdlog::get("multi-logger")->info(
                    "received SIGUSR1, reloading label map");
                ReloadLabelMaps();
                continue;
            }
            spdlog::get("multi-logger")->info(
                "received signal {}, initiating graceful shutdown", sig);
            spdlog::get("multi-logger")->flush();
            initiate_shutdown();
            zmq_poller.get_zmq_ctx().shutdown();
            return;
        }
    });

    for (std::thread &w : cisco_workers) {
        if(w.joinable()) {
            w.join();
        }
    }

    for (std::thread &w : juniper_workers) {
        if(w.joinable()) {
            w.join();
        }
    }

    for (std::thread &w : nokia_workers) {
        if(w.joinable()) {
            w.join();
        }
    }

    for (std::thread &w : huawei_workers) {
        if(w.joinable()) {
            w.join();
        }
    }

    if (zmq_single_thread_poller.joinable()) {
        zmq_single_thread_poller.join();
    }

    if (signal_watcher.joinable()) {
        signal_watcher.join();
    }

    return EXIT_SUCCESS;
}

void *ZmqSingleThreadPoller(ZmqPull &zmq_poller, zmq::socket_t &sock_pull)
{
    const std::string zmq_uri = zmq_poller.get_zmq_transport_uri();

    while(true) {
        try {
            zmq_poller.ZmqPoller(sock_pull, zmq_uri);
        } catch (const zmq::error_t &e) {
            // ETERM = parent ctx destroyed during shutdown; exit cleanly.
            return (NULL);
        }
    }

    return (NULL);
}

void *VendorThread(const std::string &socket_str)
{
    const std::string srv_socket = main_cfg_parameters.at(socket_str);
    if (socket_str.find("cisco") != std::string::npos) {
        Srv cisco_mdt_dialout_collector;
        cisco_mdt_dialout_collector.CiscoBind(srv_socket);
    } else if (socket_str.find("juniper") != std::string::npos) {
        Srv juniper_mdt_dialout_collector;
        juniper_mdt_dialout_collector.JuniperBind(srv_socket);
    } else if (socket_str.find("nokia") != std::string::npos) {
        Srv nokia_mdt_dialout_collector;
        nokia_mdt_dialout_collector.NokiaBind(srv_socket);
    } else if (socket_str.find("huawei") != std::string::npos) {
        Srv huawei_mdt_dialout_collector;
        huawei_mdt_dialout_collector.HuaweiBind(srv_socket);
    }
    return (NULL);
}

void LoadThreads(std::vector<std::thread> &workers_vec,
    const std::string &socket_str,
    const std::string &replies_str,
    const std::string &workers_str)
{
    if (main_cfg_parameters.at(socket_str).empty() == false) {
        int replies = 0;
        int workers = 0;
        try {
            replies = std::stoi(main_cfg_parameters.at(replies_str));
            workers = std::stoi(main_cfg_parameters.at(workers_str));
        } catch (const std::exception &e) {
            spdlog::get("multi-logger")->
                error("[{}/{}] configuration issue: non-numeric value: {}",
                replies_str, workers_str, e.what());
            std::exit(EXIT_FAILURE);
        }
        if (replies < 0 || replies > 1000) {
            spdlog::get("multi-logger")->
                error("[{}] configuration issue: the "
                "allowed amount of replies per session is defined between 10 "
                "and 1000. (default = 0 => unlimited)", replies_str);
            std::exit(EXIT_FAILURE);
        }
        if (workers < 1 || workers > 5) {
            spdlog::get("multi-logger")->
                error("[{}] configuration issue: the "
                "allowed amount of workers is defined between 1 "
                "and 5. (default = 1)", workers_str);
            std::exit(EXIT_FAILURE);
        }
        for (int w = 0; w < workers; w++) {
            workers_vec.push_back(std::thread(&VendorThread,
                socket_str));
        }
        spdlog::get("multi-logger")->
            info("mdt-dialout-collector listening on {} ",
            main_cfg_parameters.at(socket_str));
    }
}

void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv_path)
{
    label_map.clear();

    std::vector<std::string> vtmp;
    std::vector<std::string> ipaddrs;
    std::vector<std::string> nid;
    std::vector<std::string> pid;

    try {
        rapidcsv::Document label_map_doc(label_map_csv_path,
            rapidcsv::LabelParams(-1, -1));
        ipaddrs = label_map_doc.GetColumn<std::string>(0);
        nid = label_map_doc.GetColumn<std::string>(1);
        pid = label_map_doc.GetColumn<std::string>(2);
    } catch (std::exception &ex) {
        spdlog::get("multi-logger")->error("malformed CSV file");
        std::exit(EXIT_FAILURE);
    }

    int ipaddrs_size = ipaddrs.size();

    for (int idx_0 = 0; idx_0 < ipaddrs_size; ++idx_0) {
        vtmp.push_back(nid[idx_0]);
        vtmp.push_back(pid[idx_0]);
        label_map[ipaddrs[idx_0]] = vtmp;
        vtmp.clear();
    }
}

void LoadLabelMapPreTagStyle(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_ptm_path)
{
    label_map.clear();

    std::vector<std::string> vtmp;
    std::vector<std::string> ipaddrs;
    std::vector<std::string> nid;
    std::vector<std::string> pid;
    std::vector<std::string> _ipaddrs;
    std::vector<std::string> _labels;

    try {
        rapidcsv::Document label_map_doc(label_map_ptm_path,
            rapidcsv::LabelParams(-1, -1), rapidcsv::SeparatorParams(' '));

        // Drop trailing sentinel row: set_label=nkey%unknown%pkey%unknown
        label_map_doc.RemoveRow(label_map_doc.GetRowCount());
        _labels = label_map_doc.GetColumn<std::string>(0);
        _ipaddrs = label_map_doc.GetColumn<std::string>(1);
    } catch (std::exception &ex) {
        spdlog::get("multi-logger")->error("malformed PTM file");
        std::exit(EXIT_FAILURE);
    }

    int _ipaddrs_size = _ipaddrs.size();

    std::string d1 = "=";
    std::string d2 = "/";
    std::string d3 = "%";
    for (int idx_0 = 0; idx_0 < _ipaddrs_size; ++idx_0) {

        unsigned start_delim = (_ipaddrs.at(idx_0).find(d1) + 1);
        unsigned stop_delim = _ipaddrs.at(idx_0).find_last_of(d2);
        std::string ipaddr = _ipaddrs.at(idx_0).substr(
            start_delim, (stop_delim - start_delim));

        ipaddrs.push_back(ipaddr);

        start_delim = (_labels.at(idx_0).find(d1) + 1);
        stop_delim = _labels.at(idx_0).length();
        std::string __label = _labels.at(idx_0).substr(
            start_delim, (stop_delim - start_delim));

        int counter = 0;
        size_t pos = 0;
        std::string token;
        while ((pos = __label.find(d3)) != std::string::npos) {
            token = __label.substr(0, pos);
            if (counter == 1) {
                nid.push_back(token);
            }
            __label.erase(0, pos + d3.length());
            counter++;
        }
        pid.push_back(__label);
    }

    for (int idx_0 = 0; idx_0 < _ipaddrs_size; ++idx_0) {
        vtmp.push_back(nid[idx_0]);
        vtmp.push_back(pid[idx_0]);
        label_map[ipaddrs[idx_0]] = vtmp;
        vtmp.clear();
    }
}

bool DumpCorePid(int &core_pid, const std::string &core_pid_path)
{
    std::ofstream outf{ core_pid_path, std::ios::out };
    if (!outf) {
        return false;
    } else {
        outf << core_pid;
        outf.close();
    }

    return true;
}

void ReloadLabelMaps()
{
    // Holds unique_lock for the whole reload; readers stall briefly.
    std::unique_lock<std::shared_mutex> lock(label_map_mutex);
    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map").compare("true") == 0) {
        spdlog::get("multi-logger")->info("re-freshing label-map-csv data");
        LoadLabelMap(label_map,
            data_manipulation_cfg_parameters.at("label_map_csv_path"));
    }
    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map_ptm").compare("true") == 0) {
        spdlog::get("multi-logger")->info("re-freshing label-map-ptm data");
        LoadLabelMapPreTagStyle(label_map,
            data_manipulation_cfg_parameters.at("label_map_ptm_path"));
    }
}

std::string ReadVersionFromFile(const std::string &filePath)
{
    std::ifstream versionFile(filePath);
    std::string version;
    if (versionFile.is_open()) {
        std::getline(versionFile, version);
        versionFile.close();
    } else {
        version = "UNKNOWN";
    }
    return version;
}

