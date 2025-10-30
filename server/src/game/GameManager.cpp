#include "GameManager.hpp"
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#include "GameRoom.hpp"
#include "Macro.hpp"

game::GameManager::GameManager(int maxPlayers)
    : _maxPlayers(maxPlayers), _nextRoomId(1) {
}

/**
 * @brief Stop and clean up all managed game rooms.
 *
 * Ensures every active room is shut down and resources are released before the
 * manager is destroyed.
 */
game::GameManager::~GameManager() {
  shutdownRooms();
}

/**
 * @brief Creates a new game room and registers it with the manager.
 *
 * If `roomName` is empty a default name "Room <id>" is assigned. If `password`
 * is non-empty the room password is set and the room is marked private.
 *
 * @param roomName Desired room name; empty to use a generated default.
 * @param password Optional password; non-empty value makes the room private.
 * @return std::shared_ptr<game::GameRoom> Shared pointer to the newly created
 * room.
 */
std::shared_ptr<game::GameRoom> game::GameManager::createRoom(
    const std::string &roomName, const std::string &password) {
  std::scoped_lock lock(_roomMutex);
  int roomId = _nextRoomId++;

  auto room = std::make_shared<GameRoom>(roomId, _maxPlayers);

  if (!roomName.empty()) {
    room->setRoomName(roomName);
  } else {
    room->setRoomName("Room " + std::to_string(roomId));
  }

  if (!password.empty()) {
    room->setPassword(password);
    room->setPrivate(true);
  }

  _rooms[roomId] = room;
  return room;
}

/**
 * @brief Removes a room from the manager and stops it if it existed.
 *
 * If a room with the given id is present, the room is removed from the manager
 * and its stop() method is invoked to perform cleanup.
 *
 * @param roomId Identifier of the room to destroy.
 * @return true if a room with `roomId` was found and stopped, `false`
 * otherwise.
 */
bool game::GameManager::destroyRoom(std::uint32_t roomId) {
  std::shared_ptr<GameRoom> roomToStop;
  {
    std::scoped_lock lock(_roomMutex);
    auto it = _rooms.find(roomId);
    if (it != _rooms.end()) {
      roomToStop = it->second;
      _rooms.erase(it);
    }
  }
  if (roomToStop) {
    roomToStop->stop();
    return true;
  }
  return false;
}

/**
 * @brief Finds the first room that can accept additional players.
 *
 * @return std::shared_ptr<game::GameRoom> Shared pointer to the first joinable
 * room, or `nullptr` if no such room exists.
 */
std::shared_ptr<game::GameRoom> game::GameManager::findAvailableRoom() {
  std::scoped_lock lock(_roomMutex);
  for (const auto &[id, room] : _rooms) {
    if (room->canJoin()) {
      return room;
    }
  }
  return nullptr;
}

/**
 * @brief Retrieve the room with the specified identifier.
 *
 * @param roomId Identifier of the room to look up.
 * @return std::shared_ptr<game::GameRoom> Shared pointer to the room if found,
 * `nullptr` otherwise.
 */
std::shared_ptr<game::GameRoom> game::GameManager::getRoom(
    std::uint32_t roomId) const {
  std::scoped_lock lock(_roomMutex);
  auto it = _rooms.find(roomId);
  if (it != _rooms.end()) {
    return it->second;
  }
  return nullptr;
}

/**
 * @brief Attempts to add a client to the specified game room.
 *
 * If the room exists and accepts the client, the client will be added to that
 * room.
 *
 * @param roomId Identifier of the room to join.
 * @param client Shared pointer to the client to add to the room.
 * @return true if the client was added to the room, false if the room does not
 * exist or the client could not be added.
 */
bool game::GameManager::joinRoom(std::uint32_t roomId,
                                 std::shared_ptr<server::Client> client) {
  std::scoped_lock lock(_roomMutex);

  auto it = _rooms.find(roomId);
  if (it == _rooms.end()) {
    return false;
  }

  auto room = it->second;

  if (!room->addClient(client))
    return false;

  std::cout << "[ROOM] Client " << client->_player_id << " joined room "
            << roomId << std::endl;

  return true;
}

bool game::GameManager::joinAnyRoom(std::shared_ptr<server::Client> client) {
  std::scoped_lock lock(_roomMutex);
  for (const auto &[id, room] : _rooms) {
    if (room->canJoin()) {
      if (room->addClient(client)) {
        std::cout << "[ROOM] Client " << client->_player_id
                  << " joined existing room " << id << std::endl;
        return true;
      }
    }
  }
  return false;
}

void game::GameManager::leaveRoom(std::shared_ptr<server::Client> client) {
  std::shared_ptr<GameRoom> roomToStop;
  {
    std::scoped_lock lock(_roomMutex);
    if (client->_room_id == NO_ROOM)
      return;

    auto it = _rooms.find(client->_room_id);
    if (it != _rooms.end()) {
      auto room = it->second;
      room->removeClient(client->_player_id);
      std::cout << "[ROOM] Client " << client->_player_id << " left room "
                << room->getRoomId() << std::endl;

      if (room->isEmpty()) {
        std::cout << "[ROOM] Room " << room->getRoomId()
                  << " is now empty, cleaning up..." << std::endl;
        roomToStop = room;
        _rooms.erase(it);
      }
    }
  }
  if (roomToStop) {
    roomToStop->stop();
    std::cout << "[ROOM] Room " << roomToStop->getRoomId()
              << " destroyed and cleaned." << std::endl;
  }
  client->_room_id = NO_ROOM;
}

std::vector<std::shared_ptr<game::GameRoom>> game::GameManager::getAllRooms()
    const {
  std::scoped_lock lock(_roomMutex);
  std::vector<std::shared_ptr<GameRoom>> rooms;

  rooms.reserve(_rooms.size());
  for (const auto &pair : _rooms) {
    rooms.push_back(pair.second);
  }
  return rooms;
}

/**
 * @brief Removes finished, empty game rooms from the manager and stops them.
 *
 * Scans all managed rooms, erases those that are empty and in the
 * `RoomStatus::FINISHED` state from the internal container, and then calls
 * `stop()` on each removed room. Room discovery and removal are performed while
 * holding the internal mutex; actual `stop()` calls occur after the mutex is
 * released.
 */
void game::GameManager::removeEmptyRooms() {
  std::vector<std::shared_ptr<GameRoom>> roomsToStop;
  {
    std::lock_guard<std::mutex> lock(_roomMutex);
    std::vector<std::uint32_t> roomIdsToRemove;

    for (auto &[room_id, room] : _rooms) {
      if (room->isEmpty() && room->getState() == RoomStatus::FINISHED) {
        roomIdsToRemove.push_back(room_id);
        roomsToStop.push_back(room);
      }
    }
    for (std::uint32_t room_id : roomIdsToRemove) {
      _rooms.erase(room_id);
      std::cout << "[GAME_MANAGER] Marked room " << room_id << " for cleanup"
                << std::endl;
    }
  }

  for (auto &room : roomsToStop) {
    room->stop();
  }
}

size_t game::GameManager::getRoomCount() const {
  std::scoped_lock lock(_roomMutex);
  return _rooms.size();
}

void game::GameManager::shutdownRooms() {
  std::vector<std::shared_ptr<GameRoom>> roomsToStop;
  {
    std::scoped_lock lock(_roomMutex);
    roomsToStop.reserve(_rooms.size());

    for (auto &[id, room] : _rooms) {
      roomsToStop.push_back(room);
    }
    _rooms.clear();
  }

  for (auto &room : roomsToStop) {
    room->stop();
  }
}
