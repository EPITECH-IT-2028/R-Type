#pragma once

#include <cstddef>

namespace client {
  class Client;
}

namespace packet {
  class IPacket {
    public:
      virtual ~IPacket() = default;

      virtual int handlePacket(client::Client &client, const char *data,
                               std::size_t size) = 0;
  };
}  // namespace packet
