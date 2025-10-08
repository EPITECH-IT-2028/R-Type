# R-TYPE NETWORK PROTOCOL SPECIFICATION

*Internet-Draft — R-Type Project*  
*Intended status: Experimental*  
*October 2025*

---

## Abstract

This document specifies the binary **UDP-based** network protocol used by the **R-Type** game server and clients.  
The protocol defines message types, structure, and sequencing rules to support real-time multiplayer synchronization, combat events, and game state management.

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
   - [Client-to-Server Messages](#51-client-to-server-messages)  
   - [Server-to-Client Messages](#52-server-to-client-messages)  
6. [Data Types](#6-data-types)  
7. [Message Semantics](#7-message-semantics)  
8. [Example Exchange](#8-example-exchange)  
9. [Security Considerations](#9-security-considerations)  
10. [Future Work](#10-future-work)  
Appendix A. [Packet Layouts](#appendix-a-packet-layouts)  
Author’s Address  

---

## 1. Introduction

The **R-Type Network Protocol (RTNP)** defines how clients and the game server exchange structured **binary messages over UDP**.  
It supports synchronization of player state, enemies, projectiles, and combat events in a real-time environment.

The protocol is designed for fast-paced multiplayer gameplay, using UDP for minimal latency.  
Each message is self-contained and includes its own header specifying type and size.

---

## 2. Terminology and Conventions

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHOULD**, and **MAY** in this document are to be interpreted as described in [RFC 2119](https://datatracker.ietf.org/doc/html/rfc2119).

---

## 3. Protocol Overview

Clients communicate with the game server using UDP datagrams.  
Each datagram contains exactly one packet consisting of a **PacketHeader** followed by a **payload** specific to the message type.

Since UDP is unreliable, clients and servers **MUST** implement their own sequencing and resynchronization mechanisms using the `sequence_number` field provided in several packet types.

Typical communication flow:

1. The client sends a `PlayerInfo` packet to identify itself.
2. The server responds with a `NewPlayer` packet.
3. Both sides exchange movement, projectile, and event packets continuously during gameplay.
4. A `Heartbeat` packet is exchanged periodically to maintain connection state.

---

## 4. Message Header

All messages start with the following common header:
```
0 1 2 3
+-----------------------------------------------------------+
| Type (1 byte) | Size (4 bytes) |
+-----------------------------------------------------------+
```

**Fields:**

| Name | Type | Description |
|------|------|--------------|
| `Type` | `uint8_t` | Packet type identifier (see [Message Types](#5-message-types)) |
| `Size` | `uint32_t` | Total size of the packet in bytes, including the header |

All packets are **4-byte aligned** (`alignas(4)`).

---

## 5. Message Types

Each packet begins with a `PacketHeader`, followed by a type-specific payload.

### PacketType Enumeration

| Value | Name | Direction |
|--------|------|------------|
| `0x01` | Message | Both |
| `0x02` | Move | Server → Client |
| `0x03` | NewPlayer | Server → Client |
| `0x04` | PlayerInfo | Client → Server |
| `0x05` | Position | Client → Server |
| `0x06` | EnemySpawn | Server → Client |
| `0x07` | EnemyMove | Server → Client |
| `0x08` | EnemyDeath | Server → Client |
| `0x09` | PlayerShoot | Client → Server |
| `0x0A` | ProjectileSpawn | Server → Client |
| `0x0B` | ProjectileHit | Server → Client |
| `0x0C` | ProjectileDestroy | Server → Client |
| `0x0D` | GameStart | Server → Client |
| `0x0E` | GameEnd | Server → Client |
| `0x0F` | PlayerDisconnected | Client → Server |
| `0x10` | Heartbeat | Both |
| `0x11` | EnemyHit | Server → Client |
| `0x12` | PlayerHit | Client → Server |
| `0x13` | PlayerDeath | Server → Client |

---

### 5.1. Client-to-Server Messages

#### PlayerInfo (0x04)
Provides the player’s display name.

| Field | Type | Description |
|--------|------|-------------|
| `name` | `char[32]` | Null-terminated UTF-8 string (max 31 bytes) |

#### Position (0x05)
Reports player position.

| Field | Type | Description |
|--------|------|-------------|
| `sequence_number` | `uint32_t` | Packet sequence number |
| `x`, `y` | `float` | Player coordinates |

#### PlayerShoot (0x09)
Notifies the server that the player fired a projectile.

| Field | Type | Description |
|--------|------|-------------|
| `x`, `y` | `float` | Projectile origin |
| `projectile_type` | `uint8_t` | Projectile type (e.g. PLAYER_BASIC) |
| `sequence_number` | `uint32_t` | Sequence number |

#### PlayerHit (0x12)
Reports that the player was hit.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Player identifier |
| `damage` | `uint32_t` | Damage amount |
| `x`, `y` | `float` | Hit position |
| `sequence_number` | `int32_t` | Sequence number for ordering |

#### PlayerDisconnect (0x0F)
Sent when a client disconnects gracefully.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Identifier of the disconnecting player |

#### Heartbeat (0x10)
Keep-alive packet to maintain UDP connectivity.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Player identifier |

---

### 5.2. Server-to-Client Messages

#### NewPlayer (0x03)
Announces a new player to connected clients.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Player ID |
| `x`, `y` | `float` | Spawn position |
| `speed` | `float` | Movement speed |
| `max_health` | `uint32_t` | Max health points |

#### Move (0x02)
Updates a player's position.

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Target player ID |
| `sequence_number` | `uint32_t` | Movement order number |
| `x`, `y` | `float` | New position |

#### EnemySpawn (0x06)
Spawns a new enemy entity.

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Unique enemy ID |
| `enemy_type` | `uint8_t` | Enemy type (e.g. BASIC_FIGHTER) |
| `x`, `y` | `float` | Spawn position |
| `velocity_x`, `velocity_y` | `float` | Movement velocity |
| `health`, `max_health` | `uint32_t` | Current and max health |

#### EnemyMove (0x07)
Updates enemy movement and position.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Unique identifier of the enemy |
| `x`, `y` | `float` | Current enemy position |
| `velocity_x`, `velocity_y` | `float` | Current enemy velocity vector |
| `sequence_number` | `uint32_t` | Sequence number for ordering updates |

#### EnemyDeath (0x08)
Signals that an enemy was destroyed.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Identifier of the enemy that died |
| `death_x`, `death_y` | `float` | World coordinates where the death occurred |

#### EnemyHit (0x11)
Reports that an enemy took damage.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `enemy_id` | `uint32_t` | Identifier of the enemy hit |
| `hit_x`, `hit_y` | `float` | Coordinates where the hit occurred |
| `damage` | `float` | Damage dealt to the enemy |
| `sequence_number` | `int32_t` | Sequence number for event ordering |

#### ProjectileSpawn (0x0A)
Creates a projectile in the client world.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `projectile_id` | `uint32_t` | Unique identifier of the projectile |
| `projectile_type` | `uint8_t` | Projectile type (PLAYER_BASIC or ENEMY_BASIC) |
| `owner_id` | `uint32_t` | Entity ID of the projectile’s owner |
| `is_enemy_projectile` | `uint8_t` | `1` if fired by an enemy, otherwise `0` |
| `x`, `y` | `float` | Initial position of the projectile |
| `velocity_x`, `velocity_y` | `float` | Direction and velocity vector |
| `speed` | `float` | Projectile speed scalar |
| `damage` | `uint32_t` | Damage caused on impact |

#### ProjectileHit (0x0B)
Notifies that a projectile hit an entity.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `projectile_id` | `uint32_t` | Identifier of the projectile that hit |
| `target_id` | `uint32_t` | Entity ID of the impacted target |
| `target_is_player` | `uint8_t` | `1` if the target is a player, `0` if enemy |
| `hit_x`, `hit_y` | `float` | Coordinates of the hit point |

#### ProjectileDestroy (0x0C)
Removes a projectile from the world.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `projectile_id` | `uint32_t` | Identifier of the projectile to remove |
| `x`, `y` | `float` | Position where the projectile was destroyed |

#### PlayerDeath (0x13)
Informs clients that a player has died.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `player_id` | `uint32_t` | Identifier of the deceased player |
| `x`, `y` | `float` | Coordinates of the death location |

#### GameStart (0x0D)
Signals the start of the game.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `game_start` | `uint8_t` | `1` if the game has started, `0` otherwise |

#### GameEnd (0x0E)
Signals the end of the game.

**Request Fields:**

| Field | Type | Description |
|--------|------|-------------|
| `game_end` | `uint8_t` | `1` if the game has ended, `0` otherwise |


---

## 6. Data Types

| Type | Description |
|-------|-------------|
| `uint8_t`, `uint32_t` | Unsigned integers, little-endian |
| `float` | IEEE 754 single-precision |
| `char[]` | UTF-8 encoded string, null-terminated |
| Alignment | All packets are 4-byte aligned (`alignas(4)`) |

---

## 7. Message Semantics

- The server **MUST** validate the `size` field of all packets.  
- Clients **MUST NOT** send unknown or malformed packet types.  
- Sequence numbers **SHOULD** be used to detect dropped or out-of-order UDP packets.  
- The server **MAY** ignore packets received out of sequence.  
- The client **MUST** periodically send a `Heartbeat` packet to prevent timeout.  

---

## 8. Example Exchange

**Client → Server:**
```
PlayerInfo ("Alice")
Position (x=100, y=200)
PlayerShoot (projectile_type=1)
```

**Server → Client:**
```
NewPlayer (id=1, x=100, y=200)
EnemySpawn (id=42, BASIC_FIGHTER, x=500, y=100)
ProjectileSpawn (id=99, owner=1)
```

---

## 9. Security Considerations

The R-Type protocol provides **no authentication or encryption**.  
It **SHOULD** be used only in controlled or local environments.  
Future revisions **MAY** introduce integrity checks or secure channels.

---

## 10. Future Work

- Add optional reliable UDP transport.  
- Support encryption for secure communication.  
- Expand enemy and projectile type definitions.  
- Introduce session tokens and player authentication.

---

## Appendix A. Packet Layouts

See `Packets.hpp` for C++ structure definitions.  
All packets are defined with `alignas(4)` to ensure consistent binary layout across architectures.

---

## Authors

**Arthur GUERINAULT Nolann DUBOS Quentin LAPIERRE Etienne LABARBE Robin CHASSAGNE**  
R-Type Project  
