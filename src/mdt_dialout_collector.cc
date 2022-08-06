// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// C++ Standard Library headers
#include <memory>
#include <thread>
#include <csignal>
#include <fstream>
// External Library headers
#include "csv/rapidcsv.h"
// mdt-dialout-collector Library headers
#include "juniper_gnmi.pb.h"
#include "mdt_dialout_core.h"
#include "logs_handler.h"


void *CiscoThread(void *);
void *HuaweiThread(void *);
void *JuniperThread(void*);
void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv);
bool DumpCorePid(int &core_pid, const std::string &core_pid_path);
void SignalHandler(int sig_num);

int main(void)
{
    // --- DEBUG ---
    //for (auto& mp : main_cfg_parameters) {
    //    std::cout << mp.first << " ---> " << mp.second << "\n";
    //}
    //for (auto& dm : data_manipulation_cfg_parameters) {
    //    std::cout << dm.first << " ---> " << dm.second << "\n";
    //}
    //for (auto& dd : data_delivery_cfg_parameters) {
    //    std::cout << dd.first << " ---> " << dd.second << "\n";
    //}
    // --- DEBUG ---

    int core_pid = getpid();
    const std::string core_pid_folder =
        main_cfg_parameters.at("core_pid_folder");
    const std::string core_pid_file = "mdt_dialout_collector.pid";
    const std::string core_pid_path = core_pid_folder + core_pid_file;
    if (DumpCorePid(core_pid, core_pid_path) == false) {
        multi_logger->error(
            "unable to dump PID {} to {}", core_pid, core_pid_path);
        return EXIT_FAILURE;
    }

    if (data_manipulation_cfg_parameters.at(
            "enable_label_encode_as_map").compare("true") == 0) {
        LoadLabelMap(label_map,
            data_manipulation_cfg_parameters.at("label_map_csv_path"));
    }
    std::vector<std::thread> workers;

    if (main_cfg_parameters.at("ipv4_socket_cisco").empty() == true &&
        main_cfg_parameters.at("ipv4_socket_juniper").empty() == true &&
        main_cfg_parameters.at("ipv4_socket_huawei").empty() == true) {
            multi_logger->error("[ipv4_socket_*] configuration issue: "
                "unable to find at least one valid IPv4 socket where to bind "
                "the daemon");
            return EXIT_FAILURE;
    }

    // Cisco
    if (main_cfg_parameters.at("ipv4_socket_cisco").empty() == false) {
        int replies_cisco =
            std::stoi(main_cfg_parameters.at("replies_cisco"));
        if (replies_cisco < 10 || replies_cisco > 1000) {
            multi_logger->error("[replies_cisco] configuaration issue: the "
                "allowed amount of replies per session is defined between 10 "
                "and 1000. (default = 100)");
            return EXIT_FAILURE;
        }
        void *cisco_ptr {nullptr};
        int cisco_workers =
            std::stoi(main_cfg_parameters.at("cisco_workers"));
        if (cisco_workers < 0 || cisco_workers > 5) {
            multi_logger->error("[cisco_workers] configuaration issue: the "
                "allowed amount of workers is defined between 0 "
                "and 5. (default = 1)");
            return EXIT_FAILURE;
        }
        for (int w = 0; w < cisco_workers; ++w) {
            workers.push_back(std::thread(&CiscoThread, cisco_ptr));
        }
        multi_logger->info("mdt-dialout-collector listening on {} ",
            main_cfg_parameters.at("ipv4_socket_cisco"));
    }

    // Juniper
    if (main_cfg_parameters.at("ipv4_socket_juniper").empty() == false) {
        int replies_juniper =
            std::stoi(main_cfg_parameters.at("replies_juniper"));
        if (replies_juniper < 10 || replies_juniper > 1000) {
            multi_logger->error("[replies_juniper] configuaration issue: the "
                "allowed amount of replies per session is defined between 10 "
                "and 1000. (default = 100)");
            return EXIT_FAILURE;
        }
        void *juniper_ptr {nullptr};
        int juniper_workers =
            std::stoi(main_cfg_parameters.at("juniper_workers"));
        if (juniper_workers < 0 || juniper_workers > 5) {
            multi_logger->error("[juniper_workers] configuaration issue: the "
                "allowed amount of workers is defined between 0 "
                "and 5. (default = 1)");
            return EXIT_FAILURE;
        }
        for (int w = 0; w < juniper_workers; ++w) {
            workers.push_back(std::thread(&JuniperThread, juniper_ptr));
        }
        multi_logger->info("mdt-dialout-collector listening on {} ",
            main_cfg_parameters.at("ipv4_socket_juniper"));
    }

    // Huawei
    if (main_cfg_parameters.at("ipv4_socket_huawei").empty() == false) {
        int replies_huawei =
            std::stoi(main_cfg_parameters.at("replies_huawei"));
        if (replies_huawei < 10 || replies_huawei > 1000) {
            multi_logger->error("[replies_huawei] configuaration issue: the "
                "allowed amount of replies per session is defined between 10 "
                "and 1000. (default = 100)");
            return EXIT_FAILURE;
        }
        void *huawei_ptr {nullptr};
        int huawei_workers =
            std::stoi(main_cfg_parameters.at("huawei_workers"));
        if (huawei_workers < 0 || huawei_workers > 5) {
            multi_logger->error("[huawei_workers] configuaration issue: the "
                "allowed amount of workers is defined between 0 "
                "and 5. (default = 1)");
            return EXIT_FAILURE;
        }
        for (int w = 0; w < huawei_workers; ++w) {
            workers.push_back(std::thread(&HuaweiThread, huawei_ptr));
        }
        multi_logger->info("mdt-dialout-collector listening on {} ",
            main_cfg_parameters.at("ipv4_socket_huawei"));
    }

    signal(SIGUSR1, SignalHandler);

    for (std::thread &w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }

    return EXIT_SUCCESS;
}

void *CiscoThread(void *cisco_ptr)
{
    std::string ipv4_socket_cisco =
        main_cfg_parameters.at("ipv4_socket_cisco");

    std::string cisco_srv_socket {ipv4_socket_cisco};
    Srv cisco_mdt_dialout_collector;
    cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);

    return 0;
}

void *JuniperThread(void *juniper_ptr)
{
    std::string ipv4_socket_juniper =
        main_cfg_parameters.at("ipv4_socket_juniper");

    std::string juniper_srv_socket {ipv4_socket_juniper};
    Srv juniper_mdt_dialout_collector;
    juniper_mdt_dialout_collector.JuniperBind(juniper_srv_socket);

    return 0;
}

void *HuaweiThread(void *huawei_ptr)
{
    std::string ipv4_socket_huawei =
        main_cfg_parameters.at("ipv4_socket_huawei");

    std::string huawei_srv_socket {ipv4_socket_huawei};
    Srv huawei_mdt_dialout_collector;
    huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);

    return 0;
}

void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    const std::string &label_map_csv)
{
    label_map.clear();

    std::vector<std::string> vtmp;
    std::vector<std::string> ipaddrs;
    std::vector<std::string> nid;
    std::vector<std::string> pid;

    rapidcsv::Document label_map_doc(label_map_csv,
        rapidcsv::LabelParams(-1, -1));
    ipaddrs = label_map_doc.GetColumn<std::string>(0);
    nid = label_map_doc.GetColumn<std::string>(1);
    pid = label_map_doc.GetColumn<std::string>(2);

    int ipaddrs_size = ipaddrs.size();

    for (int idx_0 = 0; idx_0 < ipaddrs_size; ++idx_0) {
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

void SignalHandler(int sig_num)
{
    multi_logger->info(
        "siganl {} received, re-freshing label_map information", sig_num);
    LoadLabelMap(label_map,
        data_manipulation_cfg_parameters.at("label_map_csv_path"));
}

