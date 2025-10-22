#pragma once

#include <asio/steady_timer.hpp>
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include "Client.hpp"
#include "Game.hpp"
#include "Macro.hpp"

namespace game {

  enum class RoomStatus {
    WAITING,
    STARTING,
    RUNNING,
    FINISHED
  };

  class GameRoom {
    public:
      GameRoom(std::uint32_t room_id, std::uint16_t max_players)
          : _room_id(room_id),
            _max_players(max_players),
            _state(RoomStatus::WAITING),
            _game(std::make_unique<Game>()),
            _countdown(0),
            _countdown_timer(nullptr) {
      }

      ~GameRoom() {
        stop();
      }

      std::uint32_t getRoomId() const {
        return _room_id;
      }

      std::string getRoomName() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _room_name;
      }

      void setRoomName(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _room_name = name;
      }

      bool isFull() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _clients.size() >= _max_players;
      }

      bool isEmpty() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _clients.empty();
      }

      size_t getPlayerCount() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _clients.size();
      }

      RoomStatus getState() const {
        return _state.load();
      }

      bool canJoin() const {
        RoomStatus state = _state.load();
        return (state == RoomStatus::WAITING ||
                state == RoomStatus::STARTING) &&
               !isFull() && !isPrivate();
      }

      bool addClient(std::shared_ptr<server::Client> client) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
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
        std::unique_lock<std::shared_mutex> lock(_mutex);
        for (auto it = _clients.begin(); it != _clients.end(); ++it) {
          if ((*it)->_player_id == player_id) {
            (*it)->_room_id = NO_ROOM;
            _clients.erase(it);
            break;
          }
        }
      }

      std::vector<std::shared_ptr<server::Client>> getClients() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _clients;
      }

      Game& getGame() {
        return *_game;
      }

      void start() {
        RoomStatus expected = RoomStatus::STARTING;
        if (_state.compare_exchange_strong(expected, RoomStatus::RUNNING)) {
          _game->start();
        }
      }

      void stop() {
        if (_state.load() == RoomStatus::FINISHED) {
          return;
        }
        {
          std::unique_lock<std::shared_mutex> lock(_mutex);
          if (_countdown_timer) {
            _countdown_timer->cancel();
            _countdown_timer.reset();
          }
        }
        _state.store(RoomStatus::FINISHED);
        if (_game) {
          _game->stop();
        }
      }

      bool isActive() const {
        return _state.load() == RoomStatus::RUNNING;
      }

      void setPassword(const std::string& password) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _password = password;
      }

      std::string getPassword() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _password;
      }

      bool checkPassword(const std::string& password) const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _password == password;
      }

      bool hasPassword() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return !_password.empty();
      }

      bool isPrivate() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _is_private;
      }

      bool setPrivate(bool is_private) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _is_private = is_private;
        return _is_private;
      }

      std::uint16_t getMaxPlayers() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _max_players;
      }

      void startCountdown(int seconds,
                          std::shared_ptr<asio::steady_timer> timer) {
        RoomStatus expected = RoomStatus::WAITING;
        if (_state.compare_exchange_strong(expected, RoomStatus::STARTING)) {
          _countdown = seconds;
          std::unique_lock<std::shared_mutex> lock(_mutex);
          _countdown_timer = timer;
        }
      }

      int getCountdownValue() const {
        return _countdown.load();
      }

      void decrementCountdown() {
        int current = _countdown.load();
        if (current > 0) {
          _countdown--;
        }
      }

    private:
      std::uint32_t _room_id;
      std::string _room_name;
      const std::uint16_t _max_players;
      std::string _password;

      bool _is_private = false;

      std::atomic<RoomStatus> _state;
      std::unique_ptr<Game> _game;
      std::atomic<int> _countdown;
      std::shared_ptr<asio::steady_timer> _countdown_timer;
      std::vector<std::shared_ptr<server::Client>> _clients;
      mutable std::shared_mutex _mutex;
  };

}  // namespace game
