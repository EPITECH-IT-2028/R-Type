#include "Client.hpp"
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "Macro.hpp"

/**
 * @brief Store a packet as unacknowledged for retransmission tracking.
 *
 * Initializes tracking state for the packet identified by sequence_number so it
 * can be retransmitted until acknowledged.
 *
 * @param sequence_number Sequence number that identifies the packet.
 * @param packetData Shared pointer to the packet bytes to retain for
 * retransmission.
 *
 * @note The entry's resend count is initialized to 0 and the last-sent
 * timestamp is recorded as the current steady clock time.
 */
void server::Client::addUnacknowledgedPacket(
    std::uint32_t sequence_number,
    std::shared_ptr<std::vector<uint8_t>> packetData) {
  std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
  UnacknowledgedPacket packet;
  packet.data = packetData;
  packet.resend_count = 0;
  packet.last_sent = std::chrono::steady_clock::now();
  _unacknowledged_packets[sequence_number] = packet;
}

/**
 * @brief Remove an unacknowledged packet tracked by sequence number.
 *
 * If an entry with the given sequence number exists it is erased; if no entry
 * exists the call has no effect.
 *
 * @param sequence_number Sequence number of the packet to remove.
 */
void server::Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
  std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
  auto it = _unacknowledged_packets.find(sequence_number);
  if (it != _unacknowledged_packets.end()) {
    _unacknowledged_packets.erase(it);
  }
}

/**
 * @brief Retransmits unacknowledged packets to this client and drops those that exceeded retry limits.
 *
 * For each stored unacknowledged packet whose last send time is older than MIN_RESEND_PACKET_DELAY,
 * increments its resend count and sends it to the client; packets whose resend count is greater than
 * or equal to MAX_RESEND_ATTEMPTS are removed and not retransmitted.
 *
 * @param networkManager Server network manager used to send packets to the client.
 */
void server::Client::resendUnacknowledgedPackets(
    network::ServerNetworkManager &networkManager) {
  const auto MIN_RESEND_INTERVAL =
      std::chrono::milliseconds(MIN_RESEND_PACKET_DELAY);
  const auto now = std::chrono::steady_clock::now();

  std::vector<std::shared_ptr<std::vector<uint8_t>>> toSend;
  std::vector<uint32_t> toDrop;
  {
    std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
    for (auto &[seq, packet] : _unacknowledged_packets) {
      if (now - packet.last_sent < MIN_RESEND_INTERVAL)
        continue;
      if (packet.resend_count >= MAX_RESEND_ATTEMPTS) {
        toDrop.push_back(seq);
        continue;
      }
      packet.resend_count++;
      packet.last_sent = now;
      toSend.push_back(packet.data);
    }
    for (auto seq : toDrop) {
      _unacknowledged_packets.erase(seq);
    }
  }
  for (auto &buf : toSend) {
    networkManager.sendToClient(_player_id, buf);
  }
}