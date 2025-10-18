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
  GameEnd = 0x0E,
  PlayerDisconnected = 0x0F,
  Heartbeat = 0x10,
  EnemyHit = 0x11,
  PlayerHit = 0x12,
  PlayerDeath = 0x13,
  CreateRoom = 0x14,
  JoinRoom = 0x15,
  LeaveRoom = 0x16,
  ListRoom = 0x17,
  ListRoomResponse = 0x18,
  MatchmakingRequest = 0x19,
  MatchmakingResponse = 0x1A
};

enum class EnemyType : uint8_t {
  BASIC_FIGHTER = 0x01
};

enum class ProjectileType : uint8_t {
  PLAYER_BASIC = 0x01,
  ENEMY_BASIC = 0x02
};

enum class RoomError : uint8_t {
  SUCCESS = 0x00,
  ROOM_NOT_FOUND = 0x01,
  ROOM_FULL = 0x02,
  WRONG_PASSWORD = 0x03,
  ALREADY_IN_ROOM = 0x04,
  PLAYER_BANNED = 0x05
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
    uint32_t player_id;
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
struct ALIGNED PlayerDisconnectPacket {
    PacketHeader header;
    uint32_t player_id;
};

/* Client to server packets */
struct ALIGNED HeartbeatPlayerPacket {
    PacketHeader header;
    uint32_t player_id;
};

/**
 * @brief Packet sent from the client to the server to provide the player's
 * display name.
 *
 * Contains the common packet header and a fixed-size name buffer. The name is
 * stored as a null-terminated UTF-8 string in the 32-byte `name` field; maximum
 * 31 bytes of character data plus a terminating NUL.
 *
 * @var char PlayerInfoPacket::name
 * Player's display name (null-terminated UTF-8). */
struct ALIGNED PlayerInfoPacket {
    PacketHeader header;
    char name[32];
};

/**
 * @brief Packet sent from client to server to report a player hit event.
 *
 * Contains the common packet header and the identifying and contextual data for
 * a hit: the affected player's id, the damage amount, the world coordinates
 * where the hit occurred, and a sequence number to correlate with client-side
 * action/state.
 *
 * @param player_id Identifier of the player that was hit.
 * @param damage Amount of damage applied to the player.
 * @param x World X coordinate where the hit occurred.
 * @param y World Y coordinate where the hit occurred.
 * @param sequence_number Client-side sequence number used to correlate this
 * event with prior actions.
 */
struct ALIGNED PlayerHitPacket {
    PacketHeader header;
    uint32_t player_id;
    uint32_t damage;
    float x;
    float y;
    int sequence_number;
};

/**
 * @brief Sends the client's current position with an ordering sequence number.
 *
 * Packet used by the client to report its position to the server.
 * From server to client.
 *
 * Fields:
 *  - header: Common packet header identifying the packet type and payload size.
 *  - sequence_number: Client-side sequence number for ordering position
 * updates.
 *  - x: X coordinate of the player's position.
 *  - y: Y coordinate of the player's position.
 */
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

/**
 * @brief Describes an enemy death event sent from server to client.
 *
 * Starts with a PacketHeader identifying the packet type and payload size.
 *
 * Fields:
 * - header: PacketHeader present in all packets.
 * - enemy_id: Server-assigned identifier for the enemy that died.
 * - death_x: X coordinate of the death location in world space.
 * - death_y: Y coordinate of the death location in world space.
 * - player_id: Identifier of the player credited for the kill.
 * - score: Score awarded for the kill.
 */
struct ALIGNED EnemyDeathPacket {
    PacketHeader header;
    uint32_t enemy_id;
    float death_x;
    float death_y;
    std::uint32_t player_id;
    std::uint32_t score;
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
    float speed;
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

/**
 * @brief Signals the end of the game to the client.
 *
 * Contains the common packet header and a flag indicating whether the game has
 * ended.
 *
 * @var PacketHeader GameEndPacket::header
 *   Common packet header identifying packet type and payload size.
 * @var uint8_t GameEndPacket::game_end
 *   `1` if the game has ended, `0` otherwise.
 */
struct ALIGNED GameEndPacket {
    PacketHeader header;
    uint8_t game_end;
};

/**
 * @brief Notifies the client that an enemy was hit, including hit location,
 * damage, and ordering.
 *
 * Contains the packet header and data describing which enemy was hit, where the
 * hit occurred, how much damage was applied, and a sequence number for ordering
 * or reconciliation.
 *
 * @var header
 * PacketHeader common to all packets (type and payload size).
 *
 * @var enemy_id
 * Identifier of the enemy that was hit.
 *
 * @var hit_x
 * X coordinate of the hit position.
 *
 * @var hit_y
 * Y coordinate of the hit position.
 *
 * @var damage
 * Amount of damage inflicted by the hit.
 *
 * @var sequence_number
 * Sequence number used to order or reconcile hit events.
 */
struct ALIGNED EnemyHitPacket {
    PacketHeader header;
    std::uint32_t enemy_id;
    float hit_x;
    float hit_y;
    float damage;
    int sequence_number;
};

/**
 * @brief Informs clients that a player has died and where the death occurred.
 *
 * @details Carries the player identifier and the world coordinates of the death
 * location.
 *
 * @var std::uint32_t PlayerDeathPacket::player_id
 * ID of the player who died.
 *
 * @var float PlayerDeathPacket::x
 * X coordinate of the death location.
 *
 * @var float PlayerDeathPacket::y
 * Y coordinate of the death location.
 */
struct ALIGNED PlayerDeathPacket {
    PacketHeader header;
    std::uint32_t player_id;
    float x;
    float y;
};

/* Room Management Packets */
struct ALIGNED CreateRoomPacket {
    PacketHeader header;
    char room_name[32];
    bool is_private;
    char password[32];
    uint32_t max_players;
};

struct ALIGNED JoinRoomPacket {
    PacketHeader header;
    uint32_t room_id;
    char password[32];
};

struct ALIGNED JoinRoomResponsePacket {
    PacketHeader header;
    RoomError error_code;
};

struct ALIGNED LeaveRoomPacket {
    PacketHeader header;
    uint32_t room_id;
};

struct ALIGNED ListRoomPacket {
    PacketHeader header;
};

struct ALIGNED RoomInfo {
    uint32_t room_id;
    char room_name[32];
    uint32_t player_count;
    uint32_t max_players;
};

struct ALIGNED ListRoomResponsePacket {
    PacketHeader header;
    uint32_t room_count;
    RoomInfo rooms[10];
};

struct ALIGNED MatchmakingRequestPacket {
    PacketHeader header;
};

struct ALIGNED MatchmakingResponsePacket {
    PacketHeader header;
    RoomError error_code;
};
