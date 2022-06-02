#include <iostream>
#include <thread>
#include "mdt_dialout_core.h"


void *cisco_thread(void *);
void *huawei_thread(void *);

int main(void)
{
    void *cisco_ptr {nullptr};
    void *huawei_ptr {nullptr};

    std::thread cisco_t(&cisco_thread, cisco_ptr);
    std::thread huawei_t(&huawei_thread, huawei_ptr);

    cisco_t.join();
    huawei_t.join();

    return (0);
}

void *cisco_thread(void *cisco_ptr)
{
    std::string cisco_srv_socket {"0.0.0.0:10007"};
    Srv cisco_mdt_dialout_collector;
    cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);

    return (0);
}

void *huawei_thread(void *huawei_ptr)
{
    std::string huawei_srv_socket {"0.0.0.0:10008"};
    Srv huawei_mdt_dialout_collector;
    huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);

    return (0);
}
