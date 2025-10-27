#pragma once
#include <cstdint>

namespace network {
  class PacketLossMonitor {
    public:
      PacketLossMonitor() = default;
      ~PacketLossMonitor() = default;

      void onReceived(std::uint32_t seq) {
        if (_last_sequence_number == UINT32_MAX) {
          _last_sequence_number = seq;
          return;
        }

        uint32_t expected = _last_sequence_number + 1;
        if (seq > expected) {
          _lost_packet_count += seq - expected;
          _received_packet_count += 1;
        } else if (seq == expected) {
          _received_packet_count += 1;
        }

        _last_sequence_number = seq;
      }

      double lossRatio() const {
        std::uint32_t total = _lost_packet_count + _received_packet_count;

        if (total != 0)
          return static_cast<double>(_lost_packet_count) / total;
        return 0.0;
      }

      void reset() {
        _last_sequence_number = UINT32_MAX;
        _received_packet_count = 0;
        _lost_packet_count = 0;
      }

    private:
      std::uint32_t _last_sequence_number = UINT32_MAX;
      std::uint32_t _received_packet_count = 0;
      std::uint32_t _lost_packet_count = 0;
  };
}  // namespace network