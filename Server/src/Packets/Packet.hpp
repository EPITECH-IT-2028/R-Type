#pragma once

#include <cstddef>
#include <cstdint>

enum class PacketType : uint8_t {
  Message = 0x01,
  Move = 0x02,
  NewPlayer = 0x03,
  PlayerInfo = 0x04,
  Position = 0x05,
};

#define ALIGNED alignas(4)

struct ALIGNED PacketHeader {
    PacketType type;
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
  };
}  // namespace packet
