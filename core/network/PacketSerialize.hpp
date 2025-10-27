#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <vector>
#include "Macro.hpp"
#include "Packet.hpp"

namespace serialization {
  using Buffer = std::vector<std::uint8_t>;
  using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
  using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
}  // namespace serialization

/*
 * Common Packets
 */
template <typename S>
void serialize(S &s, PacketHeader &packet) {
  s.value1b(packet.type);
  s.value4b(packet.size);
}

template <typename S>
/**
 * @brief Serializes a MessagePacket into the provided serializer.
 *
 * Writes the packet header (type and size), timestamp, the 256-byte message
 * payload, player identifier, and sequence number into the serializer stream.
 *
 * @param packet Packet instance whose fields will be serialized into `s`.
 */
void serialize(S &s, MessagePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.timestamp);
  for (size_t i = 0; i < 256; ++i) {
    s.value1b(packet.message[i]);
  }
  s.value4b(packet.sequence_number);
  s.value4b(packet.player_id);
}

/*
 * Client to Server Packets
 */
template <typename S>
/**
 * @brief Serializes a PlayerInfoPacket into the provided serializer.
 *
 * Writes the packet header fields, the player's name as exactly 32 bytes,
 * and the packet's sequence_number.
 *
 * @param packet The PlayerInfoPacket whose header, 32-byte name array, and
 *               sequence_number will be written to the serializer.
 */
void serialize(S &s, PlayerInfoPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(packet.name[i]);
  }
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a PlayerShootPacket into the provided serializer.
 *
 * Writes the packet header (type and size), the shoot position (`x`, `y`) as
 * raw floats, the `projectile_type`, and the `sequence_number` to the
 * serializer.
 *
 * @param s Serializer adapter used to write the packet fields.
 * @param packet PlayerShootPacket instance to serialize.
 */
void serialize(S &s, PlayerShootPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value1b(packet.projectile_type);
  s.value4b(packet.sequence_number);
}

template <typename S>
void serialize(S &s, PlayerDisconnectPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
}

template <typename S>
void serialize(S &s, HeartbeatPlayerPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
}

/*
 * Server to Client Packets
 */
template <typename S>
/**
 * @brief Serializes a PlayerMovePacket into the given serialization state.
 *
 * @param s Serializer state or adapter used to write packet fields.
 * @param packet Player movement packet whose header, identifiers, and position
 * fields are written.
 */
void serialize(S &s, PlayerMovePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.value4b(packet.sequence_number);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
}

template <typename S>
/**
 * @brief Serializes a NewPlayerPacket into the provided serializer.
 *
 * Writes the packet's header, player identifier, position (x, y), speed,
 * maximum health, and sequence number to the serializer in that order.
 *
 * @param s Serializer adapter used to write packet data.
 * @param packet The NewPlayerPacket to serialize.
 */
void serialize(S &s, NewPlayerPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.template value<sizeof(float)>(packet.speed);
  s.value4b(packet.sequence_number);
  s.value4b(packet.max_health);
}

/*
 * Enemy Packets (Server to Client)
 */
template <typename S>
/**
 * @brief Serializes an EnemySpawnPacket into the provided serializer.
 *
 * Writes the packet fields in network order: header.type, header.size,
 * enemy_id, enemy_type, x, y, velocity_x, velocity_y, health, max_health,
 * and sequence_number.
 *
 * @param s Serializer adapter to write bytes into.
 * @param packet EnemySpawnPacket instance to serialize.
 */
void serialize(S &s, EnemySpawnPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.enemy_id);
  s.value1b(packet.enemy_type);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.template value<sizeof(float)>(packet.velocity_x);
  s.template value<sizeof(float)>(packet.velocity_y);
  s.value4b(packet.sequence_number);
  s.value4b(packet.health);
  s.value4b(packet.max_health);
}

template <typename S>
void serialize(S &s, EnemyMovePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.enemy_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.template value<sizeof(float)>(packet.velocity_x);
  s.template value<sizeof(float)>(packet.velocity_y);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes an EnemyDeathPacket into the provided serializer.
 *
 * Writes the packet fields in network order: header.type (1 byte), header.size
 * (4 bytes), enemy_id (4 bytes), death_x (raw float), death_y (raw float),
 * player_id (4 bytes), score (4 bytes), and sequence_number (4 bytes).
 */
void serialize(S &s, EnemyDeathPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.enemy_id);
  s.template value<sizeof(float)>(packet.death_x);
  s.template value<sizeof(float)>(packet.death_y);
  s.value4b(packet.player_id);
  s.value4b(packet.score);
  s.value4b(packet.sequence_number);
}

template <typename S>
void serialize(S &s, EnemyHitPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.enemy_id);
  s.template value<sizeof(float)>(packet.hit_x);
  s.template value<sizeof(float)>(packet.hit_y);
  s.template value<sizeof(float)>(packet.damage);
  s.value4b(packet.sequence_number);
}

/*
 * Projectile Event Packets (Server to Client)
 */
template <typename S>
/**
 * @brief Serializes a ProjectileSpawnPacket into the provided serializer in
 * wire order.
 *
 * Serializes the packet's header then the payload fields in the following
 * order: projectile_id, projectile_type, owner_id, is_enemy_projectile,
 * sequence_number, x, y, velocity_x, velocity_y, speed, and damage.
 *
 * @tparam S Serializer type.
 * @param s Serializer instance receiving the serialized data.
 * @param packet Projectile spawn packet to serialize.
 */
void serialize(S &s, ProjectileSpawnPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.projectile_id);
  s.value1b(packet.projectile_type);
  s.value4b(packet.owner_id);
  s.value1b(packet.is_enemy_projectile);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.template value<sizeof(float)>(packet.velocity_x);
  s.template value<sizeof(float)>(packet.velocity_y);
  s.template value<sizeof(float)>(packet.speed);
  s.value4b(packet.sequence_number);
  s.value4b(packet.damage);
}

template <typename S>
void serialize(S &s, ProjectileHitPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.projectile_id);
  s.value4b(packet.target_id);
  s.value1b(packet.target_is_player);
  s.template value<sizeof(float)>(packet.hit_x);
  s.template value<sizeof(float)>(packet.hit_y);
}

template <typename S>
/**
 * @brief Serializes a ProjectileDestroyPacket into the provided serializer.
 *
 * Writes the packet's header.type (1 byte), header.size (4 bytes),
 * projectile_id (4 bytes), sequence_number (4 bytes), and the x and y
 * coordinates as raw 32-bit floats.
 *
 * @param s Serializer adapter to write the packet data into.
 * @param packet ProjectileDestroyPacket instance to serialize.
 */
void serialize(S &s, ProjectileDestroyPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.projectile_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value4b(packet.sequence_number);
}

/*
 * Player Event Packets (Server to Client)
 */
template <typename S>
void serialize(S &s, PlayerHitPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value4b(packet.damage);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a PlayerDeathPacket into the provided serializer.
 *
 * Writes the packet header (type as 1 byte, size as 4 bytes), then the
 * player_id (4 bytes), sequence_number (4 bytes), and the death position
 * coordinates x and y as raw 32-bit floats.
 *
 * @param packet The PlayerDeathPacket to serialize.
 */
void serialize(S &s, PlayerDeathPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value4b(packet.sequence_number);
}

/*
 * Game State Packets (Server to Client)
 */
template <typename S>
/**
 * @brief Serializes a GameStartPacket into the provided serializer.
 *
 * Writes the packet header (type and size), the game_start flag, and the
 * packet's sequence_number into the serializer stream.
 *
 * @tparam S Serializer type.
 * @param s Serializer instance to write into.
 * @param packet GameStartPacket whose fields will be serialized.
 */
void serialize(S &s, GameStartPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.value1b(packet.game_start);
}

template <typename S>
/**
 * @brief Serialize a GameEndPacket into the provided serializer.
 *
 * Writes the packet header (type and size), the `game_end` flag, and the
 * packet's `sequence_number`.
 *
 * @param packet The GameEndPacket to serialize into `s`.
 */
void serialize(S &s, GameEndPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.value1b(packet.game_end);
}

/*
 * Room Packets
 */
template <typename S>
/**
 * @brief Serializes a CreateRoomPacket into the provided serializer.
 *
 * Writes the packet header followed by a fixed 32-byte room name, the
 * maximum player count, the privacy flag, and a fixed 32-byte password.
 *
 * @param packet Packet instance whose fields will be serialized.
 */
void serialize(S &s, CreateRoomPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(packet.room_name[i]);
  }
  s.value1b(packet.is_private);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(packet.password[i]);
  }
  s.value1b(packet.max_players);
}

template <typename S>
/**
 * @brief Serializes a JoinRoomPacket into the given serializer.
 *
 * Writes the packet header (type as 1 byte, size as 4 bytes), the room ID (4
 * bytes), and the 32-byte password as a sequence of bytes.
 *
 * @param packet The JoinRoomPacket to serialize.
 */
void serialize(S &s, JoinRoomPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.room_id);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(packet.password[i]);
  }
}

template <typename S>
/**
 * @brief Serializes a join-room response packet into the given serializer.
 *
 * Writes the packet header fields (type as 1 byte, size as 4 bytes) followed by
 * the response error code (1 byte).
 *
 * @param packet Join room response packet containing the header and
 * `error_code`.
 */
void serialize(S &s, JoinRoomResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.error_code);
}

template <typename S>
/**
 * @brief Serializes a LeaveRoomPacket into the provided serializer.
 *
 * Writes the packet header fields followed by the room identifier to the
 * serializer in the packet's wire order.
 *
 * @param packet The LeaveRoomPacket to serialize; its header (type, size) and
 * `room_id` are written.
 */
void serialize(S &s, LeaveRoomPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.room_id);
}

template <typename S>
/**
 * @brief Serialize a ListRoomPacket into the provided serializer.
 *
 * Writes the packet's header fields (type and size) to the serializer.
 */
void serialize(S &s, ListRoomPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
}

template <typename S>
/**
 * @brief Serializes a RoomInfo into the serializer in wire order.
 *
 * Serializes room_id (4 bytes), room_name as 32 bytes, player_count (1 byte),
 * then max_players (1 byte).
 *
 * @param room RoomInfo instance whose fields will be serialized.
 */
void serialize(S &s, RoomInfo &room) {
  s.value4b(room.room_id);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(room.room_name[i]);
  }
  s.value1b(room.player_count);
  s.value1b(room.max_players);
}

template <typename S>
/**
 * @brief Serializes a list-room response packet into the serializer.
 *
 * Serializes the packet header, the room count, and each RoomInfo entry in
 * the fixed-size rooms array (MAX_ROOMS entries).
 *
 * @param packet Packet containing the header, room_count, and rooms to
 * serialize.
 */
void serialize(S &s, ListRoomResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.room_count);
  for (size_t i = 0; i < MAX_ROOMS; ++i) {
    serialize(s, packet.rooms[i]);
  }
}

template <typename S>
/**
 * @brief Serializes the matchmaking request packet header into the serializer.
 *
 * @param packet MatchmakingRequestPacket whose header `type` (1 byte) and
 * `size` (4 bytes) are written to the serializer.
 */
void serialize(S &s, MatchmakingRequestPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
}

template <typename S>
/**
 * @brief Serializes a MatchmakingResponsePacket into the serializer.
 *
 * Writes the packet header fields (type and size) followed by the packet's
 * `error_code`.
 *
 * @param s Serializer to write bytes into.
 * @param packet Matchmaking response packet to serialize.
 */
void serialize(S &s, MatchmakingResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.error_code);
}

template <typename S>
/**
 * @brief Serialize a PlayerInputPacket into the given serializer.
 *
 * @param s Serializer/archive to write into.
 * @param packet PlayerInputPacket to serialize.
 */
void serialize(S &s, PlayerInputPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.input);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes an AckPacket into the provided serializer.
 *
 * Writes the packet header's type and size, then serializes the player ID
 * and the packet's sequence number.
 *
 * @param s Serializer/stream adapter to write serialized data into.
 * @param packet AckPacket instance to serialize.
 */
void serialize(S &s, AckPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.value4b(packet.player_id);
}
