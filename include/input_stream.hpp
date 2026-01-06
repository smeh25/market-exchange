#pragma once

#include <string>
#include <zmq.hpp>
#include "order.hpp"
#include "thread_safe_queue.hpp"
#include "net/codec_json.hpp"
#include "id_generator.hpp"

namespace ex {

class InputStream {
public:
    // We now take two ports: one for incoming orders, one for outgoing status
    explicit InputStream(ThreadSafeQueue<std::string>* raw_queue, const std::string& in_port);
    
    ~InputStream();

    void startListening();
    void stop();

private:

    // ZMQ Infrastructure
    zmq::context_t context;
    zmq::socket_t in_socket;   // For receiving JSON Orders

    ThreadSafeQueue<std::string>* raw_queue;
    
    bool running;
};

} // namespace ex