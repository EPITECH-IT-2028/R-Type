#pragma once

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
  #endif
#endif

#include <asio.hpp>
#include "BaseNetworkManager.hpp"
#include "Serializer.hpp"

namespace packet {

  class PacketSender {
    public:
      template <typename T>
      static void sendPacket(network::BaseNetworkManager &networkManager,
                             const T &packet) {
        auto buffer = std::make_shared<std::vector<std::uint8_t>>(
            serialization::BitserySerializer::serialize(packet));

        networkManager.send(buffer);
      }
  };
}  // namespace packet
