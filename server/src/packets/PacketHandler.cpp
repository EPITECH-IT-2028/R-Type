#include "PacketHandler.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "Broadcast.hpp"
#include "GameManager.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSerialize.hpp"
#include "Server.hpp"
#include "ServerInputSystem.hpp"

/**
 * @brief Sends a JoinRoomResponse to the specified client indicating the join
 * result.
 *
 * Sends a serialized JoinRoomResponse packet to the client identified by
 * player_id. If serialization fails, logs an error and aborts sending.
 *
 * @param player_id ID of the client that will receive the response.
 * @param error Result code describing why the join succeeded or failed.
 */
void packet::ResponseHelper::sendJoinRoomResponse(server::Server &server,
                                                  std::uint32_t player_id,
                                                  RoomError error) {
  auto response = PacketBuilder::makeJoinRoomResponse(error);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  if (serializedBuffer.empty()) {
    std::cerr << "[ERROR] Failed to serialize JoinRoomResponse for client "
              << player_id << std::endl;
    return;
  }
  server.getNetworkManager().sendToClient(
      player_id, reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
}

/**
 * @brief Send a MatchmakingResponse to a specific player indicating the
 * matchmaking result.
 *
 * @param server Server instance whose network manager will deliver the
 * response.
 * @param player_id ID of the target player to receive the response.
 * @param error Result code describing the matchmaking outcome.
 */
void packet::ResponseHelper::sendMatchmakingResponse(server::Server &server,
                                                     std::uint32_t player_id,
                                                     RoomError error) {
  auto response = PacketBuilder::makeMatchmakingResponse(error);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  if (serializedBuffer.empty()) {
    std::cerr << "[ERROR] Failed to serialize MatchmakingResponse for client "
              << player_id << std::endl;
    return;
  }
  server.getNetworkManager().sendToClient(
      player_id, reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
}

/**
 * @brief Handle an incoming chat message and broadcast it to the sender's room.
 *
 * Deserializes a ChatMessagePacket from the provided buffer, replaces the
 * packet's player_id with the sender's id, and broadcasts the validated packet
 * to all clients in the sender's room.
 *
 * @param server Server instance used to access game and network managers.
 * @param client Client that sent the packet; its player_id and room_id are
 * used.
 * @param data Pointer to the serialized ChatMessagePacket bytes.
 * @param size Number of bytes available at `data`.
 * @return int `OK` on success, `KO` if deserialization fails or the client is
 * not in a room.
 */
int packet::ChatMessageHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<ChatMessagePacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize ChatMessagePacket from client "
              << client._player_id << std::endl;
    return KO;
  }
  const ChatMessagePacket &packet = deserializedPacket.value();
  std::string message = packet.message;
  std::cout << "[MESSAGE] Player " << client._player_id << ": " << message
            << std::endl;

  ChatMessagePacket validatedPacket = packet;
  validatedPacket.player_id = static_cast<std::uint32_t>(client._player_id);

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    std::cerr << "[ERROR] Client " << client._player_id << " is not in any room"
              << std::endl;
    return KO;
  }

  auto roomClients = room->getClients();

  broadcast::Broadcast::broadcastMessageToRoom(server.getNetworkManager(),
                                               roomClients, validatedPacket);
  return OK;
}

/**
 * @brief Process a PlayerInfoPacket from a client, update the client's player
 * name, and initialize the player in a room when appropriate.
 *
 * @param server Server managing rooms, game state, and networking.
 * @param client Client that sent the packet; its `_player_name` and
 * room-related state may be updated.
 * @param data Pointer to the raw PlayerInfoPacket bytes.
 * @param size Size of the data buffer in bytes.
 * @return int `OK` if the packet was handled successfully and any required
 * player initialization completed, `KO` on deserialization failure, invalid
 * client state, or other errors.
 */
int packet::PlayerInfoHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerInfoPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerInfoPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerInfoPacket &packet = deserializedPacket.value();

  std::string name = packet.name;

  client._player_name = name;
  if (!server.getDatabaseManager().addPlayer(name, client._ip_address)) {
    std::cerr << "[ERROR] Failed to add player " << client._player_id
              << " to database" << std::endl;
  }
  if (!server.getDatabaseManager().updatePlayerStatus(client._player_name,
                                                      true)) {
    std::cerr << "[ERROR] Failed to update player status for player "
              << client._player_id << std::endl;
  }

  switch (client._state) {
    case server::ClientState::CONNECTED_MENU:
      std::cout << "[PLAYERINFO] Client " << client._player_id << " (" << name
                << ") registered in menu" << std::endl;
      return OK;
    case server::ClientState::IN_ROOM_WAITING:
    case server::ClientState::IN_GAME:
      return server.initializePlayerInRoom(client) ? OK : KO;

    default:
      std::cerr << "[ERROR] Invalid client state for PlayerInfo" << std::endl;
      return KO;
  }
}

/**
 * @brief Validates a HeartbeatPlayerPacket and updates the client's heartbeat
 * timestamp.
 *
 * Validates that the incoming packet deserializes correctly and that its
 * player_id matches the client's player id; on success updates the client's
 * last_heartbeat to the current time.
 *
 * @param client The client whose heartbeat is being validated and updated.
 * @param data Pointer to the received packet data.
 * @param size Size of the received packet data in bytes.
 * @return int `OK` on successful validation and heartbeat update, `KO`
 * otherwise.
 */
int packet::HeartbeatPlayerHandler::handlePacket(
    [[maybe_unused]] server::Server &server, server::Client &client,
    const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);
  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<HeartbeatPlayerPacket>(
          buffer);
  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize HeartbeatPlayerPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const auto &hb = deserializedPacket.value();
  if (hb.player_id != static_cast<std::uint32_t>(client._player_id)) {
    return KO;
  }
  client._last_heartbeat = std::chrono::steady_clock::now();
  return OK;
}

int packet::PlayerShootHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerShootPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerShootPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerShootPacket &packet = deserializedPacket.value();

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    return KO;
  }

  auto player = room->getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  std::pair<float, float> pos = player->getPosition();
  const float speed = PROJECTILE_SPEED;

  float vx = speed;
  float vy = 0.0f;

  std::uint32_t projectileId = room->getGame().getNextProjectileId();

  auto projectileType = packet.projectile_type;

  if (projectileType != ProjectileType::PLAYER_BASIC) {
    projectileType = ProjectileType::PLAYER_BASIC;
  }

  auto projectile = room->getGame().createProjectile(
      projectileId, client._player_id, projectileType, pos.first, pos.second,
      vx, vy);

  if (!projectile) {
    return KO;
  }

  auto playerShotPacket = PacketBuilder::makePlayerShoot(
      pos.first, pos.second, projectileType, packet.sequence_number);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerShootToRoom(
      server.getNetworkManager(), roomClients, playerShotPacket);

  return OK;
}

/**
 * @brief Handle a PlayerDisconnectPacket from a client and finalize that
 * client's disconnection.
 *
 * Updates server and client state, removes the player from their room and game
 * when present, broadcasts the disconnect and an announcement chat message to
 * remaining room clients, and clears the client's server slot.
 *
 * @returns int `OK` if the disconnection was processed successfully; `KO` if
 * packet deserialization fails or the packet's `player_id` does not match the
 * client.
 */
int packet::PlayerDisconnectedHandler::handlePacket(server::Server &server,
                                                    server::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerDisconnectPacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize PlayerDisconnectPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const auto &disc = deserializedPacket.value();
  if (disc.player_id != static_cast<std::uint32_t>(client._player_id)) {
    return KO;
  }
  std::cout << "[WORLD] Player " << client._player_id << " disconnected."
            << std::endl;

  bool wasConnected = client._connected;
  client._connected = false;

  if (wasConnected) {
    server.setPlayerCount(server.getPlayerCount() - 1);
  }

  if (client._room_id != NO_ROOM) {
    auto room = server.getGameManager().getRoom(client._room_id);
    if (room) {
      auto player = room->getGame().getPlayer(client._player_id);
      if (player) {
        room->getGame().destroyPlayer(client._player_id);
      }

      if (!server.getDatabaseManager().updatePlayerStatus(client._player_name,
                                                          false)) {
        std::cerr << "[ERROR] Failed to update player status for player "
                  << client._player_id << std::endl;
      }

      std::shared_ptr<server::Client> sharedClient;
      for (const auto &entry : server.getClients()) {
        if (entry && entry->_player_id == client._player_id) {
          sharedClient = entry;
          break;
        }
      }
      if (sharedClient) {
        server.getGameManager().leaveRoom(sharedClient);
      }
      auto roomClients = room->getClients();

      auto disconnectPacket =
          PacketBuilder::makePlayerDisconnect(client._player_id);
      broadcast::Broadcast::broadcastPlayerDisconnectToRoom(
          server.getNetworkManager(), roomClients, disconnectPacket);

      std::string msg = client._player_name + " has disconnected.";
      auto chatMessagePacket = PacketBuilder::makeChatMessage(
          msg, SERVER_SENDER_ID, 255, 255, 0, 255);
      broadcast::Broadcast::broadcastMessageToRoom(
          server.getNetworkManager(), roomClients, chatMessagePacket);
    }
  }
  server.clearClientSlot(client._player_id);
  return OK;
}

/**
 * @brief Process a CreateRoomPacket: create a room, join the requesting client,
 * and initialize the player in that room.
 *
 * Deserializes the incoming packet, creates a game room using the provided name
 * and password, has the client join the newly created room, and initializes the
 * player state inside the room. On failure the function performs necessary
 * cleanup (leaving/destroying the room) and restores client state.
 *
 * @param server Server instance handling game rooms and clients.
 * @param client Client that sent the CreateRoomPacket.
 * @param data Pointer to the serialized packet data.
 * @param size Size in bytes of the serialized packet.
 * @return int `OK` on success after room creation, join, and player
 * initialization; `KO` on any failure.
 */
int packet::CreateRoomHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<CreateRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize CreateRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const CreateRoomPacket &packet = deserializedPacket.value();

  std::string roomName = packet.room_name;
  std::string password = packet.password;

  std::string actualPassword = packet.is_private ? password : "";

  auto newRoom = server.getGameManager().createRoom(roomName, actualPassword);

  if (!newRoom) {
    std::cerr << "[ERROR] Failed to create room for client "
              << client._player_id << std::endl;
    return KO;
  }

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    std::cerr << "[ERROR] Failed to get shared_ptr for client "
              << client._player_id << std::endl;
    server.getGameManager().destroyRoom(newRoom->getRoomId());
    return KO;
  }

  bool joinSuccess = server.getGameManager().joinRoom(
      newRoom->getRoomId(), server.getClientById(client._player_id));

  if (!joinSuccess) {
    std::cerr << "[ERROR] Client " << client._player_id
              << " failed to join newly created room " << newRoom->getRoomId()
              << std::endl;
    if (!server.getGameManager().destroyRoom(newRoom->getRoomId())) {
      std::cerr << "[ERROR] Failed to delete room " << newRoom->getRoomId()
                << std::endl;
    } else
      std::cout << "[INFO] Room " << newRoom->getRoomId() << std::endl;
    return KO;
  }

  client._state = server::ClientState::IN_ROOM_WAITING;

  if (!server.initializePlayerInRoom(client)) {
    std::cerr << "[ERROR] Failed to initialize player " << client._player_id
              << " in room " << newRoom->getRoomId() << std::endl;
    server.getGameManager().leaveRoom(sharedClient);
    server.getGameManager().destroyRoom(newRoom->getRoomId());
    client._state = server::ClientState::CONNECTED_MENU;
    return KO;
  }

  auto response = PacketBuilder::makeCreateRoomResponse(RoomError::SUCCESS,
                                                        newRoom->getRoomId());
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  std::cout << "[CREATE ROOM] Client " << client._player_id
            << " created and joined room " << newRoom->getRoomId() << " ("
            << roomName << ")" << std::endl;

  return OK;
}

/**
 * @brief Process a client's JoinRoom request and respond with the appropriate
 * JoinRoomResponse.
 *
 * Deserializes a JoinRoomPacket from the provided buffer, validates room
 * existence and password, attempts to join the room, initializes the player in
 * the room on success, and sends a JoinRoomResponse indicating the resulting
 * RoomError.
 *
 * @param data Pointer to the serialized JoinRoomPacket.
 * @param size Size of the serialized data in bytes.
 * @return int `OK` on success, `KO` on failure.
 */
int packet::JoinRoomHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<JoinRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize JoinRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const JoinRoomPacket &packet = deserializedPacket.value();

  auto room = server.getGameManager().getRoom(packet.room_id);
  if (!room) {
    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::ROOM_NOT_FOUND);
    return KO;
  }

  if (room->hasPassword()) {
    std::string providedHash(packet.password);
    std::string storedPassword = room->getPassword();

    bool valid = server.getChallengeManager().validateJoinRoom(
        client._player_id, providedHash, storedPassword);

    if (!valid) {
      std::cerr << "[WARN] Invalid password for room " << packet.room_id
                << " from player " << client._player_id << std::endl;
      ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                           RoomError::WRONG_PASSWORD);
      return KO;
    }
  }

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::UNKNOWN_ERROR);
    return KO;
  }

  bool joinSuccess =
      server.getGameManager().joinRoom(packet.room_id, sharedClient);
  if (!joinSuccess) {
    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::ROOM_FULL);
    return KO;
  }

  client._state = server::ClientState::IN_ROOM_WAITING;

  if (!server.initializePlayerInRoom(client)) {
    std::cerr << "[ERROR] Failed to initialize player in room" << std::endl;
    server.getGameManager().leaveRoom(sharedClient);
    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::UNKNOWN_ERROR);
    client._state = server::ClientState::CONNECTED_MENU;
    return KO;
  }

  ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                       RoomError::SUCCESS);

  std::cout << "[SUCCESS] Player " << client._player_id << " joined room "
            << packet.room_id << std::endl;

  return OK;
}

/**
 * @brief Handle a client's request to leave their current room.
 *
 * If the client is in a room, removes the client from that room and sets the
 * client's state to CONNECTED_MENU. On success the client will no longer be
 * associated with the previous room.
 *
 * @param data Pointer to the raw packet bytes containing the LeaveRoomPacket.
 * @param size Number of bytes available at data.
 * @return int OK on success (client left room or was not in a room), KO on
 * failure (failed to deserialize the packet or failed to obtain the shared
 * client).
 */
int packet::LeaveRoomHandler::handlePacket(server::Server &server,
                                           server::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<LeaveRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize LeaveRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  if (client._room_id == NO_ROOM) {
    std::cout << "[LEAVE ROOM] Client " << client._player_id
              << " is not in any room" << std::endl;
    return OK;
  }

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    std::cerr << "[ERROR] Failed to get shared_ptr for client "
              << client._player_id << std::endl;
    return KO;
  }

  std::cout << "[LEAVE ROOM] Client " << client._player_id << " leaving room "
            << client._room_id << std::endl;

  server.getGameManager().leaveRoom(sharedClient);

  client._state = server::ClientState::CONNECTED_MENU;

  return OK;
}

/**
 * @brief Send the current list of active rooms to the requesting client.
 *
 * @returns int `OK` on success, `KO` if the request cannot be processed (for
 * example, deserialization or serialization failure).
 */
int packet::ListRoomHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<ListRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize ListRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  auto rooms = server.getGameManager().getAllRooms();
  std::vector<RoomInfo> roomInfos;
  for (const auto &room : rooms) {
    RoomInfo info;
    info.room_id = room->getRoomId();
    info.room_name = room->getRoomName();
    info.player_count = room->getPlayerCount();
    info.max_players = room->getMaxPlayers();
    roomInfos.push_back(info);
  }

  auto listRoomResponsePacket = PacketBuilder::makeListRoomResponse(roomInfos);

  auto serializedBuffer =
      serialization::BitserySerializer::serialize(listRoomResponsePacket);
  if (serializedBuffer.empty()) {
    std::cerr
        << "[ERROR] Failed to serialize ListRoomResponsePacket for client "
        << client._player_id << std::endl;
    return KO;
  }
  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
  return OK;
}

/**
 * @brief Attempts to place the client into a matchmaking room and responds with
 * the result.
 *
 * Deserializes a MatchmakingRequestPacket from the provided buffer, then either
 * joins the client to an existing suitable room or creates and joins a new
 * matchmaking room. On success the client is transitioned to IN_ROOM_WAITING,
 * the server initializes the player in the room, and a success matchmaking
 * response is sent to the client. On failure an appropriate matchmaking
 * response (e.g., UNKNOWN_ERROR) is sent and the client's state is restored
 * where applicable.
 *
 * @param server Reference to the server managing rooms, clients, and game
 * state.
 * @param client Reference to the client issuing the matchmaking request; may be
 * updated.
 * @param data Pointer to the serialized MatchmakingRequestPacket data.
 * @param size Size in bytes of the serialized data buffer.
 * @return int `OK` if matchmaking succeeded and the client was initialized in a
 * room, `KO` otherwise.
 */
int packet::MatchmakingRequestHandler::handlePacket(server::Server &server,
                                                    server::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<MatchmakingRequestPacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize MatchmakingRequestPacket "
                 "from client "
              << client._player_id << std::endl;
    return KO;
  }

  const MatchmakingRequestPacket &packet = deserializedPacket.value();

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                            RoomError::UNKNOWN_ERROR);
    return KO;
  }

  bool joinSuccess = server.getGameManager().joinAnyRoom(sharedClient);

  if (!joinSuccess) {
    auto newRoom = server.getGameManager().createRoom("Matchmaking Room");
    if (!newRoom) {
      std::cerr << "[ERROR] Client " << client._player_id
                << " failed to create new room for matchmaking" << std::endl;
      ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                              RoomError::UNKNOWN_ERROR);
      return KO;
    }
    auto joinSuccess =
        server.getGameManager().joinRoom(newRoom->getRoomId(), sharedClient);
    if (newRoom && joinSuccess) {
      std::cout << "[MATCHMAKING] Client " << client._player_id
                << " created and joined new room " << newRoom->getRoomId()
                << std::endl;

      client._state = server::ClientState::IN_ROOM_WAITING;

      if (!server.initializePlayerInRoom(client)) {
        server.getGameManager().leaveRoom(sharedClient);
        server.getGameManager().destroyRoom(newRoom->getRoomId());
        client._state = server::ClientState::CONNECTED_MENU;
        return KO;
      }

      ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                              RoomError::SUCCESS);
    } else {
      std::cerr << "[ERROR] Client " << client._player_id
                << " failed to create/join new room for matchmaking"
                << std::endl;
      ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                              RoomError::UNKNOWN_ERROR);
      return KO;
    }
  } else {
    std::cout << "[MATCHMAKING] Client " << client._player_id
              << " joined existing room" << std::endl;

    client._state = server::ClientState::IN_ROOM_WAITING;

    if (!server.initializePlayerInRoom(client)) {
      std::cerr << "[MATCHMAKING] Failed to initialize player" << std::endl;
      server.getGameManager().leaveRoom(sharedClient);
      client._state = server::ClientState::CONNECTED_MENU;
      return KO;
    }

    ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                            RoomError::SUCCESS);
  }

  return OK;
}

/**
 * @brief Apply a player's movement input to their in-room player state and
 * broadcast the resulting PlayerMove to the room.
 *
 * Deserializes the incoming PlayerInput packet from the provided buffer,
 * updates the player's sequence number and position according to the input and
 * game delta time (clamped to window bounds), and broadcasts a PlayerMove
 * packet to all clients in the same room.
 *
 * @param server Server instance used to access game and network managers.
 * @param client Client that sent the input; identifies the player and room.
 * @param data Pointer to the raw packet data buffer containing a
 * PlayerInputPacket.
 * @param size Size of the raw packet data buffer in bytes.
 * @return int `OK` on success; `KO` on error (e.g., deserialization failure,
 * missing or inactive room, or missing player).
 */
int packet::PlayerInputHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerInputPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerInputPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerInputPacket &packet = deserializedPacket.value();
  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    return KO;
  }
  auto sis = room->getGame().getServerInputSystem();
  if (!sis) {
    return KO;
  }
  if (client._entity_id == static_cast<Entity>(-1)) {
    std::cerr << "[ERROR] Client " << client._player_id
              << " has invalid entity_id" << std::endl;
    return KO;
  }

  const ecs::PlayerInput packetInput = {
      static_cast<MovementInputType>(packet.input),
      static_cast<int>(packet.sequence_number)};

  sis->queueInput(client._entity_id, packetInput);
  return OK;
}

int packet::RequestChallengeHandler::handlePacket(server::Server &server,
                                                  server::Client &client,
                                                  const char *data,
                                                  std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<RequestChallengePacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize RequestChallengePacket from "
                 "client "
              << client._player_id << std::endl;
    return KO;
  }

  const RequestChallengePacket &packet = deserializedPacket.value();

  auto room = server.getGameManager().getRoom(packet.room_id);
  if (!room) {
    std::cerr << "[ERROR] Room " << packet.room_id << " not found for "
              << "client " << client._player_id << std::endl;
    return KO;
  }

  std::string challenge =
      server.getChallengeManager().createChallenge(client._player_id);

  auto time = static_cast<std::uint32_t>(
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());

  auto responsePacket = PacketBuilder::makeChallengeResponse(challenge, time);

  auto serializedBuffer =
      serialization::BitserySerializer::serialize(responsePacket);
  if (serializedBuffer.empty()) {
    std::cerr << "[ERROR] Failed to serialize ChallengeResponsePacket for "
                 "client "
              << client._player_id << std::endl;
    return KO;
  }

  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  return OK;
}
