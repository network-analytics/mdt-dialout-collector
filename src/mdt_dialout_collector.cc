#include <iostream>
#include "mdt_dialout_core.h"


int main(void)
{
    std::string cisco_srv_socket {"0.0.0.0:10007"};
    std::string huawei_srv_socket {"0.0.0.0:10008"};
    Srv mdt_dialout_collector;
    mdt_dialout_collector.Bind(cisco_srv_socket, huawei_srv_socket);
    //std::cout << "MDT Server listening on " << server_addr << std::endl;

    return (0);
}
