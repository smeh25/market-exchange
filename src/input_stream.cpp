#include "input_stream.hpp"
#include <iostream>
#include "core/message.hpp"

namespace ex {

// Constructor: Initializes ZMQ context and binds sockets
InputStream::InputStream(ThreadSafeQueue<std::string>* raw_queue, const std::string& in_port)
    : context(1), 
      in_socket(context, zmq::socket_type::pull), 
      raw_queue(raw_queue),
      running(false)
{
    try {
        // High-water mark limits internal buffering to prevent memory overflow
        in_socket.set(zmq::sockopt::rcvhwm, 10000);
        in_socket.bind("tcp://*:" + in_port);
                
        std::cout << "InputStream initialized. In:" << in_port << std::endl;
    } catch (const zmq::error_t& e) {
        std::cerr << "ZMQ Bind Error: " << e.what() << std::endl;
    }
}

// Destructor: Clean up resources
InputStream::~InputStream() {
    stop();
}

void InputStream::stop() {
    running = false;
    in_socket.close();
}

void InputStream::startListening() {
    running = true;
    std::cout << "InputStream: Start listening for orders..." << std::endl;

    while(running){
        try {
            zmq::message_t request;

            auto result = in_socket.recv(request, zmq::recv_flags::none);

            if (result) {
                raw_queue->push(std::string(static_cast<char*>(request.data()), request.size()));
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

} // namespace ex