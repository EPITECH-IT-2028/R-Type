#include <gtest/gtest.h>
#include "Server.hpp"

using asio::ip::udp;

TEST(ServerTest, NewClientIsCreated) {
    asio::io_context io;
    server::Server s(io, 4242);

    udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 5000);
    auto idx = s.findOrCreateClient(ep);

    ASSERT_GE(idx, 0);
    auto client = s.getClient(idx);
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(client->_player_id, 0);
    EXPECT_TRUE(client->_connected);
}

TEST(ServerTest, ExistingClientIsReused) {
    asio::io_context io;
    server::Server s(io, 4242);

    udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 5000);
    auto idx1 = s.findOrCreateClient(ep);
    auto idx2 = s.findOrCreateClient(ep);

    EXPECT_EQ(idx1, idx2);
}

TEST(ServerTest, RejectsWhenFull) {
    asio::io_context io;
    server::Server s(io, 4242);

    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 5000 + i);
        EXPECT_GE(s.findOrCreateClient(ep), 0);
    }

    udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 6000);
    EXPECT_EQ(s.findOrCreateClient(ep), -1);
}

