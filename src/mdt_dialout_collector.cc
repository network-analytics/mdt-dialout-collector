#include <iostream>
#include "mdt_dialout_core.h"


int main(void)
{
    ServerImpl::ServerImpl server;
    server.Run();

    return (0);
}