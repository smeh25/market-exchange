#include "input_stream.hpp"
#include <iostream>
#include "message.hpp"

namespace ex {

// Constructor: Initializes ZMQ context and binds sockets
InputStream::InputStream(ThreadSafeQueue<Order>* inbound_queue, const std::string& in_port, const std::string& out_port)
    : context(1), 
      in_socket(context, zmq::socket_type::pull), 
      out_socket(context, zmq::socket_type::push),
      queue(inbound_queue),
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

                if (o.qty > 0) {
                    queue->push(std::move(o));
                }
            }
        }
    }
}

Order InputStream::convertToOrder(const std::string& json_raw) {
    try {
        auto j = json::parse(json_raw);
        MessageHeader header = j.at("header").get<MessageHeader>();

        if (header.type == MsgType::NEW_ORDER) {
            auto req = j.at("body").get<NewOrderRequest>();

            Order o(header.seq, 0, req.symbol, req.side, header.type, req.limit_price, req.qty);

            Ack ack;
            ack.client_order_id = req.client_order_id;
            ack.order_id = header.seq; 
            ack.symbol = req.symbol;

            json response_json = ack;
            sendResponse(response_json.dump());

            return o;
        }
    } catch (const std::exception& e) {
        Reject rej;
        rej.symbol = "UNKNOWN";
        rej.info.reason = e.what();
        
        json reject_json = rej; 
        sendResponse(reject_json.dump());
    }

    return Order(); 
}

void InputStream::sendResponse(const std::string& message) {
    zmq::message_t response(message.size());
    memcpy(response.data(), message.data(), message.size());
    
    out_socket.send(response, zmq::send_flags::none);
}

} // namespace ex