#pragma once
#include "core/message.hpp"
#include "core/types.hpp"
#include <string>
#include <variant>

namespace ex {

struct NewOrderRequest {
  uint64_t client_order_id = 0;

  std::string symbol;   
  Side side;
  OrdType ord_type;

  Qty qty = 0;
  Price limit_price = 0;     
  TimeInForce tif = TimeInForce::Day; // how long order should be in order book if not fulfilled
};

struct CancelRequest {
  OrderId order_id = 0;         
  uint64_t client_order_id = 0; 
  std::string symbol;
};

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

struct Fill { // partially filled order
  OrderId order_id = 0;
  std::string symbol;
  Side side;
  Qty fill_qty = 0;
  Price fill_price = 0;
  bool complete = false;  
};

using InboundMsg  = std::variant<NewOrderRequest, CancelRequest>;
using OutboundMsg = std::variant<Ack, Reject, Fill>;

} 
