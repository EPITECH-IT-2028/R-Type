#pragma once
#include "IPacket.hpp"

namespace packet {
  constexpr int ERROR = -1;

  class MessageHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };
}  // namespace packet
