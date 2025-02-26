# ENetChat

Simple chat application using C++, ENet, and PDCurses (a multi-environment GUI library)

![banner](./docs/banner.png)

## Project Overview

* Leverages ENet as a simple, light-weight communication layer, built on top of UDP while also providing reliable, in-order communication
* Server-Client Architecture, allowing one user to act as the host/server while 16 others can join the session as clients
* Custom serialization & deserialization using simple byte stream to pack and unpack messages as they are transmitted
* Multi-threading to ensure transport layer and frouser interface are decoupled and non-blocking
* Custom UI used to collect user input and display messages as they are sent & received across the network

This project was largely intended as a learning experience, focused on lower-level networking concepts like serializing & deserializing data, managing a shared state between connected peers, and architecting a server-client framework with decoupled transport and front end layers (...also, learning a bit of C++ along the way :relaxed:).

## Project Setup

If not done already, the included project dependencies (e.g. ENet, PDCurses) need to be linked to the "Chat" project

### Example
Navigate to the project properties (ex: `Chat > Properties > Debug | x64`), then update:
- **VC++ Directories**
    - Include Directories (append the following): `$(SolutionDir)\Chat`
- **C/C++**
    - Additional Include Directories: `$(SolutionDir)\Dependencies\enet\include;$(SolutionDir)\Dependencies\PDCurses\include`
    - Additional #using Directories: `$(SolutionDir)\network`
- **Linker**
    - Additional Library Directories: `$(SolutionDir)\Dependencies\enet;$(SolutionDir)\Dependencies\PDCurses`
    - Additional Dependencies (append the following): `enet64.lib;ws2_32.lib;winmm.lib;pdcurses64.lib`
        - **NOTE**: use `enet.lib` and `pdcurses.lib` for Win32, `enet64.lib` and `pdcurses64.lib` for x64
