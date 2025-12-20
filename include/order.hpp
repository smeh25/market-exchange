#pragma once
#include <string>
#include "core/types.hpp"

class Order {
    public:
        uint64_t client_order_id;
        uint64_t internal_order_id;
        uint64_t timestamp;
        std::string symbol;
        ex::Side side;
        ex::MsgType type;
        double price;
        uint32_t quantity;

        // Main Constructor
        Order(uint64_t c_id, uint64t i_id, uint64_t timestamp, const std::string symbol, 
            ex::Side side, ex::MsgType type, double price, uint32_t quantity)

            : client_order_id(c_id), internal_order_id(i_id), timestamp(timestamp), symbol(symbol), 
            side(side), type(type), price(price), quantity(quantity) 

        {
        }

        // Default Constructor
        Order() = default;
};
