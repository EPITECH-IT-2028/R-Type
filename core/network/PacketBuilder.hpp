#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include "Packet.hpp"

struct PacketBuilder {
    static MessagePacket makeMessage(const std::string &msg) {
      MessagePacket packet{};
      packet.header.type = PacketType::Message;
      packet.header.size = sizeof(packet);
      packet.timestamp = static_cast<uint32_t>(time(nullptr));
      strncpy(packet.message, msg.c_str(), sizeof(packet.message) - 1);
      packet.message[sizeof(packet.message) - 1] = '\0';
      return packet;
    }

    static NewPlayerPacket makeNewPlayer(uint32_t player_id, float x, float y,
                                         float speed,
                                         uint32_t max_health = 100) {
      NewPlayerPacket packet{};
      packet.header.type = PacketType::NewPlayer;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      packet.speed = speed;
      packet.max_health = max_health;
      return packet;
    }

    static PositionPacket makePosition(float x, float y, uint32_t seq) {
      PositionPacket packet{};
      packet.header.type = PacketType::Position;
      packet.header.size = sizeof(packet);
      packet.x = x;
      packet.y = y;
      packet.sequence_number = seq;
      return packet;
    }

    static MovePacket makeMove(uint32_t player_id, uint32_t seq, float x,
                               float y) {
      MovePacket packet{};
      packet.header.type = PacketType::Move;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.sequence_number = seq;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    static PlayerInfoPacket makePlayerInfo(const std::string &name) {
      PlayerInfoPacket packet{};
      packet.header.type = PacketType::PlayerInfo;
      packet.header.size = sizeof(packet);
      strncpy(packet.name, name.c_str(), sizeof(packet.name) - 1);
      packet.name[sizeof(packet.name) - 1] = '\0';
      return packet;
    }

    static PlayerHitPacket makePlayerHit(std::uint32_t player_id,
                                         std::uint32_t damage, float x, float y,
                                         int sequence_number) {
      PlayerHitPacket packet{};
      packet.header.type = PacketType::PlayerHit;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.damage = damage;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;
      return packet;
    }

    static EnemySpawnPacket makeEnemySpawn(uint32_t enemy_id, EnemyType type,
                                           float x, float y, float vx, float vy,
                                           uint32_t health,
                                           uint32_t max_health) {
      EnemySpawnPacket packet{};
      packet.header.type = PacketType::EnemySpawn;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.enemy_type = type;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = vx;
      packet.velocity_y = vy;
      packet.health = health;
      packet.max_health = max_health;
      return packet;
    }

    static EnemyMovePacket makeEnemyMove(uint32_t enemy_id, float x, float y,
                                         float velocity_x, float velocity_y,
                                         int seq) {
      EnemyMovePacket packet{};
      packet.header.type = PacketType::EnemyMove;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = velocity_x;
      packet.velocity_y = velocity_y;
      packet.sequence_number = seq;
      return packet;
    }

    static EnemyDeathPacket makeEnemyDeath(uint32_t enemy_id, float death_x,
                                           float death_y) {
      EnemyDeathPacket packet{};
      packet.header.type = PacketType::EnemyDeath;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.death_x = death_x;
      packet.death_y = death_y;
      return packet;
    }

    static EnemyHitPacket makeEnemyHit(uint32_t enemy_id, float hit_x,
                                       float hit_y, float damage,
                                       int sequence_number) {
      EnemyHitPacket packet{};
      packet.header.type = PacketType::EnemyHit;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.hit_x = hit_x;
      packet.hit_y = hit_y;
      packet.damage = damage;
      packet.sequence_number = sequence_number;
      return packet;
    }

    static PlayerShootPacket makePlayerShoot(float x, float y,
                                             ProjectileType projectile_type,
                                             uint32_t seq) {
      PlayerShootPacket packet{};
      packet.header.type = PacketType::PlayerShoot;
      packet.header.size = sizeof(packet);
      packet.x = x;
      packet.y = y;
      packet.projectile_type = projectile_type;
      packet.sequence_number = seq;
      return packet;
    }

    static ProjectileSpawnPacket makeProjectileSpawn(
        uint32_t projectile_id, ProjectileType type, float x, float y,
        float vel_x, float vel_y, bool is_enemy, uint32_t damage,
        uint32_t owner_id) {
      ProjectileSpawnPacket packet{};
      packet.header.type = PacketType::ProjectileSpawn;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.projectile_type = type;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = vel_x;
      packet.velocity_y = vel_y;
      packet.is_enemy_projectile = is_enemy;
      packet.damage = damage;
      packet.owner_id = owner_id;
      return packet;
    }

    static ProjectileHitPacket makeProjectileHit(uint32_t projectile_id,
                                                 uint32_t target_id,
                                                 float hit_x, float hit_y,
                                                 uint8_t target_is_player) {
      ProjectileHitPacket packet{};
      packet.header.type = PacketType::ProjectileHit;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.target_id = target_id;
      packet.hit_x = hit_x;
      packet.hit_y = hit_y;
      packet.target_is_player = target_is_player;
      return packet;
    }

    static ProjectileDestroyPacket makeProjectileDestroy(uint32_t projectile_id,
                                                         float x, float y) {
      ProjectileDestroyPacket packet{};
      packet.header.type = PacketType::ProjectileDestroy;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    static GameStartPacket makeGameStart(bool started) {
      GameStartPacket packet{};
      packet.header.type = PacketType::GameStart;
      packet.header.size = sizeof(packet);
      packet.game_start = started;
      return packet;
    }

    static GameEndPacket makeGameEnd(bool ended) {
      GameEndPacket packet{};
      packet.header.type = PacketType::GameEnd;
      packet.header.size = sizeof(packet);
      packet.game_end = ended;
      return packet;
    }

    static PlayerDeathPacket makePlayerDeath(uint32_t player_id, float x,
                                             float y) {
      PlayerDeathPacket packet{};
      packet.header.type = PacketType::PlayerDeath;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    static PlayerDisconnectPacket makePlayerDisconnect(uint32_t player_id) {
      PlayerDisconnectPacket packet{};
      packet.header.type = PacketType::PlayerDisconnected;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    static HeartbeatPlayerPacket makeHeartbeatPlayer(uint32_t player_id) {
      HeartbeatPlayerPacket packet{};
      packet.header.type = PacketType::Heartbeat;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }
};
