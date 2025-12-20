#pragma once

#include <string>
#include <zmq.hpp>
#include "order.hpp"
#include "thread_safe_queue.hpp"
#include "codec_json.hpp"
#include "id_generator.hpp"

namespace ex {

class InputStream {
public:
    // We now take two ports: one for incoming orders, one for outgoing status
    explicit InputStream(ThreadSafeQueue<Order>* inbound_queue, IdGenerator* inbound_id_generator, const std::string& in_port, const std::string& out_port);
    
    ~InputStream();

    void startListening();

private:
    // This now handles the JSON logic
    Order convertToOrder(const std::string& json_raw);
    
    // Helper to send "Success" or "Invalid JSON" back to the sender
    void sendResponse(const std::string& message);

    // ZMQ Infrastructure
    zmq::context_t context;
    zmq::socket_t in_socket;   // For receiving JSON Orders
    zmq::socket_t out_socket;  // For sending receipts/errors

    ThreadSafeQueue<Order>* queue;
    IdGenerator* id_generator;
    
    bool running;
};

} // namespace ex