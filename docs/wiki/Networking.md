# Networking

This page explains the networking architecture and protocols used in the R-Type client-server communication.

## Overview

The R-Type project utilizes a **UDP-based client-server architecture** for real-time multiplayer gameplay. This choice prioritizes low-latency communication, which is crucial for fast-paced action games, over guaranteed delivery. Both the client and server implement their own mechanisms for sequencing and re-synchronization to handle UDP's unreliable nature.

## Core Components

### `BaseNetworkManager`

Located in `core/network/BaseNetworkManager.hpp`, this abstract base class provides the fundamental functionalities for network communication using `asio` (Boost.Asio or Standalone Asio). It sets up a UDP socket and defines the basic interface for sending and receiving data.

Key responsibilities:
*   Initialization of an `asio::io_context` and a `asio::ip::udp::socket`.
*   Provides virtual methods for `startReceive`, `send`, `run`, and `stop` that concrete network managers must implement.
*   Manages a receive buffer (`_recv_buffer`).

### `ClientNetworkManager`

Implemented in `core/network/ClientNetworkManager.hpp`, this class extends `BaseNetworkManager` and handles all client-side networking logic. It connects to a specific server and manages the sending of player actions and receiving of game state updates.

Key responsibilities:
*   Connecting to a remote server endpoint.
*   Sending client-specific packets (e.g., player movement, shoot events).
*   Receiving and processing server-sent packets.
*   Managing connection state (e.g., `isConnected`).

### `ServerNetworkManager`

Implemented in `core/network/ServerNetworkManager.hpp`, this class also extends `BaseNetworkManager` and manages all server-side networking logic. It listens for incoming client connections, registers clients, and broadcasts game state updates.

Key responsibilities:
*   Listening for incoming UDP packets from multiple clients.
*   Registering and unregistering clients, mapping client IDs to their network endpoints.
*   Sending specific packets to individual clients or broadcasting to all connected clients.
*   Scheduling event processing and connection timeouts.
*   Handling signals for graceful shutdown.

### `Packet.hpp`

This file (`core/network/Packet.hpp`) defines the structure of all network packets used in the R-Type protocol. It includes:

*   **`PacketType` Enum**: An enumeration of all possible packet types (e.g., `Message`, `Move`, `NewPlayer`, `EnemySpawn`, `PlayerShoot`). Each type is assigned a unique `uint8_t` value.
*   **`PacketHeader` Struct**: A common header for all packets, containing `PacketType type` and `uint32_t size` (total size of the packet including header).
*   **Specific Packet Structs**: Various `struct` definitions for different packet types (e.g., `MovePacket`, `NewPlayerPacket`, `PlayerInfoPacket`, `EnemySpawnPacket`, `ProjectileSpawnPacket`). These structs are `alignas(4)` to ensure consistent binary layout.
*   **Enums for Game Elements**: `EnemyType` and `ProjectileType` enums are also defined here to categorize different game elements.

## R-Type Network Protocol (RTNP)

The detailed specification of the R-Type Network Protocol (RTNP) can be found in `docs/server_protocol.md`. Key aspects include:

*   **UDP-based**: All communication occurs over UDP datagrams.
*   **Single Packet per Datagram**: Each UDP datagram contains exactly one packet.
*   **Packet Structure**: All packets begin with a `PacketHeader` followed by a type-specific payload.
*   **Unreliable Nature**: Due to UDP's unreliability, clients and servers implement their own sequencing and re-synchronization mechanisms using `sequence_number` fields in various packet types.
*   **Message Types**: A comprehensive list of client-to-server and server-to-client messages, detailing their purpose and payload fields.
*   **Data Types**: Specifies the data types used (e.g., `uint8_t`, `uint32_t`, `float`, `char[]` for strings) and their endianness/alignment.
*   **Message Semantics**: Guidelines for packet validation, sequence number usage, and heartbeat mechanisms.
*   **No Authentication/Encryption**: The current protocol does not include authentication or encryption and is intended for controlled environments. Future work may address these security considerations.

## Communication Flow Example

1.  **Client Connection**: A client sends a `PlayerInfo` packet to the server to identify itself.
2.  **Server Acknowledgment**: The server responds with a `NewPlayer` packet to the client and broadcasts it to other connected clients.
3.  **Gameplay Loop**: Both client and server continuously exchange movement, projectile, and event packets to synchronize game state.
4.  **Keep-Alive**: `Heartbeat` packets are exchanged periodically to maintain the connection state and prevent timeouts.