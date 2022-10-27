// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "bridge_lib.h"


int main(int argc, char *argv[])
{
    pthread_t *worker = NULL;
    start_grpc_dialout_collector(worker);

    return EXIT_SUCCESS;
}

