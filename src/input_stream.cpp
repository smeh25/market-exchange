#include "input_stream.hpp"
#include <iostream>
#include "message.hpp"

namespace ex {

// Constructor: Initializes ZMQ context and binds sockets
InputStream::InputStream(ThreadSafeQueue<Order>* inbound_queue, IdGenerator* inbound_id_generator, const std::string& in_port, const std::string& out_port)
    : context(1), 
      in_socket(context, zmq::socket_type::pull), 
      out_socket(context, zmq::socket_type::push),
      queue(inbound_queue),
      id_generator(inbound_id_generator),
      running(false) 
{
    try {
        in_socket.bind("tcp://*:" + in_port);
        
        out_socket.bind("tcp://*:" + out_port);
        
        std::cout << "InputStream initialized. In:" << in_port << " Out:" << out_port << std::endl;
    } catch (const zmq::error_t& e) {
        std::cerr << "ZMQ Bind Error: " << e.what() << std::endl;
    }
}

// Destructor: Clean up resources
InputStream::~InputStream() {
    running = false;
    in_socket.close();
    out_socket.close();
}

void InputStream::startListening() {
    running = true;
    std::cout << "InputStream: Start listening for orders..." << std::endl;

    while(running){
        try {
            zmq::message_t request;

            auto result = in_socket.recv(request, zmq::recv_flags::none);

            if (result) {
                std::string msg_str(static_cast<char*>(request.data()), request.size());

                Order o = convertToOrder(msg_str);

                if (o.quantity > 0) {
                    queue->push(std::move(o));
                }
            }
        }
        catch (const zmq::error_t& e) {
            // Check if error was just an interrupt (like Ctrl+C)
            if (e.num() == ETERM) {
                std::cout << "InputStream: ZMQ context terminated, shutting down..." << std::endl;
                running = false;
            } else {
                std::cerr << "InputStream ZMQ Error: " << e.what() << std::endl;
            }
        }
        catch (const std::exception& e) {
            // This catches JSON errors or logic errors that leaked out of convertToOrder
            std::cerr << "InputStream Logic Error: " << e.what() << std::endl;
        }
        catch (...) {
            // Catch-all for anything else
            std::cerr << "InputStream: Unknown critical error occurred!" << std::endl;
        }
    }
}

Order InputStream::convertToOrder(const std::string& json_raw) {
    try {

        EnvelopeIn envelope = parse_inbound_envelope(json_raw);

        if (std::holds_alternative<NewOrderRequest>(envelope.body)) {
            const auto& req = std::get<NewOrderRequest>(envelope.body);

            uint64_t internal_id = id_generator->next();
            uint64_t ts = 0; // placeholder for now

            Order o(
                req.client_order_id, 
                internal_id, 
                ts, 
                req.symbol, 
                req.side, 
                envelope.header.type, 
                req.limit_price, 
                req.qty
            );

            Ack ack_msg;
            ack_msg.client_order_id = req.client_order_id;
            ack_msg.order_id = internal_id;
            ack_msg.symbol = req.symbol;

            EnvelopeOut response;
            response.header = envelope.header; // Copy incoming header metadata
            response.header.type = MsgType::Ack; // Set type to Ack
            response.body = ack_msg;

            sendResponse(dump_envelope(response));

            return o;
        } 
        else if (std::holds_alternative<CancelRequest>(envelope.body)) {
            // Future: Handle CancelRequest here
            std::cout << "Received Cancel Request - Logic not yet implemented" << std::endl;
        }

    } catch (const std::exception& e) {
        // 6. Send structured REJECT
        Reject rej_msg;
        rej_msg.symbol = "UNKNOWN";
        rej_msg.info.reason = e.what();

        EnvelopeOut response;
        response.header.type = MsgType::Reject;
        response.body = rej_msg;

        sendResponse(dump_envelope(response));
    }

    return Order(); // Return empty order for non-new-order messages
}

void InputStream::sendResponse(const std::string& message) {
    zmq::message_t response(message.size());
    memcpy(response.data(), message.data(), message.size());
    
    out_socket.send(response, zmq::send_flags::none);
}

} // namespace ex