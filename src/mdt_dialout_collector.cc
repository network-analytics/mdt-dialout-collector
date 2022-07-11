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
    std::vector<std::thread> vendors;
    std::vector<std::thread> workers;

    if ((main_cfg_handler->get_ipv4_socket_cisco()).empty() and
        (main_cfg_handler->get_ipv4_socket_huawei()).empty() and
        (main_cfg_handler->get_ipv4_socket_juniper()).empty()) {
            std::cout << "no ipv4 sockets were configured" << std::endl;
            return(EXIT_FAILURE);
    }

    if (!(main_cfg_handler->get_ipv4_socket_cisco()).empty()) {
        int cisco_workers = std::stoi(main_cfg_handler->get_cisco_workers());
        void *cisco_ptr {nullptr};
        for (int w = 0; w < cisco_workers; ++w) {
            workers.push_back(std::move(std::thread(&cisco_thread, cisco_ptr)));
        }
        std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_cisco() << "..." << std::endl;
    }

    if (!(main_cfg_handler->get_ipv4_socket_juniper()).empty()) {
        void *juniper_ptr {nullptr};
        std::thread juniper_t_0(&juniper_thread, juniper_ptr);
        std::thread juniper_t_1(&juniper_thread, juniper_ptr);
        std::thread juniper_t_2(&juniper_thread, juniper_ptr);
        std::thread juniper_t_3(&juniper_thread, juniper_ptr);
        std::cout << "mdt-dialout-collector listening on "
        << main_cfg_handler->get_ipv4_socket_juniper() << "..." << std::endl;
        vendors.push_back(std::move(juniper_t_0));
        vendors.push_back(std::move(juniper_t_1));
        vendors.push_back(std::move(juniper_t_2));
        vendors.push_back(std::move(juniper_t_3));
    }

    if (!(main_cfg_handler->get_ipv4_socket_huawei()).empty()) {
        void *huawei_ptr {nullptr};
        std::thread huawei_t_0(&huawei_thread, huawei_ptr);
        std::thread huawei_t_1(&huawei_thread, huawei_ptr);
        std::thread huawei_t_2(&huawei_thread, huawei_ptr);
        std::thread huawei_t_3(&huawei_thread, huawei_ptr);
        std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_huawei() << "..." << std::endl;
        vendors.push_back(std::move(huawei_t_0));
        vendors.push_back(std::move(huawei_t_1));
        vendors.push_back(std::move(huawei_t_2));
        vendors.push_back(std::move(huawei_t_3));
    }

    // Handling only required threads
    //for(std::thread& v : vendors) {
    //    if(v.joinable()) {
    //        v.join();
    //    }
    //}
    for(std::thread& w : workers) {
        if(w.joinable()) {
            w.join();
        }
    }

    return (EXIT_SUCCESS);
}

void *cisco_thread(void *cisco_ptr)
{
    // --- Required for config parameters ---
    std::string ipv4_socket_cisco = main_cfg_handler->get_ipv4_socket_cisco();
    // --- Required for config parameters ---

    std::string cisco_srv_socket {ipv4_socket_cisco};
    Srv cisco_mdt_dialout_collector;
    cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);

    return (EXIT_SUCCESS);
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

    return (EXIT_SUCCESS);
}

void *huawei_thread(void *huawei_ptr)
{
    // --- Required for config parameters ---
    std::string ipv4_socket_huawei = main_cfg_handler->get_ipv4_socket_huawei();
    // --- Required for config parameters ---

    std::string huawei_srv_socket {ipv4_socket_huawei};
    Srv huawei_mdt_dialout_collector;
    huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);

    return (EXIT_SUCCESS);
}

