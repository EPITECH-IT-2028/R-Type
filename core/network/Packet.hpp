#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "Macro.hpp"

enum class PacketType : std::uint8_t {
  ChatMessage = 0x01,
  PlayerMove = 0x02,
  NewPlayer = 0x03,
  PlayerInfo = 0x04,
  EnemySpawn = 0x05,
  EnemyMove = 0x06,
  EnemyDeath = 0x07,
  PlayerShoot = 0x08,
  ProjectileSpawn = 0x09,
  ProjectileHit = 0x0A,
  ProjectileDestroy = 0x0B,
  GameStart = 0x0C,
  GameEnd = 0x0D,
  PlayerDisconnected = 0x0E,
  Heartbeat = 0x0F,
  EnemyHit = 0x10,
  PlayerHit = 0x11,
  PlayerDeath = 0x12,
  CreateRoom = 0x13,
  JoinRoom = 0x14,
  LeaveRoom = 0x15,
  ListRoom = 0x16,
  ListRoomResponse = 0x17,
  MatchmakingRequest = 0x18,
  MatchmakingResponse = 0x19,
  JoinRoomResponse = 0x1A,
  PlayerInput = 0x1B,
  RequestChallenge = 0x1C,
  ChallengeResponse = 0x1D,
  CreateRoomResponse = 0x1E,
  Ping = 0x1F,
  Pong = 0x20,
  Ack = 0x21,
  ScoreboardRequest = 0x22,
  ScoreboardResponse = 0x23
};

enum class EnemyType : std::uint8_t {
  BASIC_FIGHTER = 0x01
};

enum class ProjectileType : std::uint8_t {
  PLAYER_BASIC = 0x01,
  ENEMY_BASIC = 0x02
};

enum class RoomError : std::uint8_t {
  SUCCESS = 0x00,
  ROOM_NOT_FOUND = 0x01,
  ROOM_FULL = 0x02,
  WRONG_PASSWORD = 0x03,
  ALREADY_IN_ROOM = 0x04,
  PLAYER_BANNED = 0x05,
  UNKNOWN_ERROR = 0x06
};

enum class MovementInputType : std::uint8_t {
  UP = 1 << 0,
  DOWN = 1 << 1,
  LEFT = 1 << 2,
  RIGHT = 1 << 3
};

#define ALIGNED alignas(8)

/**
 * @brief 8-byte-aligned header present at the start of every network packet.
 *
 * Identifies the packet kind and records the total serialized packet size in
 * bytes (including this header) for routing and validation of incoming and
 * outgoing packet data.
 *
 * @var type
 *   PacketType value identifying the specific packet structure that follows.
 * @var size
 *   Total size of the serialized packet in bytes, including this header.
 */
struct ALIGNED PacketHeader {
    PacketType type;
    std::uint32_t size;
};

constexpr std::size_t HEADER_SIZE = sizeof(PacketType) + sizeof(std::uint32_t);

/**
 * @brief Packet that transmits a timestamped UTF-8 chat message with sender
 * identity and display color.
 *
 * Carries the message text, the sending player's identifier, an RGBA color to
 * render the message, a timestamp, and a per-packet sequence number for
 * ordering.
 *
 * @var header Common packet header containing the packet type and total
 * serialized size.
 * @var timestamp 32-bit timestamp associated with the message.
 * @var message UTF-8 encoded chat message (std::string; variable length).
 * @var player_id 32-bit identifier of the player who sent the message.
 * @var r Red color component (0–255).
 * @var g Green color component (0–255).
 * @var b Blue color component (0–255).
 * @var a Alpha (opacity) component (0–255).
 * @var sequence_number Per-packet ordering index used for reliability and
 * ordering.
 */
struct ALIGNED ChatMessagePacket {
    PacketHeader header;
    std::uint32_t timestamp;
    std::string message;
    std::uint32_t player_id;
    std::uint8_t r, g, b, a;
    std::uint32_t sequence_number;
};

/**
 * @brief Packet sent from server to client to convey a player's position
 * update.
 *
 * Carries the target player's identifier, an ordering sequence number, and the
 * player's world-space coordinates.
 *
 * @param header Common packet header containing packet type and payload size.
 * @param player_id Identifier of the player whose position is being reported.
 * @param sequence_number Sequence number used to order or correlate movement
 * updates.
 * @param x Player's X coordinate in world space.
 * @param y Player's Y coordinate in world space.
 */
struct ALIGNED PlayerMovePacket {
    PacketHeader header;
    std::uint32_t player_id;
    std::uint32_t sequence_number;
    float x;
    float y;
};

/**
 * @brief Notifies clients that a new player has spawned.
 *
 * Contains the server-assigned player ID, the player's display name, spawn
 * position, movement speed, and maximum health.
 *
 * @details
 * - header: Common packet header present at the start of every packet.
 * - player_id: Server-assigned unique player identifier.
 * - player_name: UTF-8 display name.
 * - x, y: Spawn world coordinates.
 * - speed: Movement speed scalar.
 * - sequence_number: Per-packet ordering index for reliable processing.
 * - max_health: Player's maximum health points.
 */
struct ALIGNED NewPlayerPacket {
    PacketHeader header;
    std::uint32_t player_id;
    std::string player_name;
    float x;
    float y;
    float speed;
    std::uint32_t sequence_number;
    std::uint32_t max_health;
};

/**
 * @brief Indicates a client-to-server notification that a player is
 * disconnecting.
 *
 * Contains the common packet header along with the identifier of the
 * disconnecting player and a per-packet sequence number used for
 * ordering/reliability.
 *
 * @param player_id Identifier of the player who is disconnecting.
 * @param sequence_number Per-packet ordering index for reliability.
 */
struct ALIGNED PlayerDisconnectPacket {
    PacketHeader header;
    std::uint32_t player_id;
    std::uint32_t sequence_number;
};

/**
 * @brief Heartbeat packet sent from client to server to indicate player
 * liveness.
 *
 * Contains the common packet header and the identifier of the player sending
 * the heartbeat.
 */
struct ALIGNED HeartbeatPlayerPacket {
    PacketHeader header;
    std::uint32_t player_id;
};

/**
 * @brief Client-to-server packet that conveys a player's display name.
 *
 * Includes the common packet header, the player's UTF-8 display name, and a
 * per-packet sequence number used for ordering/reliability. The `name` field is
 * truncated to 32 bytes during serialization.
 *
 * @var PacketHeader PlayerInfoPacket::header Common packet header identifying
 * the packet type and total serialized size.
 * @var std::string PlayerInfoPacket::name Player's display name encoded in
 * UTF-8; truncated to 32 bytes when serialized.
 * @var std::uint32_t PlayerInfoPacket::sequence_number Per-packet ordering
 * index for reliability.
 */
struct ALIGNED PlayerInfoPacket {
    PacketHeader header;
    std::string name;
    std::uint32_t sequence_number;
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
    std::uint32_t player_id;
    std::uint32_t damage;
    float x;
    float y;
    std::uint32_t sequence_number;
};

/* Enemy Packets */
/**
 * @brief Packet sent by the server to notify clients that an enemy has spawned.
 *
 * Contains the spawned enemy's identifier, variant, position, velocity,
 * sequencing index, and current/max health.
 *
 * Members:
 * - header: Common packet header (type and payload size).
 * - enemy_id: Unique identifier for the spawned enemy.
 * - enemy_type: Variant of the enemy.
 * - x, y: Spawn position coordinates.
 * - velocity_x, velocity_y: Velocity components.
 * - sequence_number: Per-packet ordering index for reliable sequencing.
 * - health: Current health of the enemy.
 * - max_health: Maximum health of the enemy.
 */
struct ALIGNED EnemySpawnPacket {
    PacketHeader header;
    std::uint32_t enemy_id;
    EnemyType enemy_type;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    std::uint32_t sequence_number;
    std::uint32_t health;
    std::uint32_t max_health;
};

/**
 * @brief Server-to-client update of an enemy's position, velocity, and sequence
 * order.
 *
 * Contains the common packet header, the enemy identifier, current position
 * (`x`, `y`), current velocity (`velocity_x`, `velocity_y`), and a
 * `sequence_number` used to order or reconcile updates.
 */
struct ALIGNED EnemyMovePacket {
    PacketHeader header;
    std::uint32_t enemy_id;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    std::uint32_t sequence_number;
};

/**
 * @brief Notifies clients that an enemy has died.
 *
 * Carries the enemy identifier, death world coordinates, the player credited
 * for the kill, the score awarded for the kill, and a per-packet sequence
 * number for ordering/acknowledgement.
 */
struct ALIGNED EnemyDeathPacket {
    PacketHeader header;
    std::uint32_t enemy_id;
    float death_x;
    float death_y;
    std::uint32_t player_id;
    std::uint32_t score;
    std::uint32_t sequence_number;
};

/* Projectile Packets */
/**
 * @brief Client-to-server packet reporting a player's shooting action.
 *
 * Contains the firing position, projectile type, and a sequence number used for
 * ordering and acknowledgement of the shoot event.
 *
 * Fields:
 * - header: Common packet header containing packet type and payload size.
 * - x: Firing X coordinate.
 * - y: Firing Y coordinate.
 * - projectile_type: Variant of projectile to spawn.
 * - sequence_number: Monotonic client sequence number for this action.
 */
struct ALIGNED PlayerShootPacket {
    PacketHeader header;
    float x;
    float y;
    ProjectileType projectile_type;
    std::uint32_t sequence_number;
};

/**
 * @brief Notifies clients that a projectile has spawned and conveys its
 * identity, ownership, motion, and damage.
 *
 * Carries the server-assigned projectile identifier, projectile variant, owner
 * identity, allegiance flag, world position, velocity components, scalar speed,
 * per-packet sequence number, and damage amount.
 *
 * @param header Packet header containing the packet type and payload size.
 * @param projectile_id Server-assigned unique identifier for the projectile.
 * @param projectile_type Variant/type of the projectile.
 * @param owner_id Identifier of the entity that spawned the projectile.
 * @param is_enemy_projectile Nonzero if the projectile was spawned by an enemy,
 * zero otherwise.
 * @param x World X coordinate of the projectile spawn position.
 * @param y World Y coordinate of the projectile spawn position.
 * @param velocity_x X component of the projectile's velocity.
 * @param velocity_y Y component of the projectile's velocity.
 * @param speed Scalar speed magnitude of the projectile.
 * @param sequence_number Per-packet ordering index associated with this spawn
 * event.
 * @param damage Damage value applied by the projectile on hit.
 */
struct ALIGNED ProjectileSpawnPacket {
    PacketHeader header;
    std::uint32_t projectile_id;
    ProjectileType projectile_type;
    std::uint32_t owner_id;
    std::uint8_t is_enemy_projectile;
    float x;
    float y;
    float velocity_x;
    float velocity_y;
    float speed;
    std::uint32_t sequence_number;
    std::uint32_t damage;
};

/**
 * @brief Notifies the client that a projectile impacted a target.
 *
 * Carries the projectile identifier, the impacted target identifier and type,
 * and the world coordinates of the impact.
 *
 * @var header Common packet header containing type and payload size.
 * @var projectile_id Identifier of the projectile that hit a target.
 * @var target_id Identifier of the target that was hit.
 * @var target_is_player `1` if the target is a player, `0` otherwise.
 * @var hit_x X coordinate of the impact in world space.
 * @var hit_y Y coordinate of the impact in world space.
 */
struct ALIGNED ProjectileHitPacket {
    PacketHeader header;
    std::uint32_t projectile_id;
    std::uint32_t target_id;
    std::uint8_t target_is_player;
    float hit_x;
    float hit_y;
};

/**
 * @brief Notifies clients that a projectile has been destroyed or expired.
 *
 * Contains the projectile's unique identifier, its last known world position,
 * and a per-packet sequence number for ordering.
 *
 * @var PacketHeader header Packet header identifying the packet type and total
 * payload size.
 * @var std::uint32_t projectile_id Unique identifier of the projectile.
 * @var float x World-space X coordinate where the projectile was destroyed.
 * @var float y World-space Y coordinate where the projectile was destroyed.
 * @var std::uint32_t sequence_number Per-packet ordering index.
 */
struct ALIGNED ProjectileDestroyPacket {
    PacketHeader header;
    std::uint32_t projectile_id;
    float x;
    float y;
    std::uint32_t sequence_number;
};

/**
 * @brief Announces the game's start state to a client.
 *
 * Contains the common packet header, a per-packet ordering index, and a flag
 * that indicates whether the game has started.
 *
 * @note `game_start` is `1` when the game has started, `0` otherwise.
 * @note `sequence_number` is a per-packet ordering index used for reliability
 * and ordering.
 */
struct ALIGNED GameStartPacket {
    PacketHeader header;
    std::uint32_t sequence_number;
    std::uint8_t game_start;
};

/**
 * @brief Notifies a client that the game has ended.
 *
 * Packet containing a header, a per-packet sequence number, and a flag
 * indicating game end.
 *
 * @var PacketHeader GameEndPacket::header Packet header identifying packet type
 * and payload size.
 * @var std::uint32_t GameEndPacket::sequence_number Per-packet ordering index.
 * @var std::uint8_t GameEndPacket::game_end `1` if the game has ended, `0`
 * otherwise.
 */
struct ALIGNED GameEndPacket {
    PacketHeader header;
    std::uint32_t sequence_number;
    std::uint8_t game_end;
};

/**
 * @brief Packet notifying clients that an enemy was hit, including hit
 * location, damage, and ordering information.
 *
 * Describes which enemy was hit, the world-space coordinates of the hit, the
 * damage applied, and a sequence number for ordering or reconciliation.
 *
 * @var header PacketHeader common to all packets (type and total serialized
 * size).
 * @var enemy_id Identifier of the enemy that was hit.
 * @var hit_x World-space X coordinate of the hit.
 * @var hit_y World-space Y coordinate of the hit.
 * @var damage Amount of damage applied by the hit.
 * @var sequence_number Sequence number used to order or reconcile hit events.
 */
struct ALIGNED EnemyHitPacket {
    PacketHeader header;
    std::uint32_t enemy_id;
    float hit_x;
    float hit_y;
    float damage;
    std::uint32_t sequence_number;
};

/**
 * @brief Notifies clients that a player died and provides the death location.
 *
 * Carries the player identifier and world coordinates of the death location,
 * and includes a per-packet sequence number for ordering.
 *
 * @var std::uint32_t PlayerDeathPacket::player_id ID of the player who died.
 * @var float PlayerDeathPacket::x X coordinate of the death location.
 * @var float PlayerDeathPacket::y Y coordinate of the death location.
 * @var std::uint32_t PlayerDeathPacket::sequence_number Per-packet ordering
 * index.
 */
struct ALIGNED PlayerDeathPacket {
    PacketHeader header;
    std::uint32_t player_id;
    float x;
    float y;
    std::uint32_t sequence_number;
};

/**
 * @brief Client request to create a room including access controls and
 * capacity.
 *
 * Carries the desired room display name, privacy flag, optional password,
 * maximum player count, and a per-packet sequence number for ordering.
 *
 * Fields:
 * - header: common PacketHeader placed at the start of every packet.
 * - room_name: UTF-8 display name; truncated to 32 bytes on serialization.
 * - is_private: `1` = private (password required), `0` = public.
 * - password: Password for private rooms; ignored for public rooms; truncated
 * to 32 bytes on serialization.
 * - max_players: Maximum number of players allowed in the room.
 * - sequence_number: Per-packet ordering index used for reliability.
 */
struct ALIGNED CreateRoomPacket {
    PacketHeader header;
    std::string room_name;
    std::uint8_t is_private;
    std::string password;
    std::uint8_t max_players;
    std::uint32_t sequence_number;
};

/**
 * @brief Server response to a create-room request indicating the result.
 *
 * Contains the room creation result code, the assigned room identifier when
 * creation succeeded, and a sequence number for ordering/reliability.
 *
 * @var header Common packet header containing the packet type and total
 * serialized size.
 * @var error_code Room creation result code.
 * @var room_id Assigned room identifier (valid when `error_code ==
 * RoomError::SUCCESS`).
 * @var sequence_number Per-packet ordering index used for reliability.
 */
struct ALIGNED CreateRoomResponsePacket {
    PacketHeader header;
    RoomError error_code;
    std::uint32_t room_id;
    std::uint32_t sequence_number;
};

/**
 * @brief Client-to-server request to join a room.
 *
 * Carries the common packet header, the numeric room identifier, an optional
 * UTF‑8 password, and a per-packet sequence number for ordering.
 *
 * @var header Common packet header identifying the packet type and total
 * serialized size (including the header).
 * @var room_id Numeric identifier of the room to join.
 * @var password UTF‑8 password for the room; empty when no password is
 * required. The password is truncated to 32 bytes when serialized.
 * @var sequence_number Per-packet ordering index used for reliability and
 *                      sequencing.
 */
struct ALIGNED JoinRoomPacket {
    PacketHeader header;
    std::uint32_t room_id;
    std::string password;
    std::uint32_t sequence_number;
};

/**
 * @brief Server response to a join-room request indicating success or the
 * failure reason.
 *
 * Contains the common packet header and a RoomError value describing the
 * outcome.
 *
 * - header: Common packet header identifying the packet type and payload size.
 * - error_code: `RoomError` value; `SUCCESS` means the join succeeded, other
 * values indicate the specific failure.
 */
struct ALIGNED JoinRoomResponsePacket {
    PacketHeader header;
    RoomError error_code;
    std::uint32_t sequence_number;
};

/**
 * @brief Client-to-server request to leave a room.
 *
 * Contains the common packet header and the identifier of the room the client
 * requests to leave.
 *
 * Fields:
 * - header: Common packet header indicating type and payload size.
 * - room_id: Identifier of the room to leave.
 */
struct ALIGNED LeaveRoomPacket {
    PacketHeader header;
    std::uint32_t room_id;
};

/**
 * @brief Client-to-server request to retrieve the list of available rooms.
 *
 * This packet contains only the standard packet header and signals the server
 * to respond with a ListRoomResponsePacket containing current room summaries.
 */
struct ALIGNED ListRoomPacket {
    PacketHeader header;
};

/**
 * @brief Describes a room's identity and current occupancy for listings.
 *
 * @var room_id Unique numeric identifier for the room.
 * @var room_name UTF-8 room name.
 * @var player_count Current number of players in the room.
 * @var max_players Maximum allowed players for the room.
 */
struct ALIGNED RoomInfo {
    std::uint32_t room_id;
    std::string room_name;
    std::uint8_t player_count;
    std::uint8_t max_players;
};

/**
 * @brief Server response containing a list of available rooms.
 *
 * Contains up to MAX_ROOMS room summaries and the count of valid entries.
 *
 * @var header Common packet header indicating packet type and payload size.
 * @var room_count Number of valid RoomInfo entries in `rooms` (0 through
 * MAX_ROOMS).
 * @var rooms Fixed-size array of room summaries; only the first `room_count`
 * entries are populated.
 */
struct ALIGNED ListRoomResponsePacket {
    PacketHeader header;
    std::uint32_t room_count;
    RoomInfo rooms[MAX_ROOMS];
};

/**
 * @brief Client-to-server request to initiate matchmaking.
 *
 * This packet contains only the common PacketHeader and signals that the sender
 * wants to be matched into a game lobby or match queue.
 */
struct ALIGNED MatchmakingRequestPacket {
    PacketHeader header;
    std::uint32_t sequence_number;
};

/**
 * @brief Server-to-client response to a matchmaking request indicating the
 * result.
 *
 * Contains the common packet header and a RoomError value that describes
 * whether matchmaking succeeded or the reason it failed.
 *
 * @var header Common packet header with packet type and payload size.
 * @var error_code RoomError value indicating matchmaking result (e.g.,
 * `SUCCESS`, `ROOM_NOT_FOUND`, `ROOM_FULL`, `WRONG_PASSWORD`,
 * `ALREADY_IN_ROOM`, `PLAYER_BANNED`, `UNKNOWN_ERROR`).
 */
struct ALIGNED MatchmakingResponsePacket {
    PacketHeader header;
    RoomError error_code;
    std::uint32_t sequence_number;
};

/**
 * @brief Conveys a client's directional input and its client-side sequence
 * number.
 *
 * Contains the packet header, an 8-bit bitfield of MovementInputType flags in
 * `input`, and `sequence_number` for ordering and acknowledgement correlation.
 */
struct ALIGNED PlayerInputPacket {
    PacketHeader header;
    std::uint8_t input;
    std::uint32_t sequence_number;
};

/**
 * @brief Ping packet sent from client to server to measure latency.
 *
 * Contains the common packet header and a timestamp representing when the ping was sent.
 */
struct ALIGNED PingPacket {
    PacketHeader header;
    std::uint32_t timestamp;
    std::uint32_t sequence_number;
};

/**
 * @brief Pong packet sent from server to client in response to a ping.
 *
 * Contains the common packet header and a timestamp representing when the original ping was received.
 */
struct ALIGNED PongPacket {
    PacketHeader header;
    std::uint32_t timestamp;
    std::uint32_t sequence_number;
};

/**
 * @brief Acknowledges receipt of a specific packet sequence number from a
 * player.
 *
 * Contains the common packet header and identifies the acknowledged sequence
 * number along with the player ID associated with that acknowledgement.
 *
 * @var PacketHeader header Common packet header.
 * @var std::uint32_t sequence_number Sequence number being acknowledged.
 * @var std::uint32_t player_id ID of the player associated with the
 * acknowledged packet.
 */
struct ALIGNED AckPacket {
    PacketHeader header;
    std::uint32_t sequence_number;
    std::uint32_t player_id;
};

/**
 * @brief Client request to obtain a challenge string for joining a room.
 *
 * Contains the target room's identifier and a per-packet sequence number used
 * for ordering/reliability.
 *
 * @param room_id Identifier of the room for which a challenge is requested.
 * @param sequence_number Per-packet sequence number used to order and correlate
 * packets.
 */
struct ALIGNED RequestChallengePacket {
    PacketHeader header;
    std::uint32_t room_id;
    std::uint32_t sequence_number;
};

/**
 * @brief Server response carrying a challenge string and a timestamp for
 * challenge-response flows.
 *
 * Contains the packet header and payload used to validate or authenticate a
 * room/connection:
 * - header: Common packet header with type and total serialized size.
 * - challenge: UTF-8 challenge string to be presented or signed by the
 * requester.
 * - timestamp: 32-bit timestamp associated with the challenge (e.g., epoch
 * seconds).
 * - sequence_number: Packet ordering index used for reliability and
 * deduplication.
 */
struct ALIGNED ChallengeResponsePacket {
    PacketHeader header;
    std::string challenge;
    std::uint32_t timestamp;
    std::uint32_t sequence_number;
};

/**
 * @brief Entry for a single player's score in the scoreboard.
 */
struct ALIGNED ScoreEntry {
    std::string player_name;
    std::uint32_t score;
};

/**
 * @brief Request packet to fetch the top scores from the server.
 *
 * @param header Common packet header.
 * @param limit Maximum number of top scores to retrieve.
 */
struct ALIGNED ScoreboardRequestPacket {
    PacketHeader header;
    std::uint32_t limit;
};

/**
 * @brief Response packet containing the top player scores.
 *
 * @param header Common packet header.
 * @param entry_count Number of score entries in the scores array.
 * @param scores Array of score entries, sorted by score (highest first).
 */
struct ALIGNED ScoreboardResponsePacket {
    PacketHeader header;
    std::uint32_t entry_count;
    std::vector<ScoreEntry> scores;
};
