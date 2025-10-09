#pragma once
#include "IPacket.hpp"

namespace packet {
  constexpr int KO = -1;

  class MessageHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };
}  // namespace packet
