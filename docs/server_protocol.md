# R-TYPE NETWORK PROTOCOL SPECIFICATION

*Internet-Draft — R-Type Project*  
*Intended status: Experimental*  
*November 2025*

---

## Abstract

This document specifies the binary **UDP-based** network protocol used by the **R-Type** game server and clients.  
The protocol defines message types, structure, and sequencing rules to support real-time multiplayer synchronization, combat events, room management, and game state management.

---

## Status of This Memo

This Internet-Draft is submitted for experimental purposes as part of the R-Type open-source project.  
It does not represent a standard of the IETF.  
Distribution of this memo is unlimited.

---

## Copyright Notice

Copyright © 2025 R-Type Project Authors.  
All rights reserved.

---

## Table of Contents

1. [Introduction](#1-introduction)  
2. [Terminology and Conventions](#2-terminology-and-conventions)  
3. [Protocol Overview](#3-protocol-overview)  
4. [Message Header](#4-message-header)  
5. [Message Types](#5-message-types)  
   - [Common Messages](#51-common-messages)  
   - [Client-to-Server Messages](#52-client-to-server-messages)  
   - [Server-to-Client Messages](#53-server-to-client-messages)  
   - [Room Management Messages](#54-room-management-messages)  
   - [Acknowledgment Messages](#55-acknowledgment-messages)  
6. [Data Types](#6-data-types)  
7. [Enumeration Types](#7-enumeration-types)  
8. [Message Semantics](#8-message-semantics)  
9. [Example Exchange](#9-example-exchange)  
10. [Security Considerations](#10-security-considerations)  
11. [Future Work](#11-future-work)  
Appendix A. [Packet Layouts](#appendix-a-packet-layouts)  
Author's Address  

---

## 1. Introduction

The **R-Type Network Protocol (RTNP)** defines how clients and the game server exchange structured **binary messages over UDP**.  
It supports synchronization of player state, enemies, projectiles, combat events, room management, and matchmaking in a real-time environment.

The protocol is designed for fast-paced multiplayer gameplay, using UDP for minimal latency.  
Each message is self-contained and includes its own header specifying type and size.

---

## 2. Terminology and Conventions

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHOULD**, and **MAY** in this document are to be interpreted as described in [RFC 2119](https://datatracker.ietf.org/doc/html/rfc2119).

---

## 3. Protocol Overview

Clients communicate with the game server using UDP datagrams.  
Each datagram contains exactly one packet consisting of a **PacketHeader** followed by a **payload** specific to the message type.

Since UDP is unreliable, clients and servers **MUST** implement their own sequencing and resynchronization mechanisms using the `sequence_number` field provided in several packet types. The protocol supports selective acknowledgment through ACK packets.

Typical communication flow:

1. The client sends a `PlayerInfo` packet to identify itself.
2. The client requests to create or join a room via `CreateRoom`, `JoinRoom`, or `MatchmakingRequest`.
3. The server responds with appropriate room response packets.
4. The server sends a `NewPlayer` packet for each player in the room.
5. Both sides exchange movement, projectile, and event packets continuously during gameplay.
6. A `Heartbeat` packet is exchanged periodically to maintain connection state.
7. Critical packets are acknowledged using `Ack` packets.

---

## 4. Message Header

All messages start with the following common header:
```
0 1 2 3 4 5 6 7 8
+-----------------------------------------------------------+
| Type (1 byte) | Size (4 bytes)                            |
+-----------------------------------------------------------+
```

**Fields:**

| Name | Type | Description |
|------|------|--------------|
| `Type` | `uint8_t` | Packet type identifier (see [Message Types](#5-message-types)) |
| `Size` | `uint32_t` | Total size of the packet in bytes, including the header |

All packets are **8-byte aligned** (`alignas(8)`).

---

## 5. Message Types

Each packet begins with a `PacketHeader`, followed by a type-specific payload.

### PacketType Enumeration

| Value | Name | Direction |
|--------|------|------------|
| `0x01` | ChatMessage | Both |
| `0x02` | PlayerMove | Server → Client |
| `0x03` | NewPlayer | Server → Client |
| `0x04` | PlayerInfo | Client → Server |
| `0x05` | EnemySpawn | Server → Client |
| `0x06` | EnemyMove | Server → Client |
| `0x07` | EnemyDeath | Server → Client |
| `0x08` | PlayerShoot | Client → Server |
| `0x09` | ProjectileSpawn | Server → Client |
| `0x0A` | ProjectileHit | Server → Client |
| `0x0B` | ProjectileDestroy | Server → Client |
| `0x0C` | GameStart | Server → Client |
| `0x0D` | GameEnd | Server → Client |
| `0x0E` | PlayerDisconnected | Both |
| `0x0F` | Heartbeat | Client → Server |
| `0x10` | EnemyHit | Server → Client |
| `0x11` | PlayerHit | Client → Server |
| `0x12` | PlayerDeath | Server → Client |
| `0x13` | CreateRoom | Client → Server |
| `0x14` | JoinRoom | Client → Server |
| `0x15` | LeaveRoom | Client → Server |
| `0x16` | ListRoom | Client → Server |
| `0x17` | ListRoomResponse | Server → Client |
| `0x18` | MatchmakingRequest | Client → Server |
| `0x19` | MatchmakingResponse | Server → Client |
| `0x1A` | JoinRoomResponse | Server → Client |
| `0x1B` | PlayerInput | Client → Server |
| `0x1C` | Ack | Both |
| `0x1D` | RequestChallenge | Client → Server |
| `0x1E` | ChallengeResponse | Server → Client |
| `0x1F` | CreateRoomResponse | Server → Client |

---

### 5.1. Common Messages

#### ChatMessage (0x01)
Sends a chat message with timestamp and color information.

| Field | Type | Description |
|--------|------|-------------|
| `timestamp` | `uint32_t` | Message timestamp |
| `message` | `char[512]` | UTF-8 message text (max 511 bytes) |
| `player_id` | `uint32_t` | Sender's player ID |
| `r`, `g`, `b`, `a` | `uint8_t` | RGBA color values (0-255) |
| `sequence_number` | `uint32_t` | Packet sequence number |

---

### 5.2. Client-to-Server Messages

#### PlayerInfo (0x04)
Provides the player's display name.

| Field | Type | Description |
|--------|------|-------------|
| `name` | `char[32]` | Null-terminated UTF-8 string (max 31 bytes) |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### PlayerShoot (0x08)
Notifies the server that the player fired a projectile.

| Field | Type | Description |
|--------|------|-------------|
| `x`, `y` | `float` | Projectile origin |
| `projectile_type` | `uint8_t` | Projectile type (e.g. PLAYER_BASIC) |
| `sequence_number` | `uint32_t` | Sequence number |

#### PlayerHit (0x11)
Reports that the player was hit.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Player identifier |
| `damage` | `uint32_t` | Damage amount |
| `x`, `y` | `float` | Hit position |
| `sequence_number` | `uint32_t` | Sequence number for ordering |

#### PlayerDisconnect (0x0E)
Sent when a client disconnects gracefully.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Identifier of the disconnecting player |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### Heartbeat (0x0F)
Keep-alive packet to maintain UDP connectivity.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Player identifier |

#### PlayerInput (0x1B)
Sends player movement input as a bitfield.

| Field | Type | Description |
|--------|------|-------------|
| `input` | `uint8_t` | Bitfield of MovementInputType flags |
| `sequence_number` | `uint32_t` | Client-side input sequence number |

---

### 5.3. Server-to-Client Messages

#### NewPlayer (0x03)
Announces a new player to connected clients.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Player ID |
| `player_name` | `char[32]` | Player display name (max 31 bytes) |
| `x`, `y` | `float` | Spawn position |
| `speed` | `float` | Movement speed |
| `sequence_number` | `uint32_t` | Packet sequence number |
| `max_health` | `uint32_t` | Max health points |

#### PlayerMove (0x02)
Updates a player's position.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Target player ID |
| `sequence_number` | `uint32_t` | Movement order number |
| `x`, `y` | `float` | New position |

#### EnemySpawn (0x05)
Spawns a new enemy entity.

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Unique enemy ID |
| `enemy_type` | `uint8_t` | Enemy type (e.g. BASIC_FIGHTER) |
| `x`, `y` | `float` | Spawn position |
| `velocity_x`, `velocity_y` | `float` | Movement velocity |
| `sequence_number` | `uint32_t` | Packet sequence number |
| `health`, `max_health` | `uint32_t` | Current and max health |

#### EnemyMove (0x06)
Updates enemy movement and position.

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Unique identifier of the enemy |
| `x`, `y` | `float` | Current enemy position |
| `velocity_x`, `velocity_y` | `float` | Current enemy velocity vector |
| `sequence_number` | `uint32_t` | Sequence number for ordering updates |

#### EnemyDeath (0x07)
Signals that an enemy was destroyed.

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Identifier of the enemy that died |
| `death_x`, `death_y` | `float` | World coordinates where the death occurred |
| `player_id` | `uint32_t` | Player who killed the enemy |
| `score` | `uint32_t` | Score awarded for the kill |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### EnemyHit (0x10)
Reports that an enemy took damage.

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Identifier of the enemy hit |
| `hit_x`, `hit_y` | `float` | Coordinates where the hit occurred |
| `damage` | `float` | Damage dealt to the enemy |
| `sequence_number` | `uint32_t` | Sequence number for event ordering |

#### ProjectileSpawn (0x09)
Creates a projectile in the client world.

| Field | Type | Description |
|--------|------|-------------|
| `projectile_id` | `uint32_t` | Unique identifier of the projectile |
| `projectile_type` | `uint8_t` | Projectile type (PLAYER_BASIC or ENEMY_BASIC) |
| `owner_id` | `uint32_t` | Entity ID of the projectile's owner |
| `is_enemy_projectile` | `uint8_t` | `1` if fired by an enemy, otherwise `0` |
| `x`, `y` | `float` | Initial position of the projectile |
| `velocity_x`, `velocity_y` | `float` | Direction and velocity vector |
| `speed` | `float` | Projectile speed scalar |
| `sequence_number` | `uint32_t` | Packet sequence number |
| `damage` | `uint32_t` | Damage caused on impact |

#### ProjectileHit (0x0A)
Notifies that a projectile hit an entity.

| Field | Type | Description |
|--------|------|-------------|
| `projectile_id` | `uint32_t` | Identifier of the projectile that hit |
| `target_id` | `uint32_t` | Entity ID of the impacted target |
| `target_is_player` | `uint8_t` | `1` if the target is a player, `0` if enemy |
| `hit_x`, `hit_y` | `float` | Coordinates of the hit point |

#### ProjectileDestroy (0x0B)
Removes a projectile from the world.

| Field | Type | Description |
|--------|------|-------------|
| `projectile_id` | `uint32_t` | Identifier of the projectile to remove |
| `x`, `y` | `float` | Position where the projectile was destroyed |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### PlayerDeath (0x12)
Informs clients that a player has died.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Identifier of the deceased player |
| `x`, `y` | `float` | Coordinates of the death location |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### GameStart (0x0C)
Signals the start of the game.

| Field | Type | Description |
|--------|------|-------------|
| `sequence_number` | `uint32_t` | Packet sequence number |
| `game_start` | `uint8_t` | `1` if the game has started, `0` otherwise |

#### GameEnd (0x0D)
Signals the end of the game.

| Field | Type | Description |
|--------|------|-------------|
| `sequence_number` | `uint32_t` | Packet sequence number |
| `game_end` | `uint8_t` | `1` if the game has ended, `0` otherwise |

---

### 5.4. Room Management Messages

#### CreateRoom (0x13)
Requests creation of a new game room.

| Field | Type | Description |
|--------|------|-------------|
| `room_name` | `char[32]` | Room display name (max 31 bytes) |
| `is_private` | `uint8_t` | `1` for private (password required), `0` for public |
| `password` | `char[32]` | Password for private rooms (max 31 bytes) |
| `max_players` | `uint8_t` | Maximum number of players allowed |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### CreateRoomResponse (0x1F)
Server response to a CreateRoom request.

| Field | Type | Description |
|--------|------|-------------|
| `error_code` | `uint8_t` | RoomError result code |
| `room_id` | `uint32_t` | Assigned room ID (valid when SUCCESS) |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### JoinRoom (0x14)
Requests to join an existing room.

| Field | Type | Description |
|--------|------|-------------|
| `room_id` | `uint32_t` | Target room identifier |
| `password` | `char[64]` | Room password or hash (max 63 bytes) |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### JoinRoomResponse (0x1A)
Server response to a JoinRoom request.

| Field | Type | Description |
|--------|------|-------------|
| `error_code` | `uint8_t` | RoomError result code |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### LeaveRoom (0x15)
Requests to leave the current room.

| Field | Type | Description |
|--------|------|-------------|
| `room_id` | `uint32_t` | Room identifier to leave |

#### ListRoom (0x16)
Requests a list of available rooms.

*No payload beyond header.*

#### ListRoomResponse (0x17)
Server response containing available rooms.

| Field | Type | Description |
|--------|------|-------------|
| `room_count` | `uint32_t` | Number of valid room entries |
| `rooms` | `RoomInfo[MAX_ROOMS]` | Array of room information structures |

**RoomInfo Structure:**

| Field | Type | Description |
|--------|------|-------------|
| `room_id` | `uint32_t` | Unique room identifier |
| `room_name` | `char[32]` | Room display name |
| `player_count` | `uint8_t` | Current number of players |
| `max_players` | `uint8_t` | Maximum allowed players |

#### MatchmakingRequest (0x18)
Requests automatic matchmaking into a suitable room.

| Field | Type | Description |
|--------|------|-------------|
| `sequence_number` | `uint32_t` | Packet sequence number |

#### MatchmakingResponse (0x19)
Server response to matchmaking request.

| Field | Type | Description |
|--------|------|-------------|
| `error_code` | `uint8_t` | RoomError result code |
| `sequence_number` | `uint32_t` | Packet sequence number |

---

### 5.5. Acknowledgment Messages

#### Ack (0x1C)
Acknowledges receipt of a sequenced packet.

| Field | Type | Description |
|--------|------|-------------|
| `sequence_number` | `uint32_t` | Sequence number being acknowledged |
| `player_id` | `uint32_t` | Player ID associated with the acknowledgment |

#### RequestChallenge (0x1D)
Requests a challenge string for password verification.

| Field | Type | Description |
|--------|------|-------------|
| `room_id` | `uint32_t` | Room for which challenge is requested |
| `sequence_number` | `uint32_t` | Packet sequence number |

#### ChallengeResponse (0x1E)
Server provides a challenge string and timestamp.

| Field | Type | Description |
|--------|------|-------------|
| `challenge` | `char[128]` | Challenge string for hashing (max 127 bytes) |
| `timestamp` | `uint32_t` | Challenge creation timestamp (epoch seconds) |
| `sequence_number` | `uint32_t` | Packet sequence number |

---

## 6. Data Types

| Type | Description |
|-------|-------------|
| `uint8_t`, `uint32_t` | Unsigned integers, little-endian |
| `float` | IEEE 754 single-precision, little-endian |
| `char[]` | UTF-8 encoded string, null-terminated |
| Alignment | All packets are 8-byte aligned (`alignas(8)`) |

**String Serialization Limits:**

| Field Size | Constant | Description |
|------------|----------|-------------|
| 32 bytes | `SERIALIZE_32_BYTES` | Short strings (names, etc.) |
| 64 bytes | `SERIALIZE_64_BYTES` | Passwords and hashes |
| 128 bytes | `SERIALIZE_128_BYTES` | Challenge strings |
| 512 bytes | `SERIALIZE_512_BYTES` | Chat messages |

---

## 7. Enumeration Types

### EnemyType

| Value | Name | Description |
|--------|------|-------------|
| `0x01` | BASIC_FIGHTER | Standard enemy type |

### ProjectileType

| Value | Name | Description |
|--------|------|-------------|
| `0x01` | PLAYER_BASIC | Standard player projectile |
| `0x02` | ENEMY_BASIC | Standard enemy projectile |

### RoomError

| Value | Name | Description |
|--------|------|-------------|
| `0x00` | SUCCESS | Operation succeeded |
| `0x01` | ROOM_NOT_FOUND | Requested room does not exist |
| `0x02` | ROOM_FULL | Room has reached maximum capacity |
| `0x03` | WRONG_PASSWORD | Invalid password provided |
| `0x04` | ALREADY_IN_ROOM | Player is already in a room |
| `0x05` | PLAYER_BANNED | Player is banned from the room |
| `0x06` | UNKNOWN_ERROR | Unspecified error occurred |

### MovementInputType (Bitfield)

| Value | Name | Description |
|--------|------|-------------|
| `1 << 0` | UP | Move up |
| `1 << 1` | DOWN | Move down |
| `1 << 2` | LEFT | Move left |
| `1 << 3` | RIGHT | Move right |

---

## 8. Message Semantics

### General Rules

- The server **MUST** validate the `size` field of all packets.  
- Clients **MUST NOT** send unknown or malformed packet types.  
- Sequence numbers **SHOULD** be used to detect dropped or out-of-order UDP packets.  
- The server **MAY** ignore packets received out of sequence.  
- The client **MUST** periodically send a `Heartbeat` packet to prevent timeout.

### Acknowledgment System

Critical packets requiring acknowledgment include:
- ChatMessage
- NewPlayer
- PlayerDisconnect
- EnemySpawn
- EnemyDeath
- ProjectileSpawn
- ProjectileDestroy
- GameStart
- PlayerShoot
- JoinRoomResponse
- MatchmakingResponse
- ChallengeResponse
- CreateRoomResponse

When a packet requires acknowledgment:
1. The sender stores the packet with its sequence number
2. The receiver sends an Ack packet upon successful processing
3. The sender removes the packet from unacknowledged storage upon receiving the Ack
4. Unacknowledged packets **MAY** be retransmitted after a timeout

### Room Password Security

For password-protected rooms:
1. Client requests challenge via `RequestChallenge`
2. Server responds with `ChallengeResponse` containing a random challenge string
3. Client hashes the password with the challenge: `hash(password + challenge + timestamp)`
4. Client sends the hash in the `JoinRoom` password field
5. Server validates by computing the same hash

---

## 9. Example Exchange

### Initial Connection and Room Join

**Client → Server:**
```
PlayerInfo (name="Alice", seq=1)
```

**Server → Client:**
```
Ack (seq=1, player_id=1)
```

**Client → Server:**
```
MatchmakingRequest (seq=2)
```

**Server → Client:**
```
Ack (seq=2, player_id=1)
MatchmakingResponse (error_code=SUCCESS, seq=100)
NewPlayer (id=1, name="Alice", x=100, y=200, seq=101)
```

**Client → Server:**
```
Ack (seq=100, player_id=1)
Ack (seq=101, player_id=1)
```

### Gameplay

**Client → Server:**
```
PlayerInput (input=UP|RIGHT, seq=3)
PlayerShoot (x=100, y=200, type=PLAYER_BASIC, seq=4)
Heartbeat (player_id=1)
```

**Server → Client:**
```
PlayerMove (id=1, x=105, y=195, seq=102)
ProjectileSpawn (id=99, owner=1, type=PLAYER_BASIC, seq=103)
EnemySpawn (id=42, type=BASIC_FIGHTER, x=500, y=100, seq=104)
```

**Client → Server:**
```
Ack (seq=103, player_id=1)
Ack (seq=104, player_id=1)
```

---

## 10. Security Considerations

The R-Type protocol provides **basic authentication through challenge-response** for room passwords but **no encryption**.  

**Known Vulnerabilities:**
- Packets can be intercepted and read in plaintext
- No protection against replay attacks beyond challenge timeout
- No integrity checks or packet signing
- Denial-of-service attacks possible via packet flooding

The protocol **SHOULD** be used only in controlled or local environments.  
Future revisions **MAY** introduce:
- TLS/DTLS for encryption
- HMAC for message authentication
- Rate limiting and flood protection
- Session tokens and player authentication

---

## 11. Future Work

- Add optional reliable UDP transport layer.  
- Support encryption for secure communication (DTLS).  
- Expand enemy and projectile type definitions.  
- Introduce session tokens and centralized player authentication.
- Add spectator mode support.
- Implement voice chat capability.
- Add replay recording and playback support.

---

## Appendix A. Packet Layouts

See `Packet.hpp` for complete C++ structure definitions.  
All packets are defined with `alignas(8)` to ensure consistent binary layout across architectures.

### Serialization

Packets are serialized using the Bitsery library with the following conventions:
- 1-byte values: `s.value1b(field)`
- 4-byte values: `s.value4b(field)`
- Floats: `s.template value<sizeof(float)>(field)`
- Strings: `s.text1b(field, SIZE_CONSTANT)`

All integer types use little-endian byte order.

---

## Authors

**Arthur GUERINAULT, Nolann DUBOS, Quentin LAPIERRE, Etienne LABARBE, Robin CHASSAGNE**  
R-Type Project  
November 2025
