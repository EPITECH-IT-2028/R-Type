# R-Type Project

Welcome to the R-Type project wiki! This document provides an overview of the project, its architecture, and how to get started.

## Overview

This project is a re-implementation of the classic arcade game R-Type, built with a focus on a modular and extensible architecture using an Entity-Component-System (ECS) pattern. It features both a client and a server component, allowing for networked multiplayer gameplay.

## Features

*   **Client-Server Architecture**: Dedicated server for game logic and client for rendering and user interaction.
*   **Entity-Component-System (ECS)**: A flexible and powerful architecture for game entities and their behaviors.
*   **Networked Gameplay**: Supports multiple players connecting to a central server.
*   **Asset Management**: Efficient handling of game assets like sprites and backgrounds.
*   **Cross-platform**: Designed to be runnable on various operating systems.

## Project Structure

The project is organized into several key directories:

*   `client/`: Contains all client-side specific code, including rendering, input handling, and client-side networking.
*   `server/`: Contains all server-side specific code, including game logic, entity management, and server-side networking.
*   `core/`: Shared utilities and networking components used by both client and server.
*   `game_engine/`: The core ECS implementation, including components, systems, and managers.
*   `docs/`: Project documentation, including protocol specifications and server setup guides.

## Getting Started

Please refer to the [Getting Started](GettingStarted.md) page for detailed instructions on how to build and run the project.

## Contributing

We welcome contributions! Please see the [Contribution Guidelines](ContributionGuidelines.md) for more information.

## Technical Details

For more in-depth technical information, explore the following sections:

*   [Game Engine (ECS)](GameEngineECS.md)
*   [Networking](Networking.md)
*   [Asset Management](AssetManagement.md)
