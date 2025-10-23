#include "Client.hpp"
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <vector>

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

void server::Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
  std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
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
