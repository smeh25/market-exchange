#include <iostream>
#include <thread>
#include <iomanip> // For pretty printing
#include "input_stream.hpp"
#include "order.hpp"
#include "thread_safe_queue.hpp"
#include "id_generator.hpp"

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


    ThreadSafeQueue<Order> orderQueue;
    IdGenerator id_generator;

    InputStream inputProcessor(&orderQueue, &id_generator, "5555", "5556");

    // create and detach thread
    std::thread inputThread(&InputStream::startListening, &inputProcessor);
    inputThread.detach();

    std::cout << "[CORE] Exchange is LIVE. Waiting for orders..." << std::endl;

    // main processing loop
    while (true) {
        Order o = orderQueue.pop();

        printOrder(o); // placeholder for now, matching engine and order books will be used here

    }

    return 0;
}
