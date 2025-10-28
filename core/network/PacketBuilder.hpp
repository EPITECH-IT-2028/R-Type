#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include "Macro.hpp"
#include "Packet.hpp"
#include "Serializer.hpp"
#include "PacketSerialize.hpp"

struct PacketBuilder {
    /**
     * @brief Constructs a ChatMessagePacket populated with the given text,
     * player ID, RGBA color, and the current timestamp.
     *
     * The provided message is copied into the packet's message buffer; it will
     * be truncated to fit and always null-terminated.
     *
     * @param msg Chat text to include in the packet.
     * @param player_id ID of the player sending the message.
     * @param r Red color component (0-255) for the message.
     * @param g Green color component (0-255) for the message.
     * @param b Blue color component (0-255) for the message.
     * @param a Alpha (opacity) component (0-255) for the message.
     * @return ChatMessagePacket Packet with header.type set to ChatMessage,
     * header.size set to the packet size, timestamp set to the current time,
     * message copied (truncated if necessary and null-terminated), player_id
     * assigned, and color components assigned from the provided RGBA values.
     */
    static ChatMessagePacket makeChatMessage(const std::string &msg,
                                             std::uint32_t player_id,
                                             std::uint8_t r, std::uint8_t g,
                                             std::uint8_t b, std::uint8_t a) {
      ChatMessagePacket packet{};
      packet.header.type = PacketType::ChatMessage;
      packet.header.size = 0;
      packet.timestamp = static_cast<std::uint32_t>(time(nullptr));
      packet.message = msg;
      packet.player_id = player_id;
      packet.r = r;
      packet.g = g;
      packet.b = b;
      packet.a = a;

      ChatMessagePacket temp_packet = packet;
      serialization::Buffer serializedBuffer = serialization::BitserySerializer::serialize(temp_packet);
      std::uint32_t fullSerializedSize = serializedBuffer.size();
      packet.header.size = fullSerializedSize - sizeof(PacketHeader);
      return packet;
    }

    /**
     * @brief Constructs a ChatMessagePacket with default white color.
     *
     * This overload of makeChatMessage sets the message color to white
     * (RGBA: 255, 255, 255, 255) by default.
     *
     * @param msg Text to include in the message packet; may be truncated to fit
     * the packet.
     * @param player_id Identifier of the player who sent the message.
     * @return ChatMessagePacket Packet with header.type ==
     * PacketType::ChatMessage, header.size set to sizeof(packet), a
     * null-terminated `message` field, current `timestamp`, and `player_id`
     * set. Message color is white.
     */
    static ChatMessagePacket makeChatMessage(const std::string &msg,
                                             std::uint32_t player_id) {
      return makeChatMessage(msg, player_id, 255, 255, 255, 255);
    }

    /**
     * @brief Creates a NewPlayerPacket for a newly joined player.
     *
     * The returned packet contains the player's identity, position, movement
     * speed, and maximum health, and has its packet header initialized for a
     * NewPlayer.
     *
     * @param player_id Unique identifier for the player.
     * @param player_name Name of the player.
     * @param x Initial X position of the player.
     * @param y Initial Y position of the player.
     * @param speed Initial movement speed of the player.
     * @param max_health Maximum health for the player (default: 100).
     * @return NewPlayerPacket Populated packet ready to be serialized and sent.
     */
    static NewPlayerPacket makeNewPlayer(std::uint32_t player_id,
                                         const std::string &player_name,
                                         float x, float y, float speed,
                                         std::uint32_t max_health = 100) {
      NewPlayerPacket packet{};
      packet.header.type = PacketType::NewPlayer;
      packet.header.size = 0;
      packet.player_id = player_id;
      packet.player_name = player_name;
      packet.x = x;
      packet.y = y;
      packet.speed = speed;
      packet.max_health = max_health;

      NewPlayerPacket temp_packet = packet;
      serialization::Buffer serializedBuffer = serialization::BitserySerializer::serialize(temp_packet);
      std::uint32_t fullSerializedSize = serializedBuffer.size();
      packet.header.size = fullSerializedSize - sizeof(PacketHeader);
      return packet;
    }

    /**
     * @brief Create a movement update for a player.
     *
     * @param player_id ID of the player who moved.
     * @param seq Sequence number used to order this movement update.
     * @param x New X coordinate of the player.
     * @param y New Y coordinate of the player.
     * @return PlayerMovePacket Packet populated with header.type set to
     * PlayerMove, header.size set to the packet size, and fields: player_id,
     * sequence_number, x, and y.
     */
    static PlayerMovePacket makePlayerMove(std::uint32_t player_id,
                                           std::uint32_t seq, float x,
                                           float y) {
      PlayerMovePacket packet{};
      packet.header.type = PacketType::PlayerMove;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.sequence_number = seq;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    /**
     * @brief Create a PlayerInfo packet containing the player's name.
     *
     * @param name Player's name; copied into the packet's name field and
     * truncated if it exceeds the field size. The stored name is guaranteed to
     * be null-terminated.
     * @return PlayerInfoPacket Packet with header.type set to
     * PacketType::PlayerInfo, header.size populated, and the name field filled
     * from `name`.
     */
    static PlayerInfoPacket makePlayerInfo(const std::string &name) {
      PlayerInfoPacket packet{};
      packet.header.type = PacketType::PlayerInfo;
      packet.header.size = 0;
      packet.name = name;

      PlayerInfoPacket temp_packet = packet;
      serialization::Buffer serializedBuffer = serialization::BitserySerializer::serialize(temp_packet);
      std::uint32_t fullSerializedSize = serializedBuffer.size();
      packet.header.size = fullSerializedSize - sizeof(PacketHeader);
      return packet;
    }

    /**
     * @brief Create a PlayerHitPacket describing a hit applied to a player.
     *
     * @param player_id ID of the player who was hit.
     * @param damage Amount of damage applied to the player.
     * @param x X coordinate of the hit location.
     * @param y Y coordinate of the hit location.
     * @param sequence_number Sequence number associated with this event.
     * @return PlayerHitPacket Packet populated with the provided player ID,
     * damage, hit coordinates, and sequence number.
     */
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

    /**
     * @brief Constructs an EnemySpawnPacket with the enemy's identity, type,
     * position, velocity, and health.
     *
     * @param enemy_id Unique identifier for the enemy.
     * @param type Enemy type.
     * @param x Spawn X coordinate.
     * @param y Spawn Y coordinate.
     * @param vx Initial velocity in the X direction.
     * @param vy Initial velocity in the Y direction.
     * @param health Current health.
     * @param max_health Maximum health.
     * @return EnemySpawnPacket Packet with fields initialized and its header
     * indicating an EnemySpawn.
     */
    static EnemySpawnPacket makeEnemySpawn(std::uint32_t enemy_id,
                                           EnemyType type, float x, float y,
                                           float vx, float vy,
                                           std::uint32_t health,
                                           std::uint32_t max_health) {
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

    /**
     * @brief Constructs an EnemyMovePacket representing an enemy's position and
     * velocity update.
     *
     * @param enemy_id Unique identifier of the enemy.
     * @param x X coordinate of the enemy's position.
     * @param y Y coordinate of the enemy's position.
     * @param velocity_x Velocity of the enemy along the X axis.
     * @param velocity_y Velocity of the enemy along the Y axis.
     * @param seq Sequence number used to order movement updates.
     * @return EnemyMovePacket Packet with header.type set to
     * `PacketType::EnemyMove`, header.size set to the packet size, and fields
     * populated with the provided values.
     */
    static EnemyMovePacket makeEnemyMove(std::uint32_t enemy_id, float x,
                                         float y, float velocity_x,
                                         float velocity_y, int seq) {
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

    /**
     * @brief Create an EnemyDeath packet describing an enemy's death and
     * awarding score to a player.
     *
     * @param enemy_id Unique identifier of the enemy.
     * @param death_x X coordinate of the enemy's death location.
     * @param death_y Y coordinate of the enemy's death location.
     * @param player_id ID of the player credited with the kill.
     * @param score Score awarded to the player for the kill.
     * @return EnemyDeathPacket Populated packet containing enemy_id, death
     * coordinates, player_id, and score; packet header is set to
     * `PacketType::EnemyDeath` and `header.size` is set to the packet's size.
     */
    static EnemyDeathPacket makeEnemyDeath(std::uint32_t enemy_id,
                                           float death_x, float death_y,
                                           std::uint32_t player_id,
                                           std::uint32_t score) {
      EnemyDeathPacket packet{};
      packet.header.type = PacketType::EnemyDeath;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.death_x = death_x;
      packet.death_y = death_y;
      packet.player_id = player_id;
      packet.score = score;
      return packet;
    }

    /**
     * @brief Constructs an EnemyHit packet describing damage dealt to an enemy.
     *
     * @param enemy_id Identifier of the enemy that was hit.
     * @param hit_x World X coordinate where the hit occurred.
     * @param hit_y World Y coordinate where the hit occurred.
     * @param damage Amount of damage applied to the enemy.
     * @param sequence_number Sequence number for ordering the event.
     * @return EnemyHitPacket Packet populated with header, enemy id, hit
     * position, damage, and sequence number.
     */
    static EnemyHitPacket makeEnemyHit(std::uint32_t enemy_id, float hit_x,
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

    /**
     * @brief Constructs a PlayerShootPacket with shot position, projectile
     * type, and sequence number.
     *
     * @param x X coordinate of the shot.
     * @param y Y coordinate of the shot.
     * @param projectile_type Projectile type fired.
     * @param seq Sequence number used to order the shot.
     * @return PlayerShootPacket Packet populated with position, projectile
     * type, and sequence number.
     */
    static PlayerShootPacket makePlayerShoot(float x, float y,
                                             ProjectileType projectile_type,
                                             std::uint32_t seq) {
      PlayerShootPacket packet{};
      packet.header.type = PacketType::PlayerShoot;
      packet.header.size = sizeof(packet);
      packet.x = x;
      packet.y = y;
      packet.projectile_type = projectile_type;
      packet.sequence_number = seq;
      return packet;
    }

    /**
     * @brief Constructs a ProjectileSpawnPacket with the provided projectile
     * properties.
     *
     * @param projectile_id Unique identifier for the projectile.
     * @param type ProjectileType enum value specifying the projectile kind.
     * @param x Initial x coordinate of the projectile.
     * @param y Initial y coordinate of the projectile.
     * @param vel_x Initial velocity along the x axis.
     * @param vel_y Initial velocity along the y axis.
     * @param is_enemy True if the projectile was fired by an enemy, false if
     * fired by a player.
     * @param damage Damage value carried by the projectile.
     * @param owner_id Identifier of the entity that owns or fired the
     * projectile.
     * @return ProjectileSpawnPacket Packet populated with header type/size and
     * the provided fields.
     */
    static ProjectileSpawnPacket makeProjectileSpawn(
        std::uint32_t projectile_id, ProjectileType type, float x, float y,
        float vel_x, float vel_y, bool is_enemy, std::uint32_t damage,
        std::uint32_t owner_id) {
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

    /**
     * @brief Constructs a packet representing a projectile hitting a target.
     *
     * @param projectile_id Identifier of the projectile that hit the target.
     * @param target_id Identifier of the target that was hit.
     * @param hit_x X coordinate where the hit occurred.
     * @param hit_y Y coordinate where the hit occurred.
     * @param target_is_player `1` if the target is a player, `0` otherwise.
     * @return ProjectileHitPacket Packet with header.type set to
     * `ProjectileHit`, header.size set to the packet size, and all hit fields
     * populated.
     */
    static ProjectileHitPacket makeProjectileHit(
        std::uint32_t projectile_id, std::uint32_t target_id, float hit_x,
        float hit_y, std::uint8_t target_is_player) {
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

    /**
     * @brief Constructs a packet signalling that a projectile was destroyed.
     *
     * @param projectile_id ID of the destroyed projectile.
     * @param x X-coordinate of the destruction location.
     * @param y Y-coordinate of the destruction location.
     * @return ProjectileDestroyPacket Packet whose header.type is
     * ProjectileDestroy, whose header.size is set to the packet size, and whose
     * projectile_id, x, and y fields are populated.
     */
    static ProjectileDestroyPacket makeProjectileDestroy(
        std::uint32_t projectile_id, float x, float y) {
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

    /**
     * @brief Constructs a GameEnd packet indicating whether the game has ended.
     *
     * @param ended `true` if the game has ended, `false` if the game is
     * ongoing.
     * @return GameEndPacket Packet with header fields populated and `game_end`
     * set to `ended`.
     */
    static GameEndPacket makeGameEnd(bool ended) {
      GameEndPacket packet{};
      packet.header.type = PacketType::GameEnd;
      packet.header.size = sizeof(packet);
      packet.game_end = ended;
      return packet;
    }

    /**
     * @brief Constructs a PlayerDeath packet describing a player's death
     * location.
     *
     * Builds a PlayerDeathPacket with the header type and size set, and fills
     * the player identifier and world coordinates where the player died.
     *
     * @param player_id Identifier of the player who died.
     * @param x World X coordinate of the death location.
     * @param y World Y coordinate of the death location.
     * @return PlayerDeathPacket Populated packet ready for transmission.
     */
    static PlayerDeathPacket makePlayerDeath(std::uint32_t player_id, float x,
                                             float y) {
      PlayerDeathPacket packet{};
      packet.header.type = PacketType::PlayerDeath;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    /**
     * @brief Construct a PlayerDisconnect packet for the given player.
     *
     * @param player_id ID of the player that disconnected.
     * @return PlayerDisconnectPacket Packet with header type set to
     * PacketType::PlayerDisconnected, header size set to the packet's sizeof,
     * and player_id populated.
     */
    static PlayerDisconnectPacket makePlayerDisconnect(
        std::uint32_t player_id) {
      PlayerDisconnectPacket packet{};
      packet.header.type = PacketType::PlayerDisconnected;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    /**
     * @brief Create a HeartbeatPlayerPacket for the specified player.
     *
     * @param player_id Player identifier to embed in the packet.
     * @return HeartbeatPlayerPacket with header.type set to
     * PacketType::Heartbeat, header.size set to the packet size, and player_id
     * set to the provided value.
     */
    static HeartbeatPlayerPacket makeHeartbeatPlayer(std::uint32_t player_id) {
      HeartbeatPlayerPacket packet{};
      packet.header.type = PacketType::Heartbeat;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    /**
     * @brief Constructs a CreateRoomPacket populated with the given room
     * parameters.
     *
     * @param room_name Name of the room; will be copied into the packet and
     * truncated if longer than the packet field.
     * @param max_players Maximum number of players allowed in the room.
     * @param password Optional room password. If non-empty the packet is marked
     * private and the password is stored; if empty the password field is
     * cleared.
     * @return CreateRoomPacket Packet ready for sending to request room
     * creation; its header size is set to sizeof(packet) and its type is set to
     * CreateRoom.
     */
    static CreateRoomPacket makeCreateRoom(const std::string &room_name,
                                           std::uint32_t max_players,
                                           const std::string &password = "") {
      CreateRoomPacket packet{};
      packet.header.type = PacketType::CreateRoom;
      packet.header.size = 0;
      packet.room_name = room_name;
      packet.max_players = max_players;
      packet.is_private = !password.empty();
      if (packet.is_private) {
        packet.password = password;
      } else {
        packet.password.clear();
      }

      CreateRoomPacket temp_packet = packet;
      serialization::Buffer serializedBuffer = serialization::BitserySerializer::serialize(temp_packet);
      std::uint32_t fullSerializedSize = serializedBuffer.size();
      packet.header.size = fullSerializedSize - sizeof(PacketHeader);
      return packet;
    }

    /**
     * Constructs a JoinRoomPacket populated with the specified room ID and
     * password.
     *
     * @param room_id Identifier of the room to join.
     * @param password Password for the room; provide an empty string for no
     * password.
     * @return JoinRoomPacket Packet with header.type set to
     * PacketType::JoinRoom, header.size set to the packet size, and the room_id
     * and password fields populated.
     */
    static JoinRoomPacket makeJoinRoom(std::uint32_t room_id,
                                       const std::string &password) {
      JoinRoomPacket packet{};
      packet.header.type = PacketType::JoinRoom;
      packet.header.size = 0;
      packet.room_id = room_id;
      packet.password = password;

      JoinRoomPacket temp_packet = packet;
      serialization::Buffer serializedBuffer = serialization::BitserySerializer::serialize(temp_packet);
      std::uint32_t fullSerializedSize = serializedBuffer.size();
      packet.header.size = fullSerializedSize - sizeof(PacketHeader);
      return packet;
    }

    /**
     * @brief Constructs a JoinRoomResponsePacket with the specified room error
     * code.
     *
     * @param error_code The RoomError value to set in the packet's error_code
     * field.
     * @return JoinRoomResponsePacket Packet with header.type set to
     * JoinRoomResponse, header.size set to the packet size, and error_code set
     * to the provided value.
     */
    static JoinRoomResponsePacket makeJoinRoomResponse(
        const RoomError &error_code) {
      JoinRoomResponsePacket packet{};
      packet.header.type = PacketType::JoinRoomResponse;
      packet.header.size = sizeof(packet);
      packet.error_code = error_code;
      return packet;
    }

    /**
     * @brief Create a LeaveRoomPacket for the given room identifier.
     *
     * @param room_id Identifier of the room to leave.
     * @return LeaveRoomPacket with its header type set to `LeaveRoom`, header
     * size set to the packet size, and `room_id` set to the provided value.
     */
    static LeaveRoomPacket makeLeaveRoom(std::uint32_t room_id) {
      LeaveRoomPacket packet{};
      packet.header.type = PacketType::LeaveRoom;
      packet.header.size = sizeof(packet);
      packet.room_id = room_id;
      return packet;
    }

    /**
     * @brief Creates a ListRoomPacket with its header initialized.
     *
     * @return ListRoomPacket whose header.type is set to PacketType::ListRoom
     * and header.size is set to the packet's byte size.
     */
    static ListRoomPacket makeListRoom() {
      ListRoomPacket packet{};
      packet.header.type = PacketType::ListRoom;
      packet.header.size = sizeof(packet);
      return packet;
    }

    /**
     * @brief Construct a ListRoomResponsePacket populated from a vector of
     * rooms.
     *
     * @param rooms Vector of RoomInfo entries to include in the response; up to
     * MAX_ROOMS entries are copied.
     * @return ListRoomResponsePacket Packet whose header type is set to
     * ListRoomResponse, header size is set, and whose room_count is the minimum
     * of rooms.size() and MAX_ROOMS with the first room_count entries copied
     * into packet.rooms.
     */
    static ListRoomResponsePacket makeListRoomResponse(
        const std::vector<RoomInfo> &rooms) {
      ListRoomResponsePacket packet{};
      packet.header.type = PacketType::ListRoomResponse;
      packet.header.size = 0;
      packet.room_count = static_cast<std::uint32_t>(
          std::min<std::size_t>(rooms.size(), MAX_ROOMS));

      for (std::size_t i = 0; i < packet.room_count; ++i) {
        packet.rooms[i] = rooms[i];
      }

      ListRoomResponsePacket temp_packet = packet;
      serialization::Buffer serializedBuffer = serialization::BitserySerializer::serialize(temp_packet);
      std::uint32_t fullSerializedSize = serializedBuffer.size();
      packet.header.size = fullSerializedSize - sizeof(PacketHeader);
      return packet;
    }

    /**
     * @brief Constructs a MatchmakingRequestPacket initialized with its header.
     *
     * The packet's header type is set to PacketType::MatchmakingRequest and its
     * size is set to the packet's sizeof value.
     *
     * @return MatchmakingRequestPacket packet whose `header.type` is
     * `PacketType::MatchmakingRequest` and `header.size` equals
     * `sizeof(MatchmakingRequestPacket)`.
     */
    static MatchmakingRequestPacket makeMatchmakingRequest() {
      MatchmakingRequestPacket packet{};
      packet.header.type = PacketType::MatchmakingRequest;
      packet.header.size = sizeof(packet);
      return packet;
    }

    /**
     * @brief Create a packet indicating the result of a matchmaking request.
     *
     * @param error_code The RoomError value representing the outcome of the
     * matchmaking request.
     * @return MatchmakingResponsePacket The constructed packet whose header
     * identifies it as a MatchmakingResponse and whose `error_code` field
     * contains the provided `error_code`.
     */
    static MatchmakingResponsePacket makeMatchmakingResponse(
        const RoomError &error_code) {
      MatchmakingResponsePacket packet{};
      packet.header.type = PacketType::MatchmakingResponse;
      packet.header.size = sizeof(packet);
      packet.error_code = error_code;
      return packet;
    };

    /**
     * @brief Construct a PlayerInputPacket containing a player's current input
     * state.
     *
     * The packet's header.type and header.size are initialized; the packet
     * carries the provided input flags and sequence number for ordering.
     *
     * @param input Bitmask of player input flags (buttons/actions).
     * @param sequence_number Monotonically increasing sequence number for this
     * input.
     * @return PlayerInputPacket The populated packet with input flags and
     * sequence number.
     */
    static PlayerInputPacket makePlayerInput(std::uint8_t input,
                                             std::uint32_t sequence_number) {
      PlayerInputPacket packet{};
      packet.header.type = PacketType::PlayerInput;
      packet.header.size = sizeof(packet);
      packet.input = input;
      packet.sequence_number = sequence_number;
      return packet;
    }
};
