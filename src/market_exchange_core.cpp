#include <iostream>
#include <thread>
#include <iomanip> // For pretty printing
#include "input_stream.hpp"
#include "order.hpp"
#include "thread_safe_queue.hpp"
#include "id_generator.hpp"
#include "order_generator.hpp"

using namespace ex;

/**
 * @brief Helper to print Order details to the console
 */
void printOrder(const Order& o) {
    std::cout << "[CORE] Received Order: "
              << "ID: " << std::setw(4) << o.internal_order_id << " | "
              << "Symbol: " << std::setw(5) << o.symbol << " | "
              << "Side: " << (o.side == Side::Buy ? "BUY " : "SELL") << " | "
              << "Qty: " << std::setw(5) << o.quantity << " | "
              << "Price: " << std::setw(8) << o.price 
              << std::endl;
}

int main() {
    std::cout << "--- Initializing Market Exchange Core ---" << std::endl;

    ThreadSafeQueue<std::string> rawQueue;
    ThreadSafeQueue<Order> orderQueue;
    IdGenerator id_generator;
    const std::string inbound_port = "5555";
    const std::string outbound_port = "5556";
    const int num_json_parsing_threads = 8;

    InputStream inputProcessor(&rawQueue, inbound_port);

    // create and detach thread
    std::thread inputThread(&InputStream::startListening, &inputProcessor);
    inputThread.detach();

    // parellelize json parsing
    std::vector<std::unique_ptr<OrderGenerator>> workers;
    for (int i = 0; i < num_json_parsing_threads; ++i) {
        // Each worker handles JSON parsing and ID generation
        workers.push_back(std::make_unique<OrderGenerator>(
            &rawQueue, &orderQueue, &id_generator, outbound_port
        ));
        
        // Launch worker in its own thread
        std::thread([&worker = workers.back()]() {
            worker->run();
        }).detach();
    }

    std::cout << "[CORE] Exchange is LIVE. Waiting for orders..." << std::endl;

    // main processing loop
    while (true) {
        Order o = orderQueue.pop();
        std::cout << "[DEBUG] Raw Queue Size: " << rawQueue.size() << " | Orders Parsed: " << o.internal_order_id << std::endl;
        printOrder(o); // placeholder for now, matching engine and order books will be used here

    }

    return 0;
}
