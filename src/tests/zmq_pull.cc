// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <zmq.hpp>
#include <zmq_addon.hpp>


void *zmq_pull(zmq::context_t &ctx);

// CONNECT/RECEIVE - PULL
int main(void)
{
    // ZMQ Context
    zmq::context_t ctx;

    // Actual receiving
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

// Read from socks - multiple threads
void *zmq_pull(zmq::context_t &ctx)
{
    zmq::socket_t sock(ctx, zmq::socket_type::pull);

    // Message Buff preparation
    const size_t size = 4092;
    zmq::message_t message(size);

    // --- Convert the thread ID into string --- //
    auto t_id = std::this_thread::get_id();
    std::stringstream ss;
    ss << t_id;
    std::string thread_id = ss.str();
    // --- Convert the thread ID into string --- //

    std::string sok = "ipc:///tmp/grpc.sock";
    std::cout << "PULL-ing from " << sok << "\n";
    sock.bind(sok);
    while(true) {
        auto res = sock.recv(message, zmq::recv_flags::none);
        if (res.value() != 0) {
            std::cout << thread_id << " PULL-ing from " << sok << ": "
                << message.to_string() << "\n";
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (0);
}

