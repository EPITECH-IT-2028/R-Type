#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <vector>
#include "Packet.hpp"

namespace serialization {
  using Buffer = std::vector<uint8_t>;
  using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
  using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
}  // namespace serialization

/*
 * Common Packets
 */
template <typename S>
/**
 * @brief Serializes a PacketHeader into the provided serializer by writing its type and size fields.
 *
 * @tparam S Serializer adapter type used by the serialization framework.
 * @param packet PacketHeader whose `type` (1 byte) and `size` (4 bytes) are written.
 */
void serialize(S& s, PacketHeader& packet) {
  s.value1b(packet.type);
  s.value4b(packet.size);
}

template <typename S>
/**
 * @brief Serializes a MessagePacket into the provided serializer.
 *
 * Encodes the packet fields in order: header.type (1 byte), header.size (4 bytes),
 * timestamp (4 bytes), and the 256-byte message payload (each byte serialized).
 *
 * @tparam S Serializer type providing `value1b` and `value4b` methods used to encode bytes and 4-byte values.
 * @param s Serializer instance to receive the encoded data.
 * @param packet MessagePacket instance whose fields will be serialized.
 */
void serialize(S& s, MessagePacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.timestamp);
  for (size_t i = 0; i < 256; ++i) {
    s.value1b(packet.message[i]);
  }
}

/*
 * Client to Server Packets
 */
template <typename S>
/**
 * @brief Serializes a PlayerInfoPacket into the provided Bitsery serializer.
 *
 * Writes the header's `type` as 1 byte, `size` as 4 bytes, then the 32-byte
 * `name` array in order.
 *
 * @param packet PlayerInfoPacket to serialize (header and fixed-size name).
 */
void serialize(S& s, PlayerInfoPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(packet.name[i]);
  }
}

template <typename S>
/**
 * @brief Serializes a PositionPacket into the given Bitsery serializer.
 *
 * Writes the packet's header.type, header.size, sequence_number, and the
 * x/y coordinates as 4-byte floats in that order.
 *
 * @param packet The PositionPacket to serialize.
 */
void serialize(S& s, PositionPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
}

template <typename S>
/**
 * @brief Writes the fields of a PlayerShootPacket to the provided serializer.
 *
 * @tparam S Serializer type (Bitsery-style) used to encode packet fields.
 * @param s Serializer instance that will receive the packet data.
 * @param packet PlayerShootPacket whose header and payload fields will be serialized.
 */
void serialize(S& s, PlayerShootPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value1b(packet.projectile_type);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a PlayerDisconnectPacket into the given Bitsery serializer.
 *
 * Serializes the packet's header fields (type and size) followed by the player_id payload.
 *
 * @param packet The PlayerDisconnectPacket to serialize.
 */
void serialize(S& s, PlayerDisconnectPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
}

template <typename S>
/**
 * @brief Serializes a player heartbeat packet (header and player identifier).
 *
 * Serializes the packet's header (type and size) followed by the player_id field.
 *
 * @param packet The HeartbeatPlayerPacket to serialize; its header.type, header.size, and player_id are written.
 */
void serialize(S& s, HeartbeatPlayerPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
}

/*
 * Server to Client Packets
 */
template <typename S>
/**
 * @brief Serializes or deserializes the fields of a MovePacket using a Bitsery stream.
 *
 * @tparam S Bitsery serialization context (serializer/deserializer adapter).
 * @param s Bitsery stream adapter used to read/write values.
 * @param packet Packet instance whose header and payload fields (header.type, header.size, player_id, sequence_number, x, y) are serialized or deserialized.
 */
void serialize(S& s, MovePacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.value4b(packet.sequence_number);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
}

template <typename S>
/**
 * @brief Serializes a NewPlayerPacket into or from the Bitsery serializer.
 *
 * @tparam S Serializer type used by Bitsery (e.g., an output or input adapter).
 * @param packet Packet whose fields (header, identifiers, position, speed, and health) are serialized.
 */
void serialize(S& s, NewPlayerPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.template value<sizeof(float)>(packet.speed);
  s.value4b(packet.max_health);
}

/*
 * Enemy Packets (Server to Client)
 */
template <typename S>
/**
 * @brief Serializes or deserializes an EnemySpawnPacket using the provided serializer.
 *
 * @tparam S Serializer type (e.g., a Bitsery serializer/adapter).
 * @param s Serializer instance that performs the (de)serialization.
 * @param packet Enemy spawn packet whose fields are (de)serialized in packet field order.
 */
void serialize(S& s, EnemySpawnPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.enemy_id);
  s.value1b(packet.enemy_type);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.template value<sizeof(float)>(packet.velocity_x);
  s.template value<sizeof(float)>(packet.velocity_y);
  s.value4b(packet.health);
  s.value4b(packet.max_health);
}

template <typename S>
/**
 * @brief Serializes an EnemyMovePacket into the provided Bitsery serializer.
 *
 * Writes the packet's header and payload fields in the order required for network transmission.
 *
 * @param packet The enemy movement packet to serialize.
 */
void serialize(S& s, EnemyMovePacket& packet) {
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
 * @brief Serializes an EnemyDeathPacket into the provided serialization stream.
 *
 * Serializes the packet header (type, size) followed by enemy_id, death_x, death_y,
 * player_id, and score in that order.
 *
 * @tparam S Serialization stream/adapter type (e.g., a Bitsery serializer).
 * @param packet The EnemyDeathPacket to serialize or deserialize.
 */
void serialize(S& s, EnemyDeathPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.enemy_id);
  s.template value<sizeof(float)>(packet.death_x);
  s.template value<sizeof(float)>(packet.death_y);
  s.value4b(packet.player_id);
  s.value4b(packet.score);
}

template <typename S>
/**
 * @brief Serializes an EnemyHitPacket into the provided Bitsery serializer.
 *
 * @tparam S Serializer/archive type compatible with Bitsery value handlers.
 * @param packet Packet containing enemy hit data to serialize.
 */
void serialize(S& s, EnemyHitPacket& packet) {
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
 * @brief Serializes a ProjectileSpawnPacket into the provided serializer.
 *
 * Writes the packet header (type and size) followed by projectile_id, projectile_type,
 * owner_id, is_enemy_projectile, position (x, y), velocity (velocity_x, velocity_y),
 * speed, and damage in that order.
 *
 * @param packet The ProjectileSpawnPacket whose fields will be serialized into `s`.
 */
void serialize(S& s, ProjectileSpawnPacket& packet) {
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
  s.value4b(packet.damage);
}

template <typename S>
/**
 * @brief Serializes a ProjectileHitPacket into the serializer stream.
 *
 * Writes the packet header followed by the projectile identifier, target identifier,
 * a flag indicating whether the target is a player, and the hit coordinates.
 *
 * @param packet The packet to serialize; its serialized form includes:
 *               - header.type and header.size
 *               - projectile_id
 *               - target_id
 *               - target_is_player (1 byte)
 *               - hit_x and hit_y (float-sized values)
 */
void serialize(S& s, ProjectileHitPacket& packet) {
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
 * Serializes the packet header fields (type and size), then the projectile_id
 * and the packet's x and y coordinates as 4-byte floating-point values.
 *
 * @tparam S Serializer type that provides Bitsery-style value/valueN methods.
 * @param packet The ProjectileDestroyPacket to serialize.
 */
void serialize(S& s, ProjectileDestroyPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.projectile_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
}

/*
 * Player Event Packets (Server to Client)
 */
template <typename S>
/**
 * @brief Serializes a PlayerHitPacket into the provided serializer.
 *
 * Writes the packet header (type and size) followed by player_id, damage,
 * x and y coordinates, and sequence_number in the order expected on the wire.
 *
 * @param packet The PlayerHitPacket to serialize or deserialize.
 */
void serialize(S& s, PlayerHitPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.value4b(packet.damage);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value4b(packet.sequence_number);
}

template <typename S>
/**
 * @brief Serializes a PlayerDeathPacket into the serializer.
 *
 * Writes the packet header (type and size) followed by player_id, x, and y.
 *
 * @param packet The PlayerDeathPacket to serialize.
 */
void serialize(S& s, PlayerDeathPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
}

/*
 * Game State Packets (Server to Client)
 */
template <typename S>
/**
 * @brief Serializes a GameStartPacket into the serializer.
 *
 * Writes the packet header (type and size) followed by the game_start flag.
 *
 * @tparam S Serializer type.
 * @param s Serializer instance.
 * @param packet Packet to serialize.
 */
void serialize(S& s, GameStartPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.game_start);
}

template <typename S>
/**
 * @brief Serializes a GameEndPacket into the provided serializer.
 *
 * Writes the packet header fields and the game end flag.
 *
 * @param packet The packet containing the header and `game_end` flag to serialize.
 */
void serialize(S& s, GameEndPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.game_end);
}