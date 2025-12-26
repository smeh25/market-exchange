#pragma once

#include "lib/nlohmann/json.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

#include "core/types.hpp"
#include "core/message.hpp"
#include "core/messages.hpp"

namespace ex {

using nlohmann::json;

// =============================================================================
// JSON wire format (matches jsonExample.txt)
//
// {
//   "header": {"version":1,"type":1,"seq":12,"client_id":7},
//   "body": {
//     "client_order_id": 999,
//     "symbol":"AAPL",
//     "side":"B",
//     "ord_type":"LMT",
//     "qty":10,
//     "limit_price":10123
//   }
// }
// =============================================================================

template <class E>
constexpr std::underlying_type_t<E> to_u(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

// -----------------------------------------------------------------------------
// Enum <-> JSON (canonical output uses short codes from the example)
//   Side:        "B" | "S"
//   OrdType:     "MKT" | "LMT"
//   TimeInForce: "DAY" | "IOC"
//   MsgType:     numeric (uint16) in header
// -----------------------------------------------------------------------------

// ----- Side -----
inline std::string side_to_code(Side s) {
  switch (s) {
    case Side::Buy:  return "B";
    case Side::Sell: return "S";
  }
  throw std::runtime_error("Invalid Side");
}

inline Side side_from_any(const json& j) {
  if (j.is_number_integer()) {
    const int v = j.get<int>();
    if (v == 1) return Side::Buy;
    if (v == 2) return Side::Sell;
  }
  const std::string s = j.get<std::string>();
  if (s == "B" || s == "Buy" || s == "BUY" || s == "bid" || s == "Bid") return Side::Buy;
  if (s == "S" || s == "Sell" || s == "SELL" || s == "ask" || s == "Ask") return Side::Sell;
  throw std::runtime_error("Invalid Side: " + s);
}

// ----- OrdType -----
inline std::string ordtype_to_code(OrdType t) {
  switch (t) {
    case OrdType::Market: return "MKT";
    case OrdType::Limit:  return "LMT";
  }
  throw std::runtime_error("Invalid OrdType");
}

inline OrdType ordtype_from_any(const json& j) {
  if (j.is_number_integer()) {
    const int v = j.get<int>();
    if (v == 1) return OrdType::Market;
    if (v == 2) return OrdType::Limit;
  }
  const std::string s = j.get<std::string>();
  if (s == "MKT" || s == "Market" || s == "MARKET") return OrdType::Market;
  if (s == "LMT" || s == "Limit" || s == "LIMIT") return OrdType::Limit;
  throw std::runtime_error("Invalid OrdType: " + s);
}

// ----- TimeInForce -----
inline std::string tif_to_code(TimeInForce tif) {
  switch (tif) {
    case TimeInForce::Day: return "DAY";
    case TimeInForce::IOC: return "IOC";
  }
  throw std::runtime_error("Invalid TimeInForce");
}

inline TimeInForce tif_from_any(const json& j) {
  if (j.is_number_integer()) {
    const int v = j.get<int>();
    if (v == 1) return TimeInForce::Day;
    if (v == 2) return TimeInForce::IOC;
  }
  const std::string s = j.get<std::string>();
  if (s == "DAY" || s == "Day") return TimeInForce::Day;
  if (s == "IOC") return TimeInForce::IOC;
  throw std::runtime_error("Invalid TimeInForce: " + s);
}

// ----- MsgType -----
inline MsgType msgtype_from_any(const json& j) {
  if (j.is_number_integer()) {
    const int v = j.get<int>();
    return static_cast<MsgType>(static_cast<uint16_t>(v));
  }
  const std::string s = j.get<std::string>();
  if (s == "NewOrder" || s == "NewOrderRequest" || s == "NEW_ORDER") return MsgType::NewOrder;
  if (s == "Cancel" || s == "CANCEL") return MsgType::Cancel;
  if (s == "Ack" || s == "ACK") return MsgType::Ack;
  if (s == "Reject" || s == "REJECT") return MsgType::Reject;
  if (s == "Fill" || s == "FILL") return MsgType::Fill;
  if (s == "Heartbeat" || s == "HEARTBEAT") return MsgType::Heartbeat;
  throw std::runtime_error("Invalid MsgType: " + s);
}

// nlohmann/json hooks
inline void to_json(json& j, const Side& s) { j = side_to_code(s); }
inline void from_json(const json& j, Side& s) { s = side_from_any(j); }

inline void to_json(json& j, const OrdType& t) { j = ordtype_to_code(t); }
inline void from_json(const json& j, OrdType& t) { t = ordtype_from_any(j); }

inline void to_json(json& j, const TimeInForce& t) { j = tif_to_code(t); }
inline void from_json(const json& j, TimeInForce& t) { t = tif_from_any(j); }

// IMPORTANT: header.type is numeric in your example
inline void to_json(json& j, const MsgType& t) { j = to_u(t); }
inline void from_json(const json& j, MsgType& t) { t = msgtype_from_any(j); }

// -----------------------------------------------------------------------------
// MessageHeader
// -----------------------------------------------------------------------------
inline void to_json(json& j, const MessageHeader& h) {
  j = json{
    {"version",   h.version},
    {"type",      h.type},
    {"seq",       h.seq},
    {"client_id", h.client_id}
  };
}

inline void from_json(const json& j, MessageHeader& h) {
  if (j.contains("version")) j.at("version").get_to(h.version);
  j.at("type").get_to(h.type);
  j.at("seq").get_to(h.seq);
  if (j.contains("client_id")) j.at("client_id").get_to(h.client_id);
}

// -----------------------------------------------------------------------------
// RejectInfo
// -----------------------------------------------------------------------------
inline void to_json(json& j, const RejectInfo& r) {
  j = json{{"reason", r.reason}, {"code", r.code}};
}

inline void from_json(const json& j, RejectInfo& r) {
  if (j.contains("reason")) j.at("reason").get_to(r.reason);
  if (j.contains("code")) j.at("code").get_to(r.code);
}

// -----------------------------------------------------------------------------
// NewOrderRequest (body matches example)
// -----------------------------------------------------------------------------
inline void to_json(json& j, const NewOrderRequest& r) {
  j = json{
    {"client_order_id", r.client_order_id},
    {"symbol", r.symbol},
    {"side", r.side},
    {"ord_type", r.ord_type},
    {"qty", r.qty},
    {"limit_price", r.limit_price}
  };

  // only include tif if it isn't the default (keeps JSON minimal)
  if (r.tif != TimeInForce::Day) j["tif"] = r.tif;
}

inline void from_json(const json& j, NewOrderRequest& r) {
  if (j.contains("client_order_id")) j.at("client_order_id").get_to(r.client_order_id);
  j.at("symbol").get_to(r.symbol);
  j.at("side").get_to(r.side);
  j.at("ord_type").get_to(r.ord_type);
  j.at("qty").get_to(r.qty);

  if (j.contains("limit_price")) j.at("limit_price").get_to(r.limit_price);
  // accept alias "price"
  if (!j.contains("limit_price") && j.contains("price")) j.at("price").get_to(r.limit_price);

  if (j.contains("tif")) j.at("tif").get_to(r.tif);
}

// -----------------------------------------------------------------------------
// CancelRequest
// -----------------------------------------------------------------------------
inline void to_json(json& j, const CancelRequest& r) {
  j = json{};
  if (r.order_id != 0) j["order_id"] = r.order_id;
  if (r.client_order_id != 0) j["client_order_id"] = r.client_order_id;
  if (!r.symbol.empty()) j["symbol"] = r.symbol;
}

inline void from_json(const json& j, CancelRequest& r) {
  if (j.contains("order_id")) j.at("order_id").get_to(r.order_id);
  if (j.contains("client_order_id")) j.at("client_order_id").get_to(r.client_order_id);
  if (j.contains("symbol")) j.at("symbol").get_to(r.symbol);
}

// -----------------------------------------------------------------------------
// Ack / Reject / Fill (outbound)
// -----------------------------------------------------------------------------
inline void to_json(json& j, const Ack& a) {
  j = json{
    {"client_order_id", a.client_order_id},
    {"order_id", a.order_id},
    {"symbol", a.symbol}
  };
}

inline void from_json(const json& j, Ack& a) {
  if (j.contains("client_order_id")) j.at("client_order_id").get_to(a.client_order_id);
  if (j.contains("order_id")) j.at("order_id").get_to(a.order_id);
  if (j.contains("symbol")) j.at("symbol").get_to(a.symbol);
}

inline void to_json(json& j, const Reject& r) {
  j = json{
    {"client_order_id", r.client_order_id},
    {"symbol", r.symbol},
    {"info", r.info}
  };
}

inline void from_json(const json& j, Reject& r) {
  if (j.contains("client_order_id")) j.at("client_order_id").get_to(r.client_order_id);
  if (j.contains("symbol")) j.at("symbol").get_to(r.symbol);
  if (j.contains("info")) j.at("info").get_to(r.info);
}

inline void to_json(json& j, const Fill& f) {
  j = json{
    {"order_id", f.order_id},
    {"symbol", f.symbol},
    {"side", f.side},
    {"fill_qty", f.fill_qty},
    {"fill_price", f.fill_price},
    {"complete", f.complete}
  };
}

inline void from_json(const json& j, Fill& f) {
  if (j.contains("order_id")) j.at("order_id").get_to(f.order_id);
  if (j.contains("symbol")) j.at("symbol").get_to(f.symbol);
  if (j.contains("side")) j.at("side").get_to(f.side);
  if (j.contains("fill_qty")) j.at("fill_qty").get_to(f.fill_qty);
  if (j.contains("fill_price")) j.at("fill_price").get_to(f.fill_price);
  if (j.contains("complete")) j.at("complete").get_to(f.complete);
}

// -----------------------------------------------------------------------------
// Envelope helpers (one-call parse/dump for ZMQ request/reply)
// -----------------------------------------------------------------------------

struct EnvelopeIn {
  MessageHeader header;
  InboundMsg body;
};

struct EnvelopeOut {
  MessageHeader header;
  OutboundMsg body;
};

inline void to_json(json& j, const EnvelopeIn& e) {
  j = json{{"header", e.header}};
  std::visit([&](const auto& msg) { j["body"] = msg; }, e.body);
}

inline void from_json(const json& j, EnvelopeIn& e) {
  j.at("header").get_to(e.header);
  const auto& b = j.at("body");

  switch (e.header.type) {
    case MsgType::NewOrder: e.body = b.get<NewOrderRequest>(); break;
    case MsgType::Cancel:   e.body = b.get<CancelRequest>();   break;
    default:
      throw std::runtime_error("Unsupported inbound MsgType: " +
                               std::to_string(to_u(e.header.type)));
  }
}

inline void to_json(json& j, const EnvelopeOut& e) {
  j = json{{"header", e.header}};
  std::visit([&](const auto& msg) { j["body"] = msg; }, e.body);
}

inline void from_json(const json& j, EnvelopeOut& e) {
  j.at("header").get_to(e.header);
  const auto& b = j.at("body");

  switch (e.header.type) {
    case MsgType::Ack:    e.body = b.get<Ack>();    break;
    case MsgType::Reject: e.body = b.get<Reject>(); break;
    case MsgType::Fill:   e.body = b.get<Fill>();   break;
    default:
      throw std::runtime_error("Unsupported outbound MsgType: " +
                               std::to_string(to_u(e.header.type)));
  }
}

inline EnvelopeIn parse_inbound_envelope(const std::string& raw) {
  return json::parse(raw).get<EnvelopeIn>();
}

inline std::string dump_envelope(const EnvelopeIn& e) {
  return json(e).dump();
}

inline std::string dump_envelope(const EnvelopeOut& e) {
  return json(e).dump();
}

} // namespace ex
