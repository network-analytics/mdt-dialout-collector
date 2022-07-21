#include <iostream>
#include <thread>
#include "mdt_dialout_core.h"
#include "cfg_handler.h"


void *cisco_thread(void *);
void *huawei_thread(void *);
void *juniper_thread(void *);
// --- Required for config parameters ---
std::unique_ptr<MainCfgHandler> main_cfg_handler(new MainCfgHandler());
// --- Required for config parameters ---

int main(void)
{
    std::vector<std::thread> workers;

    if ((main_cfg_handler->get_ipv4_socket_cisco()).empty() == true and
        (main_cfg_handler->get_ipv4_socket_huawei()).empty() == true and
        (main_cfg_handler->get_ipv4_socket_juniper()).empty() == true) {
            std::cout << "no ipv4 sockets were configured\n";
            return EXIT_FAILURE;
    }

    if ((main_cfg_handler->get_ipv4_socket_cisco()).empty() == false) {
        void *cisco_ptr {nullptr};
        int cisco_workers = std::stoi(main_cfg_handler->get_cisco_workers());
        for (int w = 0; w < cisco_workers; ++w) {
            workers.push_back(std::thread(&cisco_thread, cisco_ptr));
        }
        std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_cisco() << "...\n";
    }

    if ((main_cfg_handler->get_ipv4_socket_juniper()).empty() == false) {
        void *juniper_ptr {nullptr};
        int juniper_workers = std::stoi(main_cfg_handler->
            get_juniper_workers());
        for (int w = 0; w < juniper_workers; ++w) {
            workers.push_back(std::thread(&juniper_thread, juniper_ptr));
        }
        std::cout << "mdt-dialout-collector listening on "
        << main_cfg_handler->get_ipv4_socket_juniper() << "...\n";
    }

    if ((main_cfg_handler->get_ipv4_socket_huawei()).empty() == false) {
        void *huawei_ptr {nullptr};
        int huawei_workers = std::stoi(main_cfg_handler->get_huawei_workers());
        for (int w = 0; w < huawei_workers; ++w) {
            workers.push_back(std::thread(&huawei_thread, huawei_ptr));
        }
        std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_huawei() << "...\n";
    }

    for(std::thread& w : workers) {
        if(w.joinable()) {
            w.join();
        }
    }

    return EXIT_SUCCESS;
}

void *cisco_thread(void *cisco_ptr)
{
    // --- Required for config parameters ---
    std::string ipv4_socket_cisco = main_cfg_handler->get_ipv4_socket_cisco();
    // --- Required for config parameters ---

    std::string cisco_srv_socket {ipv4_socket_cisco};
    Srv cisco_mdt_dialout_collector;
    cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);

    return 0;
}

void *juniper_thread(void *juniper_ptr)
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

void *huawei_thread(void *huawei_ptr)
{
    // --- Required for config parameters ---
    std::string ipv4_socket_huawei = main_cfg_handler->get_ipv4_socket_huawei();
    // --- Required for config parameters ---

    std::string huawei_srv_socket {ipv4_socket_huawei};
    Srv huawei_mdt_dialout_collector;
    huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);

    return 0;
}

