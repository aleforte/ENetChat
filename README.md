# ENetChat


## Project Setup

Under each project, Server & Client, include ENet & link to the library for each Configuration/Platform.

### Example
Navigate to the project properties (ex: `Client > Properties > Debug | x64`), then update:
- **C/C++**
    - Additional Include Directories: `..\libs\include`
- **Linker**
    - Additional Library Directories: `../libs`
    - Additional Dependencies (append the following): `enet64.lib;ws2_32.lib;winmm.lib`
        - **NOTE**: use `enet.lib` for Win32, `enet64.lib` for x64
