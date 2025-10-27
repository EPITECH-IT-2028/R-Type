#include "Client.hpp"
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <vector>

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
 * @brief Remove the stored unacknowledged packet with the given sequence
 * number.
 *
 * If the packet exists, it is erased from the client's unacknowledged-packet
 * map; otherwise a warning is emitted indicating the packet was not found.
 *
 * @param sequence_number Sequence number of the packet to remove.
 */
void server::Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
  std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
  auto it = _unacknowledged_packets.find(sequence_number);
  if (it != _unacknowledged_packets.end()) {
    _unacknowledged_packets.erase(it);
  } else {
    std::cout << "[WARNING] Server tried to remove non-existent packet "
              << sequence_number << " from player " << _player_id << std::endl;
  }
}

/**
 * @brief Retransmits stored unacknowledged packets to the client and removes
 * packets that exceeded retry limits.
 *
 * Iterates the client's internal unacknowledged-packet map, retransmitting each
 * packet and tracking per-sequence resend attempts. Packets whose resend
 * attempts reach the configured maximum are removed from the unacknowledged map
 * after processing.
 *
 * Notes:
 * - Resend attempts are tracked in a static map that persists across calls to
 * this function.
 * - The function uses the provided network manager to send each packet to the
 * client identified by this client's player ID.
 */
void server::Client::resendUnacknowledgedPackets(
    network::ServerNetworkManager &networkManager) {
  const int MAX_RESEND_ATTEMPTS = 5;
  const auto MIN_RESEND_INTERVAL = std::chrono::milliseconds(500);
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
