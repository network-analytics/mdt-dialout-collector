// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// C++ Standard Library headers
#include <exception>
#include <memory>
#include <thread>
#include <csignal>
#include <fstream>
#include <zmq.hpp>
// External Library headers
#include "csv/rapidcsv.h"
// mdt-dialout-collector Library headers
#include "core/mdt_dialout_core.h"


void *ZmqSingleThreadPoller(ZmqPull &zmq_poller,
    zmq::socket_t &zmq_sock);
void *VendorThread(const std::string &vendor);
void LoadThreads(std::vector<std::thread> &workers_vec,
    const std::string &ipv4_socket_str,
    const std::string &replies_str,
    const std::string &workers_str);
void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv);
void LoadLabelMapPreTagStyle(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv);
bool DumpCorePid(int &core_pid, const std::string &core_pid_path);
void SignalHandler(int sig_num);
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
        // default configuration file
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

    // static log-sinks are configured within the constructor
    LogsHandler logs_handler;
    spdlog::get("multi-logger-boot")->debug("main: main()");

    if (f_option_flag == 1 or cfg_path.empty() == false) {
        CfgHandler cfg_handler;
        cfg_handler.set_cfg_path(cfg_path);

        LogsCfgHandler logs_cfg_handler;
        if (logs_cfg_handler.lookup_logs_parameters(
            cfg_handler.get_cfg_path(),
            cfg_handler.get_logs_parameters()) == false) {
            // can't read the logs cfg params the destructor logging won't
            // be possible (segmentation fault)
            std::exit(EXIT_FAILURE);
        } else {
            logs_cfg_parameters = cfg_handler.get_logs_parameters();
            // set the log-sinks after reading from the configuration file
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

    // --- DEBUG ---
    //for (auto &lp : logs_cfg_parameters) {
    //    std::cout << lp.first << " ---> " << lp.second << "\n";
    //}
    //for (auto &mp : main_cfg_parameters) {
    //    std::cout << mp.first << " ---> " << mp.second << "\n";
    //}
    //for (auto &dm : data_manipulation_cfg_parameters) {
    //    std::cout << dm.first << " ---> " << dm.second << "\n";
    //}
    //for (auto &dd : kafka_delivery_cfg_parameters) {
    //    std::cout << dd.first << " ---> " << dd.second << "\n";
    //}
    // --- DEBUG ---

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
        "ipv4 sockets found in config:\n ipv4_socket_cisco: {},\n ipv4_socket_juniper: {},\n ipv4_socket_nokia: {},\n ipv4_socket_huawei: {}\n",
        main_cfg_parameters.at("ipv4_socket_cisco"),
        main_cfg_parameters.at("ipv4_socket_juniper"),
        main_cfg_parameters.at("ipv4_socket_nokia"),
        main_cfg_parameters.at("ipv4_socket_huawei")
    );
    if (main_cfg_parameters.at("ipv4_socket_cisco").empty() == true &&
        main_cfg_parameters.at("ipv4_socket_juniper").empty() == true &&
        main_cfg_parameters.at("ipv4_socket_nokia").empty() == true &&
        main_cfg_parameters.at("ipv4_socket_huawei").empty() == true) {
            spdlog::get("multi-logger")->
                error("[ipv4_socket_*] configuration issue: "
                "unable to find at least one valid IPv4 socket where to bind "
                "the daemon");
            return EXIT_FAILURE;
    }

    // --- ZMQ - Poller ---
    ZmqPull zmq_poller;
    zmq::socket_t sock_pull(zmq_poller.get_zmq_ctx(),
        zmq::socket_type::pull);
    sock_pull.bind(zmq_poller.get_zmq_transport_uri());
    std::thread zmq_single_thread_poller(
        &ZmqSingleThreadPoller,
        std::ref(zmq_poller),
        std::ref(sock_pull));
    zmq_single_thread_poller.detach();
    // --- ZMQ - Poller ---

    std::vector<std::thread> cisco_workers;
    std::vector<std::thread> juniper_workers;
    std::vector<std::thread> nokia_workers;
    std::vector<std::thread> huawei_workers;

    // Cisco
    LoadThreads(cisco_workers, "ipv4_socket_cisco", "replies_cisco",
        "cisco_workers");

    // Juniper
    LoadThreads(juniper_workers, "ipv4_socket_juniper", "replies_juniper",
        "juniper_workers");

    // Nokia
    LoadThreads(nokia_workers, "ipv4_socket_nokia", "replies_nokia",
        "nokia_workers");

    // Huawei
    LoadThreads(huawei_workers, "ipv4_socket_huawei", "replies_huawei",
        "huawei_workers");

    signal(SIGUSR1, SignalHandler);

    //std::cout << "CISCO_WORKERS:   " << cisco_workers.size() << "\n";
    //std::cout << "JUNIPER_WORKERS: " << juniper_workers.size() << "\n";
    //std::cout << "NOKIA_WORKERS: " << nokia_workers.size() << "\n";
    //std::cout << "HUAWEI_WORKERS:  " << huawei_workers.size() << "\n";

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

    return EXIT_SUCCESS;
}

void *ZmqSingleThreadPoller(ZmqPull &zmq_poller, zmq::socket_t &sock_pull)
{
    const std::string zmq_uri = zmq_poller.get_zmq_transport_uri();

    //size_t counter = 0;
    while(true) {
        //std::cout << counter++ << "\n";
        zmq_poller.ZmqPoller(
            sock_pull,
            zmq_uri);
        //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    return (NULL);
}

void *VendorThread(const std::string &ipv4_socket_str)
{
    if (ipv4_socket_str.find("cisco") != std::string::npos) {
        std::string ipv4_socket_cisco =
            main_cfg_parameters.at(ipv4_socket_str);

        std::string cisco_srv_socket {ipv4_socket_cisco};
        Srv cisco_mdt_dialout_collector;
        cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);
    } else if (ipv4_socket_str.find("juniper") != std::string::npos) {
        std::string ipv4_socket_juniper =
            main_cfg_parameters.at(ipv4_socket_str);

        std::string juniper_srv_socket {ipv4_socket_juniper};
        Srv juniper_mdt_dialout_collector;
        juniper_mdt_dialout_collector.JuniperBind(juniper_srv_socket);
    } else if (ipv4_socket_str.find("nokia") != std::string::npos) {
        std::string ipv4_socket_nokia =
            main_cfg_parameters.at(ipv4_socket_str);

        std::string nokia_srv_socket {ipv4_socket_nokia};
        Srv nokia_mdt_dialout_collector;
        nokia_mdt_dialout_collector.NokiaBind(nokia_srv_socket);
    } else if (ipv4_socket_str.find("huawei") != std::string::npos) {
        std::string ipv4_socket_huawei =
            main_cfg_parameters.at(ipv4_socket_str);

        std::string huawei_srv_socket {ipv4_socket_huawei};
        Srv huawei_mdt_dialout_collector;
        huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);
    }

    return (NULL);
}

void LoadThreads(std::vector<std::thread> &workers_vec,
    const std::string &ipv4_socket_str,
    const std::string &replies_str,
    const std::string &workers_str)
{
    if (main_cfg_parameters.at(ipv4_socket_str).empty() == false) {
        int replies =
            std::stoi(main_cfg_parameters.at(replies_str));
        if (replies < 0 || replies > 1000) {
            spdlog::get("multi-logger")->
                error("[{}] configuration issue: the "
                "allowed amount of replies per session is defined between 10 "
                "and 1000. (default = 0 => unlimited)", replies_str);
            std::exit(EXIT_FAILURE);
        }
        int workers = std::stoi(main_cfg_parameters.at(workers_str));
        if (workers < 1 || workers > 5) {
            spdlog::get("multi-logger")->
                error("[{}] configuration issue: the "
                "allowed amount of workers is defined between 1 "
                "and 5. (default = 1)", workers_str);
            std::exit(EXIT_FAILURE);
        }
        for (int w = 0; w < workers; w++) {
            workers_vec.push_back(std::thread(&VendorThread,
                ipv4_socket_str));
        }
        spdlog::get("multi-logger")->
            info("mdt-dialout-collector listening on {} ",
            main_cfg_parameters.at(ipv4_socket_str));
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
    // --- DEBUG ---
    //for (auto &lm : label_map) {
    //    spdlog::get("multi-logger")->
    //      info("{} ---> [ {} , {} ]", lm.first, lm.second.at(0),
    //        lm.second.at(1));
    //}
    // --- DEBUG ---
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

        // Delete last row: set_label=nkey%unknown%pkey%unknown
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
    // --- DEBUG ---
    //for (auto &lm : label_map) {
    //    spdlog::get("multi-logger")->
    //      info("{} ---> [ {} , {} ]", lm.first, lm.second.at(0),
    //        lm.second.at(1));
    //}
    // --- DEBUG ---
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

void SignalHandler(int sig_num)
{
    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map").compare("true") == 0) {
        spdlog::get("multi-logger")->info(
            "siganl {} received, re-freshing label-map-csv data ...", sig_num);
        LoadLabelMap(label_map,
            data_manipulation_cfg_parameters.at("label_map_csv_path"));
    }
    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map_ptm").compare("true") == 0) {
        spdlog::get("multi-logger")->info(
            "siganl {} received, re-freshing label-map-ptm data ...", sig_num);
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

