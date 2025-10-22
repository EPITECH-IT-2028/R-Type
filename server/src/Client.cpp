#include "Client.hpp"
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <vector>

void server::Client::addUnacknowledgedPacket(
    std::uint32_t sequence_number,
    std::shared_ptr<std::vector<uint8_t>> packetData) {
  UnacknowledgedPacket packet;
  packet.data = packetData;
  packet.resend_count = 0;
  packet.last_sent = std::chrono::steady_clock::now();
  _unacknowledged_packets[sequence_number] = packet;
}

void server::Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
  auto it = _unacknowledged_packets.find(sequence_number);
  if (it != _unacknowledged_packets.end()) {
    _unacknowledged_packets.erase(it);
    std::cout << "[ACK] Server player " << _player_id << " now has "
              << _unacknowledged_packets.size()
              << " unacknowledged packets remaining" << std::endl;
  } else {
    std::cout << "[WARNING] Server tried to remove non-existent packet "
              << sequence_number << " from player " << _player_id << std::endl;
  }
}

void server::Client::resendUnacknowledgedPackets(
    network::ServerNetworkManager &networkManager) {
  const int MAX_RESEND_ATTEMPTS = 5;
  auto now = std::chrono::steady_clock::now();
  const auto MIN_RESEND_INTERVAL = std::chrono::milliseconds(500);

  std::vector<uint32_t> packets_to_remove;

  for (auto &[seq, packetData] : _unacknowledged_packets) {
    static std::unordered_map<uint32_t, int> resend_counts;

    if (resend_counts[seq] >= MAX_RESEND_ATTEMPTS) {
      packets_to_remove.push_back(seq);
      continue;
    }

    networkManager.sendToClient(_player_id, packetData.data);
    resend_counts[seq]++;
  }

  for (uint32_t seq : packets_to_remove) {
    _unacknowledged_packets.erase(seq);
  }
}
