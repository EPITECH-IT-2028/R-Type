#pragma once
#include "APacket.hpp"

namespace packet {

  class MessageHandler : public APacket {
    public:
      int handlePacket(client::Client &client,
                       const char *data, std::size_t size) override;
  };

}  // namespace packet
