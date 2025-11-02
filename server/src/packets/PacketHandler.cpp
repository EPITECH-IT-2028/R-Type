#include "PacketHandler.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include "Broadcast.hpp"
#include "Client.hpp"
#include "GameManager.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "PacketSerialize.hpp"
#include "Server.hpp"
#include "ServerInputSystem.hpp"

/**
 * @brief Send a JoinRoomResponse to the given client including the room's
 * sequence number and track it as unacknowledged.
 *
 * Builds a JoinRoomResponse using the room/game sequence number, transmits the
 * serialized packet to the client, and records the sent buffer on the client as
 * an unacknowledged packet. If serialization fails, an error is logged and the
 * packet is not sent or recorded.
 *
 * @param error Result code describing why the join succeeded or failed.
 */
void packet::ResponseHelper::sendJoinRoomResponse(server::Server &server,
                                                  server::Client &client,
                                                  std::uint32_t sequence_number,
                                                  RoomError error) {
  auto room = server.getGameManager().getRoom(client._room_id);

  auto response = PacketBuilder::makeJoinRoomResponse(
      error,
      room != nullptr ? room->getGame().fetchAndIncrementSequenceNumber() : 0);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  if (serializedBuffer.empty()) {
    std::cerr << "[ERROR] Failed to serialize JoinRoomResponse for client "
              << client._player_id << std::endl;
    return;
  }

  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  client.addUnacknowledgedPacket(
      response.sequence_number,
      std::make_shared<std::vector<uint8_t>>(serializedBuffer));
}

/**
 * @brief Send a MatchmakingResponse to the specified client indicating the
 * matchmaking result.
 *
 * Builds a response populated with the room's next sequence number, serializes
 * it, sends the bytes to the client, and records the serialized buffer as an
 * unacknowledged packet associated with the response's sequence number.
 *
 * @param server Server used to access the network and game managers.
 * @param client Target client that will receive the response; its room and
 * player id are used.
 * @param error Result code describing the matchmaking outcome.
 */
void packet::ResponseHelper::sendMatchmakingResponse(
    server::Server &server, server::Client &client,
    std::uint32_t sequence_number, RoomError error) {
  auto room = server.getGameManager().getRoom(client._room_id);

  auto response = PacketBuilder::makeMatchmakingResponse(
      error,
      room != nullptr ? room->getGame().fetchAndIncrementSequenceNumber() : 0);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  if (serializedBuffer.empty()) {
    std::cerr << "[ERROR] Failed to serialize MatchmakingResponse for client "
              << client._player_id << std::endl;
    return;
  }
  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  client.addUnacknowledgedPacket(
      response.sequence_number,
      std::make_shared<std::vector<uint8_t>>(serializedBuffer));
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
  auto ackPacket = PacketBuilder::makeAckPacket(validatedPacket.sequence_number,
                                                client._player_id);
  auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(ackPacket));
  server.getNetworkManager().sendToClient(
      client._player_id, reinterpret_cast<const char *>(ackBuffer->data()),
      ackBuffer->size());
  return OK;
}

/**
 * @brief Handle a PlayerInfoPacket: register the player's name, update database
 * status, and acknowledge the sender.
 *
 * Updates the client's stored player name, attempts to add the player to the
 * database and mark them connected, and sends an AckPacket for the received
 * sequence number.
 *
 * @param server Server managing rooms, game state, and networking.
 * @param client Client that sent the packet; its `_player_name` may be updated.
 * @param data Pointer to the raw PlayerInfoPacket bytes.
 * @param size Size of the data buffer in bytes.
 * @return int `OK` if the packet was processed and an acknowledgement was sent,
 * `KO` if deserialization failed or the packet was invalid.
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

  auto playerData = server.getDatabaseManager().getPlayerByUsername(name);
  if (!playerData.has_value()) {
    if (!server.getDatabaseManager().addPlayer(name, client._ip_address)) {
      std::cerr << "[ERROR] Failed to add player " << client._player_id
                << " to database" << std::endl;
    }
    playerData = server.getDatabaseManager().getPlayerByUsername(name);
  }

  if (playerData.has_value()) {
    client._database_player_id = playerData.value().id;
  } else {
    client._database_player_id = INVALID_ID;
  }

  if (!server.getDatabaseManager().updatePlayerStatus(client._player_name,
                                                      true)) {
    std::cerr << "[ERROR] Failed to update player status for player "
              << client._player_id << std::endl;
  }

  std::cout << "[INFO] Client " << client._player_id << " (" << name
            << ") registered in menu" << std::endl;
  auto ackPacket =
      PacketBuilder::makeAckPacket(packet.sequence_number, client._player_id);
  auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(ackPacket));

  server.getNetworkManager().sendToClient(client._player_id, ackBuffer);
  return OK;
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

/**
 * @brief Handle a PlayerShootPacket from a client: spawn a projectile, send an
 * ack to the sender, and broadcast the shot to all clients in the room.
 *
 * @param data Pointer to the serialized PlayerShootPacket buffer.
 * @param size Size of the serialized buffer in bytes.
 * @return int `OK` if the packet was processed and broadcast; `KO` if
 * deserialization fails, the room or player is not found, the packet sequence
 * is not newer, or projectile creation fails.
 */
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

  std::uint64_t lastSeq = 0;
  auto lastProcessedOpt = server.getLastProcessedSeq(client._player_id);

  if (lastProcessedOpt.has_value()) {
    lastSeq = lastProcessedOpt.value();
  }
  if (packet.sequence_number <= lastSeq) {
    server.getNetworkManager().sendToClient(
        client._player_id,
        std::make_shared<std::vector<uint8_t>>(
            serialization::BitserySerializer::serialize(
                PacketBuilder::makeAckPacket(packet.sequence_number,
                                             client._player_id))));
    return OK;
  }
  auto projectile = room->getGame().createProjectile(
      projectileId, client._player_id, projectileType, pos.first, pos.second,
      vx, vy);

  if (!projectile) {
    return KO;
  }

  auto playerShotPacket = PacketBuilder::makePlayerShoot(
      pos.first, pos.second, projectileType,
      room->getGame().fetchAndIncrementSequenceNumber());
  auto playerShotBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(playerShotPacket));

  server.setLastProcessedSeq(client._player_id, packet.sequence_number);
  auto ackPacket =
      PacketBuilder::makeAckPacket(packet.sequence_number, client._player_id);
  auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(ackPacket));
  server.getNetworkManager().sendToClient(client._player_id, ackBuffer);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerShootToRoom(
      server.getNetworkManager(), roomClients, playerShotPacket);

  for (auto &client : roomClients) {
    if (client)
      client->addUnacknowledgedPacket(playerShotPacket.sequence_number,
                                      playerShotBuffer);
  }
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

      auto &game = room->getGame();
      auto disconnectPacket = PacketBuilder::makePlayerDisconnect(
          client._player_id, game.fetchAndIncrementSequenceNumber());
      auto disconnectBuffer = std::make_shared<std::vector<std::uint8_t>>(
          serialization::BitserySerializer::serialize(disconnectPacket));
      broadcast::Broadcast::broadcastPlayerDisconnectToRoom(
          server.getNetworkManager(), roomClients, disconnectPacket);
      for (const auto &roomClient : roomClients) {
        if (roomClient && roomClient->_player_id != client._player_id) {
          roomClient->addUnacknowledgedPacket(disconnectPacket.sequence_number,
                                              disconnectBuffer);
        }
      }
      std::string msg = client._player_name + " has disconnected.";
      auto chatMessagePacket = PacketBuilder::makeChatMessage(
          msg, SERVER_SENDER_ID, 255, 255, 0, 255,
          game.fetchAndIncrementSequenceNumber());
      auto chatMessageBuffer = std::make_shared<std::vector<std::uint8_t>>(
          serialization::BitserySerializer::serialize(chatMessagePacket));
      broadcast::Broadcast::broadcastMessageToRoom(
          server.getNetworkManager(), roomClients, chatMessagePacket);
      for (const auto &roomClient : roomClients) {
        if (roomClient && roomClient->_player_id != client._player_id) {
          roomClient->addUnacknowledgedPacket(chatMessagePacket.sequence_number,
                                              chatMessageBuffer);
        }
      }
    }
  }
  server.enqueueClientRemoval(client._player_id);
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

  if (client._room_id != NO_ROOM) {
    std::cout << "[CREATE ROOM] Client " << client._player_id
              << " already in room " << client._room_id
              << ", ignoring duplicate create request" << std::endl;

    auto room = server.getGameManager().getRoom(client._room_id);
    auto response = PacketBuilder::makeCreateRoomResponse(
        RoomError::SUCCESS, client._room_id,
        room != nullptr ? room->getGame().fetchAndIncrementSequenceNumber()
                        : packet.sequence_number);
    auto serializedBuffer =
        serialization::BitserySerializer::serialize(response);
    server.getNetworkManager().sendToClient(
        client._player_id,
        reinterpret_cast<const char *>(serializedBuffer.data()),
        serializedBuffer.size());
    client.addUnacknowledgedPacket(
        response.sequence_number,
        std::make_shared<std::vector<std::uint8_t>>(serializedBuffer));
    return OK;
  }

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

  auto &game = server.getGameManager().getRoom(client._room_id)->getGame();
  auto response = PacketBuilder::makeCreateRoomResponse(
      RoomError::SUCCESS, newRoom->getRoomId(),
      game.fetchAndIncrementSequenceNumber());

  auto serializedBuffer = serialization::BitserySerializer::serialize(response);

  if (serializedBuffer.empty()) {
    std::cerr << "[ERROR] Failed to serialize CreateRoomResponse for client "
              << client._player_id << std::endl;
    server.getGameManager().leaveRoom(sharedClient);
    server.getGameManager().destroyRoom(newRoom->getRoomId());
    client._state = server::ClientState::CONNECTED_MENU;
    return KO;
  }

  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  client.addUnacknowledgedPacket(
      response.sequence_number,
      std::make_shared<std::vector<std::uint8_t>>(serializedBuffer));

  std::cout << "[CREATE ROOM] Client " << client._player_id
            << " created and joined room " << newRoom->getRoomId() << " ("
            << roomName << ")" << std::endl;

  return OK;
}

/**
 * @brief Handle a client's JoinRoom request and send a JoinRoomResponse with
 * the result.
 *
 * Deserializes the incoming JoinRoomPacket, validates room existence and
 * password, attempts to join and initialize the player in the room, and sends a
 * sequence-numbered JoinRoomResponse indicating the resulting RoomError.
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

  auto ackPacket =
      PacketBuilder::makeAckPacket(packet.sequence_number, client._player_id);
  auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(ackPacket));

  server.getNetworkManager().sendToClient(
      client._player_id, reinterpret_cast<const char *>(ackBuffer->data()),
      ackBuffer->size());
  auto room = server.getGameManager().getRoom(packet.room_id);
  if (!room) {
    ResponseHelper::sendJoinRoomResponse(server, client, packet.sequence_number,
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

      ResponseHelper::sendJoinRoomResponse(
          server, client, packet.sequence_number, RoomError::WRONG_PASSWORD);
      return KO;
    }
  }

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    ResponseHelper::sendJoinRoomResponse(server, client, packet.sequence_number,
                                         RoomError::UNKNOWN_ERROR);
    return KO;
  }

  bool joinSuccess =
      server.getGameManager().joinRoom(packet.room_id, sharedClient);
  if (!joinSuccess) {
    ResponseHelper::sendJoinRoomResponse(server, client, packet.sequence_number,
                                         RoomError::ROOM_FULL);
    return KO;
  }

  client._state = server::ClientState::IN_ROOM_WAITING;

  if (!server.initializePlayerInRoom(client)) {
    std::cerr << "[ERROR] Failed to initialize player in room" << std::endl;
    server.getGameManager().leaveRoom(sharedClient);
    ResponseHelper::sendJoinRoomResponse(server, client, packet.sequence_number,
                                         RoomError::UNKNOWN_ERROR);
    client._state = server::ClientState::CONNECTED_MENU;
    return KO;
  }

  ResponseHelper::sendJoinRoomResponse(server, client, packet.sequence_number,
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
 * @brief Handle a matchmaking request and place the client into a matchmaking
 * room.
 *
 * Deserializes a MatchmakingRequestPacket, sends an acknowledgment for the
 * request, then attempts to join the client to an existing suitable room or
 * creates and joins a new matchmaking room. On success the client is
 * transitioned to IN_ROOM_WAITING, initialized in the room, and a success
 * matchmaking response is sent; on failure an appropriate error response is
 * sent and the client's state is restored where applicable.
 *
 * @param server Server managing rooms, clients, and game state.
 * @param client Client issuing the matchmaking request; may be updated.
 * @param data Pointer to the serialized MatchmakingRequestPacket data.
 * @param size Size in bytes of the serialized data buffer.
 * @return int `OK` if the client was placed in a room and initialized, `KO`
 * otherwise.
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

  auto ackPacket =
      PacketBuilder::makeAckPacket(packet.sequence_number, client._player_id);
  auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(ackPacket));

  server.getNetworkManager().sendToClient(
      client._player_id, reinterpret_cast<const char *>(ackBuffer->data()),
      ackBuffer->size());

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    ResponseHelper::sendMatchmakingResponse(
        server, client, packet.sequence_number, RoomError::UNKNOWN_ERROR);
    return KO;
  }
  bool joinSuccess = server.getGameManager().joinAnyRoom(sharedClient);

  if (!joinSuccess) {
    auto newRoom = server.getGameManager().createRoom("Matchmaking Room");
    if (!newRoom) {
      std::cerr << "[ERROR] Client " << client._player_id
                << " failed to create new room for matchmaking" << std::endl;
      ResponseHelper::sendMatchmakingResponse(
          server, client, packet.sequence_number, RoomError::UNKNOWN_ERROR);
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

      ResponseHelper::sendMatchmakingResponse(
          server, client, packet.sequence_number, RoomError::SUCCESS);
    } else {
      std::cerr << "[ERROR] Client " << client._player_id
                << " failed to create/join new room for matchmaking"
                << std::endl;
      ResponseHelper::sendMatchmakingResponse(
          server, client, packet.sequence_number, RoomError::UNKNOWN_ERROR);
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

    ResponseHelper::sendMatchmakingResponse(
        server, client, packet.sequence_number, RoomError::SUCCESS);
  }

  return OK;
}

/**
 * @brief Process a player's movement input and enqueue it for the room's input
 * system.
 *
 * Deserializes a PlayerInputPacket from the provided buffer, validates the
 * client and room state, converts the packet into an ecs::PlayerInput, and
 * queues it in the room's ServerInputSystem for the client's entity.
 *
 * @param server Server instance used to access game and network managers.
 * @param client Client that sent the input; used to identify the player entity
 * and room.
 * @param data Pointer to the raw packet data containing a serialized
 * PlayerInputPacket.
 * @param size Size of the raw packet data buffer in bytes.
 * @return int `OK` if the packet was deserialized and the input queued; `KO` on
 * error (deserialization failure, missing/inactive room, missing input system,
 * or invalid entity id).
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

/**
 * @brief Handle an incoming AckPacket and mark the referenced sequence as
 * acknowledged for the sending client.
 *
 * Validates that the AckPacket's player_id matches the sender and removes the
 * acknowledged sequence number from the client's unacknowledged packet set.
 *
 * @param data Pointer to the serialized AckPacket bytes.
 * @param size Number of bytes available at `data`.
 * @return int `OK` on success, `KO` on failure (for example, deserialization
 * failure or player_id mismatch).
 */
int packet::AckPacketHandler::handlePacket(server::Server &server,
                                           server::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<AckPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize AckPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const AckPacket &packet = deserializedPacket.value();
  if (packet.player_id != static_cast<std::uint32_t>(client._player_id)) {
    std::cerr << "[WARNING] ACK player_id mismatch (packet=" << packet.player_id
              << ", conn=" << client._player_id << ")" << std::endl;
    return KO;
  }

  client.removeAcknowledgedPacket(packet.sequence_number);

  return OK;
}

/**
 * @brief Handle a RequestChallengePacket from a client: acknowledge the
 * request, generate a challenge for the client's room, send a
 * ChallengeResponse, and track the response as unacknowledged.
 *
 * Validates packet deserialization and room existence. On success an AckPacket
 * for the incoming request is sent to the client, a challenge string is
 * created, a ChallengeResponse packet is built using the room's sequence
 * number, sent to the client, and the serialized response is stored as an
 * unacknowledged packet tied to the response's sequence number.
 *
 * @param server Server instance used for room lookup, challenge creation and
 * network I/O.
 * @param client Client that sent the request; used for player id and
 * unacknowledged tracking.
 * @param data Pointer to the incoming packet bytes.
 * @param size Size in bytes of the incoming packet data.
 * @return int `OK` on success, `KO` on failure (deserialization, missing room,
 * or serialization error).
 */
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

  auto ackPacket =
      PacketBuilder::makeAckPacket(packet.sequence_number, client._player_id);
  auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
      serialization::BitserySerializer::serialize(ackPacket));

  server.getNetworkManager().sendToClient(
      client._player_id, reinterpret_cast<const char *>(ackBuffer->data()),
      ackBuffer->size());

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

  auto responsePacket = PacketBuilder::makeChallengeResponse(
      challenge, time, room->getGame().fetchAndIncrementSequenceNumber());

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

  client.addUnacknowledgedPacket(
      responsePacket.sequence_number,
      std::make_shared<std::vector<std::uint8_t>>(serializedBuffer));
  return OK;
}

int packet::ScoreboardRequestHandler::handlePacket(server::Server &server,
                                                   server::Client &client,
                                                   const char *data,
                                                   std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<ScoreboardRequestPacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize ScoreboardRequestPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const ScoreboardRequestPacket &packet = deserializedPacket.value();
  std::uint32_t limit = std::clamp(packet.limit, 1u, MAX_TOP_SCORES);
  auto scoreData =
      server.getDatabaseManager().getTopScores(static_cast<int>(limit));

  std::vector<ScoreEntry> scoreEntries;
  auto players = server.getDatabaseManager().getAllPlayers();
  std::unordered_map<int, std::string> playerMap;
  for (const auto &playerData : players)
    playerMap[playerData.id] = playerData.username;

  for (const auto &data : scoreData) {
    ScoreEntry entry;
    auto it = playerMap.find(data.player_id);
    if (it != playerMap.end()) {
      entry.player_name = it->second;
      entry.score = data.score;
      scoreEntries.push_back(entry);
    }
  }

  auto responsePacket = PacketBuilder::makeScoreboardResponse(scoreEntries);
  serialization::Buffer responseBuffer =
      serialization::BitserySerializer::serialize(responsePacket);

  if (responseBuffer.empty()) {
    std::cerr
        << "[ERROR] Failed to serialize ScoreboardResponsePacket for client "
        << client._player_id << std::endl;
    return KO;
  }

  server.getNetworkManager().sendToClient(
      client._player_id, reinterpret_cast<const char *>(responseBuffer.data()),
      responseBuffer.size());

  return OK;
}
