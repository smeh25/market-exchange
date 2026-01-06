#include "order_generator.hpp"
#include "net/codec_json.hpp"
#include <iostream>

namespace ex {

OrderGenerator::OrderGenerator(ThreadSafeQueue<std::string>* raw_queue,
                               ThreadSafeQueue<Order>* order_queue,
                               IdGenerator* id_gen,
                               const std::string& out_port)
    : context(1), 
      raw_queue(raw_queue),
      order_queue(order_queue),
      id_generator(id_gen),
      out_socket(context, zmq::socket_type::push),
      running(false) 
{
    // Connect to the internal "Out" port to send Acks
    try{
        out_socket.connect("tcp://localhost:" + out_port);
        std::cout << "Output socket connected. Out:" << out_port << std::endl;
    } catch (const zmq::error_t& e) {
        std::cerr << "ZMQ Connect Error: " << e.what() << std::endl;
    }
}

OrderGenerator::~OrderGenerator() {
    stop();
}

void OrderGenerator::stop(){
    running = false;
    out_socket.close();
}

void OrderGenerator::run() {
    running = true;
    std::cout << "OrderGenerator thread running" << std::endl;

    while (running) {
        try{
            // Pop blocks until a raw JSON string is available from the InputStream
            std::string raw_json = raw_queue->pop();
            
            Order o = convertToOrder(raw_json);
            
            if (o.quantity > 0) {
                order_queue->push(std::move(o));
            }
        } catch (const std::exception& e) {
            // Log the error but DO NOT let the thread exit
            std::cerr << "[WORKER ERROR] Thread encountered issue: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[WORKER ERROR] Unknown critical failure in worker thread!" << std::endl;
        }
    }
}

Order OrderGenerator::convertToOrder(const std::string& json_raw) {
    try {
        EnvelopeIn envelope = parse_inbound_envelope(json_raw);

        if (std::holds_alternative<NewOrderRequest>(envelope.body)) {
            const auto& req = std::get<NewOrderRequest>(envelope.body);

            uint64_t internal_id = id_generator->next();
            
            // In a real HFT system, you'd pass a high-res timestamp from the receiver here
            uint64_t ts = 0; 

            Order o(req.client_order_id, internal_id, ts, req.symbol, 
                    req.side, envelope.header.type, req.limit_price, req.qty);

            // Prepare Ack
            Ack ack_msg { req.client_order_id, internal_id, req.symbol };
            EnvelopeOut response;
            response.header = envelope.header;
            response.header.type = MsgType::Ack;
            response.body = ack_msg;

            sendResponse(dump_envelope(response));
            return o;
        }
        // Handle other alternatives (CancelRequest, etc.) here
    } 
    catch (const std::exception& e) {
        std::cerr << "[WORKER] Parse Error: " << e.what() << std::endl;
        
        Reject rej_msg;
        rej_msg.symbol = "UNKNOWN";
        rej_msg.info.reason = e.what();

        EnvelopeOut response;
        response.header.type = MsgType::Reject;
        response.body = rej_msg;

        sendResponse(dump_envelope(response));
    }
    return Order();
}

void OrderGenerator::sendResponse(const std::string& message) {
    zmq::message_t response(message.size());
    memcpy(response.data(), message.data(), message.size());
    out_socket.send(response, zmq::send_flags::none);
}

} // namespace ex