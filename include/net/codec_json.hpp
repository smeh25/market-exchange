#pragma once
#include "core/Messages.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace ex{
    using json = nlohmann::json;

    // headers

    inline void to_json(json& j, const MessageHeader& h){
        j = json{{"version", h.version}, {"type", (int)h.type}, {"seq", h.seq}, {"client_id", h.client_id}};
    }

    inline void from_json(const json& j, MessageHeader& h){
        h.version = j.at("version").get<uint16_t>();
        h.type = (MsgType)j.at("type").get<int>();
        h.seq = j.at("seq").get<SeqNum>();
        h.client_id = j.at("client_id").get<ClientId>();
    }

    // requests

    inline void from_json(const json& j, NewOrderRequest& m){
        m.client_order_id = j.at("client_order_id").get<uint64_t>();
        m.symbol          = j.at("symbol").get<std::string>();
        m.side            = side_from(j.at("side").get<std::string>());
        m.ord_type        = ordtype_from(j.at("ord_type").get<std::string>());
        m.qty             = j.at("qty").get<Qty>();
        m.limit_price     = j.value("limit_price", 0);
    }

    inline void from_json(const json& j, CancelRequest& m) {
        m.order_id        = j.value("order_id", 0);
        m.client_order_id = j.value("client_order_id", 0);
        m.symbol          = j.at("symbol").get<std::string>();
    }

    // responses

    inline void to_json(json& j, const Ack& m) {
        j = json{{"client_order_id", m.client_order_id}, {"order_id", m.order_id}, {"symbol", m.symbol}};
    }

    inline void to_json(json& j, const Reject& m) {
        j = json{{"client_order_id", m.client_order_id}, {"symbol", m.symbol},
            {"reason", m.info.reason}, {"code", m.info.code}};
    }

    inline void to_json(json& j, const Fill& m) {
        j = json{{"order_id", m.order_id}, {"symbol", m.symbol},
            {"side", to_string(m.side)}, {"fill_qty", m.fill_qty},
            {"fill_price", m.fill_price}, {"complete", m.complete}};
    }

}