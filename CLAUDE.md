# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GCtrl is a C++20 control systems IDE and programming language for control engineers. It provides visual diagram-driven development, automatic C++ code generation, and cross-platform deployment (Windows, Linux, embedded systems).

## Build Commands

**Prerequisites (Linux):**
```bash
sudo apt-get install libsdl2-dev libglew-dev libglm-dev libgl1-mesa-dev
```

**Build:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Run:**
```bash
./build/gctrl [path_to_project.ctrl]
```

## Architecture

### Control System Hierarchy

The core domain model follows this hierarchy (top to bottom):

1. **Machine** (`engine/controls/machine/`) - Top-level system containing multiple controllers
2. **Controller** (`engine/controls/controller/`) - Subsystem containing element instances and connections
3. **Element** (`engine/controls/element/`) - Reusable computational unit with inputs, outputs, memory, and code blocks
4. **Function** (`engine/controls/function/`) - Pure computational functions (no state)
5. **Port** (`engine/controls/port/`) - Interface connectors (sockets=input, plugs=output)
6. **Signal** (`engine/controls/signal/`) - Data type definitions with C++ type mappings
7. **Driver** (`engine/controls/driver/`) - Hardware interface components

### Key Patterns

- **Object/Instance Pattern**: `*_object` classes define specifications, `*_instance` classes reference them
- **JSON Serialization**: All domain objects implement `serialize()` using nlohmann/json
- **Record Base Class**: Common base providing UUID, metadata, and serialization
- **Header-Only Design**: Most code is in `.hpp` files

### Platform Abstraction

Platform-specific code in `engine/platform/` with conditional compilation:
```cpp
#if GCTRL_PLATFORM_WINDOWS
#elif GCTRL_PLATFORM_LINUX
#endif
```

### UI Layer

ImGui-based UI in `engine/ui/`:
- `ui.hpp` - Main UI orchestration
- `graph.hpp` - Visual node editor (using imgui-node-editor)
- `navigation.hpp` - Modal navigation system
- `chart.hpp` - Real-time data visualization (using ImPlot)

### Code Generation

`engine/controls/code/generator.hpp` transforms control system definitions into deployable C++ code.

## Project Files

Projects are stored as `.ctrl` files (JSON format) containing machines, controllers, elements, functions, signals, and UI settings. Recent projects tracked in `~/.gctrlrc.json`.

## Dependencies

- SDL2 - Windowing/input
- OpenGL/GLEW - Rendering
- ImGui, ImPlot, imgui-node-editor - UI (in `lib/`)
- nlohmann/json - Serialization (in `lib/`)

## Contributing

Contributors must sign the CLA. Use feature branches and pull requests. License: GPL v3.0.
