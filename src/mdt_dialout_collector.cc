// C++ Standard Library headers
#include <thread>
#include <csignal>
#include <fstream>
// External Library headers
#include "csv/rapidcsv.h"
// mdt-dialout-collector Library headers
#include "mdt_dialout_core.h"


void *CiscoThread(void *);
void *HuaweiThread(void *);
void *JuniperThread(void*);
void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map);
void SignalHandler(int sig_num);
// --- Required for config parameters ---
std::unique_ptr<MainCfgHandler> main_cfg_handler(new MainCfgHandler());
// --- Required for config parameters ---

int main(void)
{
	// Store the core process PID to file
    int core_pid = getpid();
    std::ofstream outf{ "/var/run/mdt_dout_collector.pid", std::ios::out };
    if (!outf) {
        std::cout << "Write Error - /var/run/mdt_dout_collector.pid\n";
        return EXIT_FAILURE;
    } else {
    	outf << core_pid;
		outf.close();
	}

    LoadLabelMap(label_map);
    std::vector<std::thread> workers;

    if ((main_cfg_handler->get_ipv4_socket_cisco()).empty() == true &&
        (main_cfg_handler->get_ipv4_socket_huawei()).empty() == true &&
        (main_cfg_handler->get_ipv4_socket_juniper()).empty() == true) {
            std::cout << "no ipv4 sockets were configured\n";
            return EXIT_FAILURE;
    }

    if ((main_cfg_handler->get_ipv4_socket_cisco()).empty() == false) {
        void *cisco_ptr {nullptr};
        int cisco_workers = std::stoi(main_cfg_handler->get_cisco_workers());
        for (int w = 0; w < cisco_workers; ++w) {
            workers.push_back(std::thread(&CiscoThread, cisco_ptr));
        }
        std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_cisco() << "...\n";
    }

    if ((main_cfg_handler->get_ipv4_socket_juniper()).empty() == false) {
        void *juniper_ptr {nullptr};
        int juniper_workers = std::stoi(main_cfg_handler->
            get_juniper_workers());
        for (int w = 0; w < juniper_workers; ++w) {
            workers.push_back(std::thread(&JuniperThread, juniper_ptr));
        }
        std::cout << "mdt-dialout-collector listening on "
        << main_cfg_handler->get_ipv4_socket_juniper() << "...\n";
    }

    if ((main_cfg_handler->get_ipv4_socket_huawei()).empty() == false) {
        void *huawei_ptr {nullptr};
        int huawei_workers = std::stoi(main_cfg_handler->get_huawei_workers());
        for (int w = 0; w < huawei_workers; ++w) {
            workers.push_back(std::thread(&HuaweiThread, huawei_ptr));
        }
        std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_huawei() << "...\n";
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
    // --- Required for config parameters ---
    std::string ipv4_socket_cisco =
        main_cfg_handler->get_ipv4_socket_cisco();
    // --- Required for config parameters ---

    std::string cisco_srv_socket {ipv4_socket_cisco};
    Srv cisco_mdt_dialout_collector;
    cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);

    return 0;
}

void *JuniperThread(void *juniper_ptr)
{
    // --- Required for config parameters ---
    std::string ipv4_socket_juniper =
        main_cfg_handler->get_ipv4_socket_juniper();
    // --- Required for config parameters ---

    std::string juniper_srv_socket {ipv4_socket_juniper};
    Srv juniper_mdt_dialout_collector;
    juniper_mdt_dialout_collector.JuniperBind(juniper_srv_socket);

    return 0;
}

void *HuaweiThread(void *huawei_ptr)
{
    // --- Required for config parameters ---
    std::string ipv4_socket_huawei =
        main_cfg_handler->get_ipv4_socket_huawei();
    // --- Required for config parameters ---

    std::string huawei_srv_socket {ipv4_socket_huawei};
    Srv huawei_mdt_dialout_collector;
    huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);

    return 0;
}

void LoadLabelMap(
    std::unordered_map<std::string,std::vector<std::string>> &label_map)
{
    // Start with empty map
    label_map.clear();

    std::vector<std::string> vtmp;
    std::vector<std::string> ipaddrs;
    std::vector<std::string> nid;
    std::vector<std::string> pid;

    rapidcsv::Document label_map_doc("csv/label_map.csv",
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

void SignalHandler(int sig_num)
{
	std::cout << "Siganl " << sig_num << " received\n";
    LoadLabelMap(label_map);
}

