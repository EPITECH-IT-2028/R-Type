#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "Game.hpp"
#include "Server.hpp"

namespace game {

  enum class RoomStatus {
    WAITING,
    STARTING,
    RUNNING,
    FINISHED
  };

  class GameRoom {
    public:
      GameRoom(uint32_t room_id, uint16_t max_players)
          : _room_id(room_id),
            _max_players(max_players),
            _state(RoomStatus::WAITING),
            _game(std::make_unique<Game>()) {
      }

      ~GameRoom() {
        stop();
      }

      uint32_t getRoomId() const {
        return _room_id;
      }

      std::string getRoomName() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _room_name;
      }

      void setRoomName(const std::string& name) {
        std::lock_guard<std::mutex> lock(_mutex);
        _room_name = name;
      }

      bool isFull() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clients.size() >= _max_players;
      }

      bool isEmpty() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clients.empty();
      }

      size_t getPlayerCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clients.size();
      }

      RoomStatus getState() const {
        return _state.load();
      }

      bool canJoin() const {
        return _state.load() == RoomStatus::WAITING && !isFull();
      }

      bool addClient(std::shared_ptr<server::Client> client) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!client || client->_connected == false) {
          return false;
        }
        if (_clients.size() >= _max_players) {
          return false;
        }
        _clients.push_back(client);
        client->_room_id = _room_id;
        return true;
      }

      void removeClient(int player_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto it = _clients.begin(); it != _clients.end(); ++it) {
          if ((*it)->_player_id == player_id) {
            _clients.erase(it);
            break;
          }
        }
      }

      std::vector<std::shared_ptr<server::Client>> getClients() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clients;
      }

      Game& getGame() {
        return *_game;
      }

      void start() {
        if (_state.load() == RoomStatus::WAITING) {
          _state.store(RoomStatus::RUNNING);
          _game->start();
        }
      }

      void stop() {
        if (_state.load() == RoomStatus::FINISHED) {
          return;
        }

        _state.store(RoomStatus::FINISHED);
        if (_game) {
          _game->stop();
        }
      }

      bool isActive() const {
        return _state.load() == RoomStatus::RUNNING;
      }

    private:
      uint32_t _room_id;
      std::string _room_name;
      uint16_t _max_players;
      std::atomic<RoomStatus> _state;
      std::unique_ptr<Game> _game;
      std::vector<std::shared_ptr<server::Client>> _clients;
      mutable std::mutex _mutex;
  };

}  // namespace game
