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
      /**
       * @brief Constructs a GameRoom with the given identifier and capacity.
       *
       * Initializes the room in the WAITING state, creates an internal Game
       * instance, sets the countdown to 0, and clears any countdown timer.
       *
       * @param room_id Numeric identifier for the room.
       * @param max_players Maximum number of players allowed in the room.
       */
      GameRoom(std::uint32_t room_id, std::uint16_t max_players)
          : _room_id(room_id),
            _max_players(max_players),
            _state(RoomStatus::WAITING),
            _game(std::make_unique<Game>()),
            _countdown(0),
            _countdown_timer(nullptr) {
      }

      /**
       * @brief Cleans up the room and stops any active game or countdown.
       *
       * Ensures the room is transitioned to a stopped/finished state, cancels
       * any pending countdown timer, and stops the contained Game before
       * destruction.
       */
      ~GameRoom() {
        stop();
      }

      /**
       * @brief Retrieves the room's unique identifier.
       *
       * @return The room identifier (32-bit unsigned integer).
       */
      std::uint32_t getRoomId() const {
        return _room_id;
      }

      /**
       * @brief Gets the room's name.
       *
       * @return The current room name.
       */
      std::string getRoomName() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _room_name;
      }

      /**
       * @brief Set the room's display name.
       *
       * @param name New display name for the room.
       */
      void setRoomName(const std::string &name) {
        std::lock_guard<std::shared_mutex> lock(_mutex);
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

      /**
       * @brief Retrieves the room's current status.
       *
       * @return RoomStatus The current room status: WAITING, STARTING, RUNNING,
       * or FINISHED.
       */
      RoomStatus getState() const {
        return _state.load();
      }

      /**
       * @brief Determines whether a client may join the room.
       *
       * @return `true` if the room is in `WAITING` or `STARTING` state, is not
       * full, and is not private; `false` otherwise.
       */
      bool canJoin() const {
        RoomStatus state = _state.load();
        return (state == RoomStatus::WAITING ||
                state == RoomStatus::STARTING) &&
               !isFull() && !isPrivate();
      }

      /**
       * @brief Adds a client to the room if the client is connected and there
       * is available space.
       *
       * If added, the client's room ID is set to this room's ID. The operation
       * is performed with internal synchronization to protect the room's client
       * list.
       *
       * @param client Shared pointer to the client to add; must be non-null and
       * connected.
       * @return `true` if the client was added and their room ID assigned,
       * `false` otherwise.
       */
      bool addClient(std::shared_ptr<server::Client> client) {
        std::lock_guard<std::shared_mutex> lock(_mutex);
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

      /**
       * @brief Remove the first connected client with the given player ID from the room.
       *
       * Acquires the room mutex, finds the first client whose `_player_id` equals `player_id`,
       * sets that client's `_room_id` to `NO_ROOM`, and removes the client from the room's
       * client list. If no matching client is found, the function returns without changes.
       *
       * @param player_id The player identifier of the client to remove.
       */
      void removeClient(int player_id) {
        std::lock_guard<std::shared_mutex> lock(_mutex);
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

      /**
       * @brief Accesses the room's contained Game instance.
       *
       * @return Game& Reference to the room's Game instance.
       */
      Game &getGame() {
        return *_game;
      }

      /**
       * @brief Transition the room to the running state and start the contained
       * game.
       *
       * Attempts to change the room state to RUNNING. If the current state is
       * STARTING or, if not, WAITING, the state is set to RUNNING and the
       * contained Game's start() method is invoked.
       */
      void start() {
        RoomStatus expected = RoomStatus::STARTING;
        if (_state.compare_exchange_strong(expected, RoomStatus::RUNNING)) {
          _game->start();
        }
      }

      /**
       * @brief Stops the room and its game, transitioning the room to FINISHED.
       *
       * Cancels and clears any active countdown timer, sets the room state to
       * FINISHED, and stops the contained Game if present. If the room is
       * already in the FINISHED state, no action is taken.
       */
      void stop() {
        if (_state.load() == RoomStatus::FINISHED) {
          return;
        }
        {
          std::lock_guard<std::shared_mutex> lock(_mutex);
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

      /**
       * @brief Checks whether the room's game is currently running.
       *
       * @return `true` if the room state is `RoomStatus::RUNNING`, `false`
       * otherwise.
       */
      bool isActive() const {
        return _state.load() == RoomStatus::RUNNING;
      }

      /**
       * @brief Sets the room's access password.
       *
       * Replaces any existing password with the provided value. An empty string
       * clears the password.
       *
       * @param password New password for the room; an empty string removes the
       * password.
       */
      void setPassword(const std::string &password) {
        std::lock_guard<std::shared_mutex> lock(_mutex);
        _password = password;
      }

      /**
       * @brief Retrieves the room's password.
       *
       * This accessor is thread-safe.
       *
       * @return std::string The stored password; empty string if no password is
       * set.
       */
      std::string getPassword() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _password;
      }

      /**
       * @brief Checks whether the provided password matches the room's stored
       * password.
       *
       * @param password Candidate password to verify.
       * @return true if `password` equals the room's stored password, false
       * otherwise.
       */
      bool checkPassword(const std::string &password) const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _password == password;
      }

      /**
       * @brief Indicates whether the room has a non-empty access password.
       *
       * This method is thread-safe.
       *
       * @return true if a password is set, false otherwise.
       */
      bool hasPassword() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return !_password.empty();
      }

      /**
       * @brief Indicates whether the room is private.
       *
       * Thread-safe accessor for the room's privacy flag.
       *
       * @return `true` if the room is private, `false` otherwise.
       */
      bool isPrivate() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _is_private;
      }

      /**
       * @brief Set the room's privacy flag.
       *
       * @param is_private `true` to mark the room as private, `false` to make
       * it public.
       * @return `true` if the room is private after the update, `false`
       * otherwise.
       */
      bool setPrivate(bool is_private) {
        std::lock_guard<std::shared_mutex> lock(_mutex);
        _is_private = is_private;
        return _is_private;
      }

      /**
       * @brief Obtain the configured maximum number of players allowed in the
       * room.
       *
       * This accessor is thread-safe.
       *
       * @return std::uint16_t The maximum number of players permitted in this
       * room.
       */
      std::uint16_t getMaxPlayers() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _max_players;
      }

      /**
       * @brief Begin a room start countdown and register its timer.
       *
       * If the room is currently in WAITING, atomically transition it to
       * STARTING, set the countdown value to seconds, and store the provided
       * steady_timer for countdown events. If the state is not WAITING, this
       * function has no effect.
       *
       * @param seconds Number of seconds to count down from (must be >= 0).
       * @param timer Shared pointer to an asio::steady_timer used to drive the
       * countdown; stored if the transition succeeds.
       */
      void startCountdown(int seconds,
                          std::shared_ptr<asio::steady_timer> timer) {
        std::lock_guard<std::shared_mutex> lock(_mutex);
        RoomStatus expected = RoomStatus::WAITING;
        if (_state.compare_exchange_strong(expected, RoomStatus::STARTING)) {
          _countdown = seconds;
          _countdown_timer = timer;
        }
      }

      /**
       * @brief Retrieves the current countdown timer value.
       *
       * @return int The current countdown value in seconds; may be concurrently
       * modified by other threads.
       */
      int getCountdownValue() const {
        return _countdown.load();
      }

      /**
       * @brief Decrements the room countdown by one if its current value is
       * greater than zero.
       *
       * If the countdown is zero or negative, this function leaves it
       * unchanged.
       */
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