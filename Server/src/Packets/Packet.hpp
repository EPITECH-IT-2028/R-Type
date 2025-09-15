#pragma once

#include <cstdint>

constexpr uint8_t PACKET_TYPE_MESSAGE = 0x01;
constexpr uint8_t PACKET_TYPE_MOVE = 0x02;
constexpr uint8_t PACKET_TYPE_NEW_PLAYER = 0x03;
constexpr uint8_t PACKET_TYPE_PLAYER_INFO = 0x04;
constexpr uint8_t PACKET_TYPE_PLAYER_POSITION = 0x05;

#define ALIGNED __attribute__((aligned(4)))

struct ALIGNED PacketHeader {
    uint8_t type;
    uint32_t size;
};

struct ALIGNED MessagePacket {
    PacketHeader header;
    uint32_t timestamp;
    char message[256];
};

struct ALIGNED MovePacket {
    PacketHeader header;
    uint32_t player_id;
    int sequence_number;
    float x;
    float y;
};

struct ALIGNED NewPlayerPacket {
    PacketHeader header;
    uint32_t player_id;
    float x;
    float y;
    float speed;
};

struct ALIGNED PlayerInfoPacket {
    PacketHeader header;
    char name[32];
};

struct ALIGNED PositionPacket {
    PacketHeader header;
    int sequence_number;
    float x;
    float y;
};

namespace packet {

  class PacketSender {
    public:
  };

  class PacketReceiver {
    public:
      static int receivePacket(int socket, void *packet, size_t size);
  };
}  // namespace packet
