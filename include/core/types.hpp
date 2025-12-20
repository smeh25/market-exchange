#pragma once
#include <cstdint>
#include <string>

namespace ex {

using ClientId = uint32_t;
using OrderId  = uint64_t;
using SeqNum   = uint64_t;

using Price = int64_t;
using Qty   = int64_t;

enum class Side : uint8_t { Buy = 1, Sell = 2 };
enum class OrdType : uint8_t { Market = 1, Limit = 2 };   
enum class TimeInForce : uint8_t { Day = 1, IOC = 2 };    

enum class MsgType : uint16_t {
  NewOrder = 1,
  Cancel  = 2,

  Ack     = 100,
  Reject  = 101,
  Fill    = 102,

  Heartbeat = 900
};

}
