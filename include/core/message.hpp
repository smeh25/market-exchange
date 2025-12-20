#pragma once

#include "types.hpp"
#include <string>

namespace ex {

struct MessageHeader {
  uint16_t version = 1;
  MsgType  type = MsgType::Heartbeat;
  SeqNum   seq = 0;
  ClientId client_id = 0;
};

struct RejectInfo {
  std::string reason;
  int code = 0;
};

} // namespace ex
