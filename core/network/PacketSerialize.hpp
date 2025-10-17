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
void serialize(S& s, PacketHeader& packet) {
  s.value1b(packet.type);
  s.value4b(packet.size);
}

template <typename S>
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
void serialize(S& s, PlayerInfoPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  for (size_t i = 0; i < 32; ++i) {
    s.value1b(packet.name[i]);
  }
}

template <typename S>
void serialize(S& s, PositionPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.sequence_number);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
}

template <typename S>
void serialize(S& s, PlayerShootPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.template value<sizeof(float)>(packet.x);
  s.template value<sizeof(float)>(packet.y);
  s.value1b(packet.projectile_type);
  s.value4b(packet.sequence_number);
}

template <typename S>
void serialize(S& s, PlayerDisconnectPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
}

template <typename S>
void serialize(S& s, HeartbeatPlayerPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
}

/*
 * Server to Client Packets
 */
template <typename S>
void serialize(S& s, MovePacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.value4b(packet.sequence_number);
  s.template value<sizeof(int)>(packet.x);
  s.template value<sizeof(int)>(packet.y);
}

template <typename S>
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
void serialize(S& s, GameStartPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.game_start);
}

template <typename S>
void serialize(S& s, GameEndPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.game_end);
}

template <typename S>
void serialize(S& s, InputPlayerPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value1b(packet.input);
  s.value4b(packet.sequence_number);
}

template <typename S>
void serialize(S& s, PositionPlayerPacket& packet) {
  s.value1b(packet.header.type);
  s.value4b(packet.header.size);
  s.value4b(packet.player_id);
  s.value4b(packet.x);
  s.value4b(packet.y);
}
