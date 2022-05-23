#include <iostream>
#include <pthread.h>
#include "mdt_dialout_core.h"


void *cisco_thread(void *)
{
    std::string cisco_srv_socket {"0.0.0.0:10007"};
    Srv cisco_mdt_dialout_collector;
    cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);

    pthread_exit(NULL);
}

void *huawei_thread(void *)
{
    std::string huawei_srv_socket {"0.0.0.0:10008"};
    Srv huawei_mdt_dialout_collector;
    huawei_mdt_dialout_collector.CiscoBind(huawei_srv_socket);
    
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t cisco_t;
    pthread_t huawei_t;

    pthread_create(&cisco_t, NULL, &cisco_thread, NULL);
    pthread_create(&huawei_t, NULL, &huawei_thread, NULL);

    pthread_join(cisco_t, 0);
    pthread_join(huawei_t, 0);

    //std::string cisco_srv_socket {"0.0.0.0:10007"};
    //std::string huawei_srv_socket {"0.0.0.0:10008"};
    //Srv cisco_mdt_dialout_collector;
    //Srv huawei_mdt_dialout_collector;
    //cisco_mdt_dialout_collector.CiscoBind(cisco_srv_socket);
    //huawei_mdt_dialout_collector.HuaweiBind(huawei_srv_socket);
    //std::cout << "MDT Server listening on " << server_addr << std::endl;

    return (0);
}
