#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <algorithm>
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
/**
 * @brief Serializes a PacketHeader by writing its type and size fields.
 *
 * Serializes `type` as one byte followed by `size` as four bytes in that order.
 *
 * @param packet PacketHeader whose `type` and `size` fields will be written.
 */
void serialize(S &s, PacketHeader &packet) {
  s.value1b(packet.type);
  s.value4b(packet.size);
}

template <typename S>
/**
 * @brief Serialize a ChatMessagePacket into the provided serializer.
 *
 * Serializes the packet fields in the following order: header.type,
 * header.size, timestamp, player_id, message text stored in a fixed 512-byte
 * field (truncated or padded as needed), color components `r`, `g`, `b`, `a`,
 * and sequence_number.
 *
 * @param packet ChatMessagePacket to serialize.
 */
void serialize(S &s, ChatMessagePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.timestamp);
  s.value4b(packet.player_id);
  s.text1b(packet.message, SERIALIZE_512_BYTES);
  s.value1b(packet.r);
  s.value1b(packet.g);
  s.value1b(packet.b);
  s.value1b(packet.a);
  s.value4b(packet.sequence_number);
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
  s.text1b(packet.name, SERIALIZE_32_BYTES);
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
/**
 * @brief Serializes a PlayerDisconnectPacket into the provided serializer.
 *
 * The serialized fields, in order, are: header.type, header.size, player_id,
 * and sequence_number.
 *
 * @param packet Packet instance to serialize.
 */
void serialize(S &s, PlayerDisconnectPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.value4b(packet.sequence_number);
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
 * @brief Serialize a NewPlayerPacket into the serializer using the protocol's
 * binary layout.
 *
 * Writes fields in order: header.type, header.size, player_id, player_name as a
 * 32-byte field, x, y, speed as 32-bit floats, sequence_number, and max_health.
 *
 * @param packet The NewPlayerPacket to be serialized.
 */
void serialize(S &s, NewPlayerPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.text1b(packet.player_name, SERIALIZE_32_BYTES);
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
/**
 * @brief Serializes an EnemyMovePacket into the provided serializer.
 *
 * Serializes fields in order: header.type (1 byte), header.size (4 bytes),
 * enemy_id (4 bytes), x (32-bit float), y (32-bit float), velocity_x (32-bit
 * float), velocity_y (32-bit float), and sequence_number (4 bytes).
 *
 * @tparam S Serializer type.
 * @param s Serializer instance to write into.
 * @param packet EnemyMovePacket to serialize.
 */
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
 * @brief Serializes a ProjectileSpawnPacket into the serializer using network
 * wire order.
 *
 * Writes the packet header followed by payload fields in this order:
 * projectile_id, projectile_type, owner_id, is_enemy_projectile,
 * x, y, velocity_x, velocity_y, speed, sequence_number, and damage.
 *
 * @tparam S Serializer type.
 * @param s Serializer instance that receives the serialized bytes.
 * @param packet ProjectileSpawnPacket to serialize.
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
/**
 * @brief Serializes a ProjectileHitPacket into the provided serializer.
 *
 * Serializes the header (type and size), projectile and target IDs, the
 * target-is-player flag, and the hit coordinates in that order.
 *
 * @param s Serialization adapter.
 * @param packet Packet to serialize.
 */
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
 * @brief Serialize a ProjectileDestroyPacket into the serializer.
 *
 * Serializes the packet fields in order: header.type (1 byte), header.size (4
 * bytes), projectile_id (4 bytes), x (32-bit float), y (32-bit float), and
 * sequence_number (4 bytes).
 *
 * @param s Serializer adapter to write the packet data into.
 * @param packet ProjectileDestroyPacket to serialize.
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
/**
 * @brief Serializes a PlayerHitPacket into the provided serializer.
 *
 * Writes the packet header (type and size), player ID, x and y coordinates
 * (as 32-bit floats), damage, and sequence number in that order.
 *
 * @param packet The PlayerHitPacket to serialize.
 */
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
 * @brief Serialize a PlayerDeathPacket into the serializer.
 *
 * Serializes the packet header, then the following fields in order: player_id,
 * x and y coordinates as 32-bit floats, and sequence_number.
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
 * @brief Serializes the GameStartPacket into the given serializer.
 *
 * Serializes the packet header (type and size), the packet's sequence_number,
 * and the game_start flag.
 *
 * @tparam S Serializer type.
 * @param s Serializer instance to write into.
 * @param packet Packet to serialize.
 */
void serialize(S &s, GameStartPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.value1b(packet.game_start);
}

template <typename S>
/**
 * @brief Serialize a GameEndPacket into the serializer.
 *
 * Serializes the header's type and size, then the packet's sequence_number and
 * game_end flag.
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
 * @brief Serializes a CreateRoomPacket into the provided serializer in network
 * packet order.
 *
 * Serializes the header (type, size), `room_name` as a 32-byte field,
 * `max_players`, `is_private`, `password` as a 64-byte field, and
 * `sequence_number`, in that order.
 *
 * @param packet The CreateRoomPacket to serialize.
 */
void serialize(S &s, CreateRoomPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.text1b(packet.room_name, SERIALIZE_32_BYTES);
  s.value1b(packet.max_players);
  s.value1b(packet.is_private);
  s.text1b(packet.password, SERIALIZE_64_BYTES);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a JoinRoomPacket into the provided serializer.
 *
 * Writes header.type (1 byte), header.size (4 bytes), room_id (4 bytes),
 * sequence_number (4 bytes), and the password as a fixed 64-byte field.
 *
 * @param packet JoinRoomPacket to serialize.
 */
void serialize(S &s, JoinRoomPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.room_id);
  s.value4b(packet.sequence_number);
  s.text1b(packet.password, SERIALIZE_64_BYTES);
}

template <typename S>
/**
 * @brief Serializes a JoinRoomResponsePacket into the provided serializer.
 *
 * Serializes the packet header, then the response `error_code`, and finally the
 * `sequence_number`, in that order.
 *
 * @param packet Packet to serialize; its header, `error_code`, and
 * `sequence_number` are written to the serializer.
 */
void serialize(S &s, JoinRoomResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.error_code);
  s.value4b(packet.sequence_number);
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
  s.text1b(room.room_name, SERIALIZE_32_BYTES);
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
 * @brief Serialize a MatchmakingRequestPacket into the serializer.
 *
 * Writes the packet header's `type` and `size`, then the packet's
 * `sequence_number`.
 *
 * @param packet The matchmaking request packet to serialize.
 */
void serialize(S &s, MatchmakingRequestPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serialize a MatchmakingResponsePacket into the serializer.
 *
 * Writes the packet fields in order: header.type (1 byte), header.size (4
 * bytes), error_code (1 byte), and sequence_number (4 bytes).
 *
 * @param s Serializer adapter to write bytes into.
 * @param packet Matchmaking response packet to serialize.
 */
void serialize(S &s, MatchmakingResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.error_code);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a PlayerInputPacket into the provided archive.
 *
 * Writes the header fields (type, size), the input byte, and the packet's
 * sequence_number in that order.
 *
 * @param packet Packet to serialize.
 */
void serialize(S &s, PlayerInputPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.input);
  s.value4b(packet.sequence_number);
}

template<typename S>
/**
 * @brief Serializes a PingPacket into the provided serializer.
 *
 * Writes the packet header (type and size) followed by the timestamp.
 *
 * @param s Serializer to write into.
 * @param packet PingPacket to serialize.
 */
void serialize(S& s, PingPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.timestamp);
}

template<typename S>
/**
 * @brief Serializes a PongPacket into the provided serializer.
 * Writes the packet header (type and size) followed by the timestamp.
 * @param s Serializer to write into.
 * @param packet PongPacket to serialize.
 */
void serialize(S& s, PongPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.timestamp);
}
template <typename S>
/**
 * @brief Serialize an AckPacket into the serializer.
 *
 * Writes the packet fields in order: header.type (1 byte), header.size (4
 * bytes), sequence_number (4 bytes), and player_id (4 bytes).
 *
 * @param s Serializer adapter to write data into.
 * @param packet AckPacket to serialize.
 */
void serialize(S &s, AckPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.value4b(packet.player_id);
}

template <typename S>
/**
 * @brief Serializes a RequestChallengePacket into the serializer.
 *
 * The serialized field order is: header.type (1 byte), header.size (4 bytes),
 * room_id (4 bytes), and sequence_number (4 bytes).
 */
void serialize(S &s, RequestChallengePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.room_id);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a ChallengeResponsePacket into the provided serializer.
 *
 * Writes the packet header (type and size), the 128-byte `challenge` field,
 * `timestamp`, and `sequence_number` to the serializer in that order.
 *
 * @param packet Packet whose fields will be written to the serializer.
 */
void serialize(S &s, ChallengeResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.text1b(packet.challenge, SERIALIZE_128_BYTES);
  s.value4b(packet.timestamp);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a CreateRoomResponsePacket into the provided serializer.
 *
 * Serializes fields in order: header.type (1 byte), header.size (4 bytes),
 * error_code (1 byte), room_id (4 bytes), and sequence_number (4 bytes).
 */
void serialize(S &s, CreateRoomResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.error_code);
  s.value4b(packet.room_id);
  s.value4b(packet.sequence_number);
}

/**
 * @brief Serializes a ScoreEntry structure.
 *
 * Writes the player name (up to 32 bytes) and the score (4 bytes).
 *
 * @param s Serializer adapter.
 * @param entry ScoreEntry to serialize.
 */
template <typename S>
void serialize(S &s, ScoreEntry &entry) {
  s.text1b(entry.player_name, SERIALIZE_32_BYTES);
  s.value4b(entry.score);
}

/**
 * @brief Serializes a ScoreboardRequestPacket.
 *
 * Writes the packet header (type and size) and the limit value.
 *
 * @param s Serializer adapter.
 * @param packet ScoreboardRequestPacket to serialize.
 */
template <typename S>
void serialize(S &s, ScoreboardRequestPacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.limit);
}

/**
 * @brief Serializes a ScoreboardResponsePacket.
 *
 * Writes the packet header (type and size), the entry count, and then
 * iterates through the scores vector to serialize each ScoreEntry.
 *
 * @param s Serializer adapter.
 * @param packet ScoreboardResponsePacket to serialize.
 */
template <typename S>
void serialize(S &s, ScoreboardResponsePacket &packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.entry_count);

  packet.entry_count = std::min(packet.entry_count, SCOREBOARD_MAX_ENTRIES);

  s.container(packet.scores, packet.entry_count,
              [](S &s, ScoreEntry &entry) { serialize(s, entry); });
  packet.entry_count = static_cast<uint32_t>(packet.scores.size());
}
