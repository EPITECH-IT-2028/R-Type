#pragma once

#include <cstddef>
#include <cstdint>

enum class PacketType : uint8_t {
  Message = 0x01,
  Move = 0x02,
  NewPlayer = 0x03,
  PlayerInfo = 0x04,
  Position = 0x05,
  EnemySpawn = 0x06,
  EnemyMove = 0x07,
  EnemyDeath = 0x08,
  PlayerShoot = 0x09,
  ProjectileSpawn = 0x0A,
  ProjectileHit = 0x0B,
  ProjectileDestroy = 0x0C,
  GameStart = 0x0D,
  GameEnd = 0x0E
};

enum class EnemyType : uint8_t {
  BASIC_FIGHTER = 0x01
};

enum class ProjectileType : uint8_t {
  PLAYER_BASIC = 0x01,
  ENEMY_BASIC = 0x02
};

#define ALIGNED alignas(4)

/* both client and server packets */
struct ALIGNED PacketHeader {
    PacketType type;
    uint32_t size;
};

/* both client and server packets */
struct ALIGNED MessagePacket {
    PacketHeader header;
    uint32_t timestamp;
    char message[256];
};

/* Server to client packets */
struct ALIGNED MovePacket {
    PacketHeader header;
    uint32_t player_id;
    uint32_t sequence_number;
    float x;
    float y;
};

/* Server to client packets */
struct ALIGNED NewPlayerPacket {
    PacketHeader header;
    uint32_t player_id;
    float x;
    float y;
    float speed;
    uint32_t max_health;
};

/* Client to server packets */
struct ALIGNED PlayerInfoPacket {
    PacketHeader header;
    char name[32];
};

/* Client to server packets */
struct ALIGNED PositionPacket {
    PacketHeader header;
    uint32_t sequence_number;
    float x;
    float y;
};

/* Enemy Packets */
/* Server to client packets */
struct ALIGNED EnemySpawnPacket {
    PacketHeader header;
    uint32_t enemy_id;
    EnemyType enemy_type;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    uint32_t health;
    uint32_t max_health;
};

/* Server to client packets */
struct ALIGNED EnemyMovePacket {
    PacketHeader header;
    uint32_t enemy_id;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    uint32_t sequence_number;
};

/* Server to client packets */
struct ALIGNED EnemyDeathPacket {
    PacketHeader header;
    uint32_t enemy_id;
    float death_x;
    float death_y;
};

/* Projectile Packets */
/* Client to server packets */
struct ALIGNED PlayerShootPacket {
    PacketHeader header;
    float x;
    float y;
    ProjectileType projectile_type;
    uint32_t sequence_number;
};

/* Server to client packets */
struct ALIGNED ProjectileSpawnPacket {
    PacketHeader header;
    uint32_t projectile_id;
    ProjectileType projectile_type;
    uint32_t owner_id;
    uint8_t is_enemy_projectile;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    uint32_t damage;
};

/* Server to client packets */
struct ALIGNED ProjectileHitPacket {
    PacketHeader header;
    uint32_t projectile_id;
    uint32_t target_id;
    uint8_t target_is_player;
    float hit_x;
    float hit_y;
};

/* Server to client packets */
struct ALIGNED ProjectileDestroyPacket {
    PacketHeader header;
    uint32_t projectile_id;
    float x;
    float y;
};

/* Server to client packets */
struct ALIGNED GameStartPacket {
    PacketHeader header;
    uint8_t game_start;
};

/* Server to client packets */
struct ALIGNED GameEndPacket {
    PacketHeader header;
    uint8_t game_end;
};
