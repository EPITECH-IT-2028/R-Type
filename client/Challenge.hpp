#pragma once

#include <cstdint>
#include <string>

namespace client {
  class Challenge {
    public:
      Challenge()
          : _currentChallenge(""),
            _challengeTimestamp(0),
            _roomId(0),
            _waitingChallenge(false),
            _challengeReceived(false) {
      }

      void setChallenge(const std::string &challenge, std::uint32_t timestamp) {
        _currentChallenge = challenge;
        _challengeTimestamp = timestamp;
        _challengeReceived = true;
        _waitingChallenge = false;
      }

      void setRoomId(std::uint32_t roomId) {
        _roomId = roomId;
      }

      void setWaitingChallenge(bool waiting) {
        _waitingChallenge = waiting;
      }

      std::string getChallenge() const {
        return _currentChallenge;
      }

      std::uint32_t getRoomId() const {
        return _roomId;
      }

      bool isChallengeReceived() const {
        return _challengeReceived;
      }

      void reset() {
        _currentChallenge = "";
        _challengeTimestamp = 0;
        _roomId = 0;
        _waitingChallenge = false;
        _challengeReceived = false;
      }

    private:
      std::string _currentChallenge;
      std::uint32_t _challengeTimestamp;
      std::uint32_t _roomId;
      bool _waitingChallenge;
      bool _challengeReceived;
  };
}  // namespace client
