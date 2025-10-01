#pragma once

#include "IPacket.hpp"

namespace packet {

  class APacket : public IPacket {
    public:
      virtual ~APacket() = default;

      virtual int handlePacket(client::Client &client,
                               const char *data, std::size_t size) override = 0;
  };

}  // namespace packet
