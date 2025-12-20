#pragma once
#include <string>
#include "core/Types.hpp"

class Order {
    public:
        uint64_t id;
        uint64_t timestamp;
        std::string symbol;
        ex::Side side;
        ex::MsgType type;
        double price;
        uint32_t quantity;

        // Main Constructor
        Order(uint64_t id, uint64_t timestamp, const std::string symbol, 
            ex::Side side, ex::MsgType type, double price, uint32_t quantity)

            : id(id), timestamp(timestamp), symbol(symbol), 
            side(side), type(type), price(price), quantity(quantity) 

        {
        }

        // Default Constructor
        Order() = default;
};