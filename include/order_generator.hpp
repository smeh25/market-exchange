#pragma once

#include <string>
#include <atomic>
#include "order.hpp"
#include "thread_safe_queue.hpp"
#include "id_generator.hpp"
#include <zmq.hpp>

namespace ex {

class OrderGenerator {
public:
    OrderGenerator(ThreadSafeQueue<std::string>* raw_queue,
                   ThreadSafeQueue<Order>* order_queue,
                   IdGenerator* id_gen,
                   const std::string& out_port);

    ~OrderGenerator();
    // This is the loop that each worker thread will run
    void run();
    void stop();

private:
    // Internal logic moved from InputStream
    Order convertToOrder(const std::string& json_raw);
    void sendResponse(const std::string& message);

    zmq::context_t context;
    ThreadSafeQueue<std::string>* raw_queue;
    ThreadSafeQueue<Order>* order_queue;
    IdGenerator* id_generator;
    
    // Each worker needs its own socket to send Acks/Rejects
    zmq::socket_t out_socket;
    std::atomic<bool> running;
};

} // namespace ex