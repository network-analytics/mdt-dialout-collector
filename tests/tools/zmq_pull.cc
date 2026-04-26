// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>


void *zmq_pull(zmq::context_t &ctx);

int main(void)
{
    zmq::context_t ctx;

    std::vector<std::thread> th_fire;
    size_t th = 1;
    std::cout << "Firing " << th << " threads, Reading & PULL-ing\n";
    for (size_t t = 0; t < th; ++t) {
        th_fire.push_back(std::thread (
            &zmq_pull,
            std::ref(ctx)));
    }

    for (std::thread &t : th_fire) {
        if (t.joinable()) {
            t.join();
        }
    }

    return EXIT_SUCCESS;
}

void *zmq_pull(zmq::context_t &ctx)
{
    zmq::socket_t sock(ctx, zmq::socket_type::pull);

    const size_t size = 4096;
    zmq::message_t message(size);

    auto t_id = std::this_thread::get_id();
    std::stringstream ss;
    ss << t_id;
    std::string thread_id = ss.str();

    std::string sok = "ipc:///tmp/grpc.sock";
    std::cout << "PULL-ing from " << sok << "\n";
    sock.bind(sok);
    while(true) {
        auto res = sock.recv(message, zmq::recv_flags::none);
        if (res.value() != 0) {
            std::cout << thread_id << " PULL-ing from " << sok << ": "
                << message.to_string() << "\n";
        }
    }

    return (0);
}

