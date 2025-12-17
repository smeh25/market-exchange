#pragma once

#include "core/Messages.hpp"       
#include "core/codec_json.hpp" 
#include "core/id_generator.hpp"    
#include "core/Order.hpp"          
#include <string>
#include <memory>                  

namespace ex {
    class MatchingEngine;
}

namespace ex {

class RequestRouter {
public:

    RequestRouter(MatchingEngine& engine, IdGenerator& id_gen);

    // --- Main Entry Point ---
    // 1. Takes the raw ZMQ identity frame (so we know who sent it)
    // 2. Takes the raw JSON payload
    // 3. Routes it to the correct handler
    void process_msg(const std::string& client_address, const std::string& json_payload);

private:
    // --- Dependencies ---
    MatchingEngine& engine;
    IdGenerator& ig_gen;


    
    // Converts DTO (NewOrderRequest) -> Domain Object (Order)
    void handle_new_order(const std::string& client_address, const NewOrderRequest& req);
    
    // Handles cancellation logic
    void handle_cancel(const std::string& client_address, const CancelRequest& req);

    // Helper to send an immediate rejection if JSON is invalid
    void send_reject(const std::string& client_address, uint64_t client_order_id, const std::string& reason);
    
    void send_ack(const std::string& client_address, uint64_t client_order_id, uint64_t internal_order_id, const std::string& symbol);
    void send_to_client(const std::string& client_address, const OutboundMsg& msg);

};

}