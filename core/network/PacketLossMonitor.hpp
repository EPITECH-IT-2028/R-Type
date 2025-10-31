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
          _received_packet_count += 1;
          return;
        }

        const std::uint32_t expected = _last_sequence_number + 1;
        const std::int32_t delta = static_cast<std::int32_t>(seq - expected);

        if (delta == 0) {
          _received_packet_count += 1;
          _last_sequence_number = seq;
          return;
        }
        if (delta > 0) {
          _lost_packet_count += static_cast<std::uint32_t>(delta);
          _received_packet_count += 1;
          _last_sequence_number = seq;
          return;
        }
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