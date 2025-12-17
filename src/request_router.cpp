#include "core/request_router.hpp"
#include "core/matching_engine.hpp" 
#include <iostream>
#include <chrono>

namespace ex {

// --- Constructor ---
RequestRouter::RequestRouter(MatchingEngine& engine, IdGenerator& id_gen)
    : engine_(engine), id_gen_(id_gen) 
{
}

// --- Main Entry Point ---
void RequestRouter::process_msg(const std::string& client_address, const std::string& json_payload) {
    try {
        auto j = json::parse(json_payload);

        if (!j.contains("header") || !j.contains("body")) {
            send_reject(client_address, 0, "UNKNOWN", "Malformed JSON: Missing header/body", 400);
            return;
        }

        MessageHeader header = j["header"].get<MessageHeader>();

        switch (header.type) {
            case MsgType::NewOrder: {
                NewOrderRequest req = j["body"].get<NewOrderRequest>();
                handle_new_order(client_address, req);
                break;
            }
            case MsgType::Cancel: {
                CancelRequest req = j["body"].get<CancelRequest>();
                handle_cancel(client_address, req);
                break;
            }
            default: {
                std::cerr << "Unknown MsgType: " << (int)header.type << std::endl;
                send_reject(client_address, 0, "UNKNOWN", "Unknown Message Type", 400);
                break;
            }
        }

    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        send_reject(client_address, 0, "UNKNOWN", "Invalid JSON format", 400);
    } catch (const std::exception& e) {
        std::cerr << "Validation Error: " << e.what() << std::endl;
        send_reject(client_address, 0, "UNKNOWN", "Invalid Request Data", 400);
    }
}

// --- Handler: New Order ---
void RequestRouter::handle_new_order(const std::string& client_address, const NewOrderRequest& req) {
    if (req.qty <= 0) {
        send_reject(client_address, req.client_order_id, req.symbol, "Invalid Quantity", 101);
        return;
    }
    
    uint64_t internal_id = id_gen_.next();
    uint64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    Order new_order(
        internal_id, now, req.symbol, req.side, 
        static_cast<Type>(req.ord_type), req.limit_price, req.qty
    );

    // Send ACK immediately
    send_ack(client_address, req.client_order_id, internal_id, req.symbol);

    // Send to Engine 

    // TODO: Made the executor
    // engine_.add_order(new_order, client_address, req.client_order_id);
}

// --- Handler: Cancel ---
void RequestRouter::handle_cancel(const std::string& client_address, const CancelRequest& req) {
    // Pass to engine (Ack will come from Execution Report later)
    // engine_.cancel_order(req.order_id, req.symbol);
}

// --- Response Helpers ---

void RequestRouter::send_ack(const std::string& client_address, uint64_t client_order_id, uint64_t internal_order_id, const std::string& symbol) {
    Ack ack_msg;
    ack_msg.client_order_id = client_order_id;
    ack_msg.order_id = internal_order_id;
    ack_msg.symbol = symbol;
    send_to_client(client_address, ack_msg);
}

void RequestRouter::send_reject(const std::string& client_address, uint64_t client_order_id, const std::string& symbol, const std::string& reason, uint16_t error_code) {
    Reject rej_msg;
    rej_msg.client_order_id = client_order_id;
    rej_msg.symbol = symbol;
    rej_msg.info = { reason, error_code };
    send_to_client(client_address, rej_msg);
}

// --- Console Printer ---
void RequestRouter::send_to_client(const std::string& client_address, const OutboundMsg& msg) {
    json body;
    json header;
    
    std::visit([&](auto&& arg) {
        to_json(body, arg);
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Ack>) header["type"] = (int)MsgType::Ack;
        else if constexpr (std::is_same_v<T, Reject>) header["type"] = (int)MsgType::Reject;
        else if constexpr (std::is_same_v<T, Fill>) header["type"] = (int)MsgType::Fill;
    }, msg);

    json envelope;
    envelope["header"] = header;
    envelope["body"] = body;

    // Just print to console instead of ZMQ
    std::cout << ">>> OUTBOUND MSG [" << client_address << "] >>> " << envelope.dump() << std::endl;
}

}