#pragma once
#include "IPacket.hpp"

namespace packet {
  class MessageHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };
}  // namespace packet
