#pragma once
#include "core/Types.hpp"
#include <string>

namespace ex {

struct MessageHeader {
  uint16_t version = 1;
  MsgType  type;
  SeqNum   seq;         
  ClientId client_id;
};

struct RejectInfo {
  std::string reason;
  int code = 0;
};

} 
