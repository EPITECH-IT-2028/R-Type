#pragma once

#include "IPacket.hpp"

namespace packet {

  class APacket : public IPacket {
    public:
      virtual ~APacket() = default;

      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override = 0;
  };

}  // namespace packet
