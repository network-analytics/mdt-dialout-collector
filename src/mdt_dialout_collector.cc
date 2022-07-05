#include <iostream>
#include <thread>
#include "mdt_dialout_core.h"
#include "cfg_handler.h"


void *cisco_thread(void *);
void *huawei_thread(void *);
//void *juniper_thread(void *);
int juniper_single_process(void);
// --- Required for config parameters ---
std::unique_ptr<MainCfgHandler> main_cfg_handler(new MainCfgHandler());
// --- Required for config parameters ---

int main(void)
{
    //std::vector<std::thread> vendors;

    //if ((main_cfg_handler->get_ipv4_socket_cisco()).empty() and
    //    (main_cfg_handler->get_ipv4_socket_huawei()).empty() and
    //    (main_cfg_handler->get_ipv4_socket_juniper()).empty()) {
    //        std::cout << "no ipv4 sockets were configured" << std::endl;
    //        return(EXIT_FAILURE);
    //}

    //if (!(main_cfg_handler->get_ipv4_socket_cisco()).empty()) {
    //    void *cisco_ptr {nullptr};
    //    std::thread cisco_t(&cisco_thread, cisco_ptr);
    //    std::cout << "mdt-dialout-collector listening on "
    //        << main_cfg_handler->get_ipv4_socket_cisco() << "..." << std::endl;
    //    vendors.push_back(std::move(cisco_t));
    //}

    //if (!(main_cfg_handler->get_ipv4_socket_juniper()).empty()) {
    //    void *juniper_ptr {nullptr};
    //    std::thread juniper_t(&juniper_thread, juniper_ptr);
    //    std::cout << "mdt-dialout-collector listening on "
    //    << main_cfg_handler->get_ipv4_socket_juniper() << "..." << std::endl;
    //    vendors.push_back(std::move(juniper_t));
    //}
    
    if (!(main_cfg_handler->get_ipv4_socket_juniper()).empty()) {
        if (juniper_single_process() == 0) {
            std::cout << "mdt-dialout-collector listening on "
            << main_cfg_handler->get_ipv4_socket_juniper() << "..." << std::endl;
        }
    }

    //if (!(main_cfg_handler->get_ipv4_socket_huawei()).empty()) {
    //    void *huawei_ptr {nullptr};
    //    std::thread huawei_t(&huawei_thread, huawei_ptr);
    //    std::cout << "mdt-dialout-collector listening on "
    //        << main_cfg_handler->get_ipv4_socket_huawei() << "..." << std::endl;
    //    vendors.push_back(std::move(huawei_t));
    //}

    //// Handling only required threads
    //for(std::thread& v : vendors) {
    //    if(v.joinable()) {
    //        v.join();
    //    }
    //}

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

//void *juniper_thread(void *juniper_ptr)
//{
//    // --- Required for config parameters ---
//    std::string ipv4_socket_juniper =
//        main_cfg_handler->get_ipv4_socket_juniper();
//    // --- Required for config parameters ---
//
//    std::string juniper_srv_socket {ipv4_socket_juniper};
//    Srv juniper_mdt_dialout_collector;
//    juniper_mdt_dialout_collector.JuniperBind(juniper_srv_socket);
//
//    return (EXIT_SUCCESS);
//}

int juniper_single_process(void)
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

