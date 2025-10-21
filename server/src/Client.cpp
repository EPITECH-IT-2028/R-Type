#include "Client.hpp"

void server::Client::addUnacknowledgedPacket(
    std::uint32_t sequence_number,
    std::shared_ptr<std::vector<uint8_t>> packetData) {
  _unacknowledged_packets[sequence_number] = packetData;
}

void server::Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
  _unacknowledged_packets.erase(sequence_number);
}

void server::Client::resendUnacknowledgedPackets(
    network::ServerNetworkManager &networkManager) {
  for (const auto &[seq, packetData] : _unacknowledged_packets) {
    networkManager.sendToClient(_player_id, packetData);
  }
}
