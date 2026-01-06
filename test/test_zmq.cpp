#include <iostream>
#include <string>
#include <lib/nlohmann/json.hpp>
#include <zmq.hpp>

// Using the same JSON library you're using for your codec
using json = nlohmann::json;

int main() {
    // 1. Initialize ZMQ context and a PULL socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::pull);

    // 2. Bind to a port (we'll use 5555)
    std::string port = "5555";
    socket.bind("tcp://*:" + port);

    std::cout << "TEST: Listening for JSON messages on port " << port << "..." << std::endl;

    while (true) {
        zmq::message_t request;

        // 3. Wait for a message (this blocks until one arrives)
        auto result = socket.recv(request, zmq::recv_flags::none);

        if (result) {
            // 4. Convert the ZMQ message to a string
            std::string msg_str(static_cast<char*>(request.data()), request.size());

            try {
                // 5. Parse and Print
                auto j = json::parse(msg_str);
                
                std::cout << "\n--- NEW MESSAGE RECEIVED ---" << std::endl;
                // 'dump(4)' makes the JSON pretty-printed with 4 spaces
                std::cout << j.dump(4) << std::endl;
                std::cout << "----------------------------" << std::endl;

            } catch (const json::parse_error& e) {
                std::cerr << "JSON Parse Error: " << e.what() << std::endl;
                std::cerr << "Raw Data: " << msg_str << std::endl;
            }
        }
    }

    return 0;
}