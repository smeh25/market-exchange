#pragma once

#include "core/message.hpp"
#include "core/types.hpp"

#include <string>
#include <variant>

namespace ex {

// ===================== INBOUND (client -> venue) =====================

struct NewOrderRequest {
  uint64_t client_order_id = 0;
  std::string symbol;
  Side side = Side::Buy;
  OrdType ord_type = OrdType::Limit;
  Qty qty = 0;

  // For LMT only (ignored for MKT).
  Price limit_price = 0;

  // Optional (default Day)
  TimeInForce tif = TimeInForce::Day;
};

struct CancelRequest {
  // allow cancel by venue order id OR by client_order_id
  OrderId order_id = 0;
  uint64_t client_order_id = 0;
  std::string symbol;
};

// ===================== OUTBOUND (venue -> client) =====================

struct Ack {
  uint64_t client_order_id = 0;
  OrderId order_id = 0;
  std::string symbol;
};

struct Reject {
  uint64_t client_order_id = 0;
  std::string symbol;
  RejectInfo info;
};

struct Fill {
  OrderId order_id = 0;
  std::string symbol;
  Side side = Side::Buy;
  Qty fill_qty = 0;
  Price fill_price = 0;
  bool complete = false;
};

using InboundMsg  = std::variant<NewOrderRequest, CancelRequest>;
using OutboundMsg = std::variant<Ack, Reject, Fill>;

} // namespace ex
