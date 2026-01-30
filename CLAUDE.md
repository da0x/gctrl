# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GCtrl is a C++23 control systems IDE and programming language for control engineers. It provides visual diagram-driven development, automatic C++ code generation, and cross-platform deployment (Windows, Linux, embedded systems).

## Build Commands

### Linux

**Prerequisites:**
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
./build/engine/gctrl [path_to_project.ctrl]
```

### Windows (Visual Studio)

**Prerequisites:**
- CMake must be installed and available in PATH (the pre-build step uses it)

**Build:**
1. Open `gctrl.sln` in Visual Studio
2. Build > Build Solution (F7)

The pre-build event automatically generates `engine/generated/templates.hpp` using CMake.

**Run:**
```cmd
x64\Release\gctrl.exe [path_to_project.ctrl]
```

## Architecture

### Control System Hierarchy

The core domain model follows this hierarchy (top to bottom):

1. **Network** (`engine/controls/network/`) - Top-level container holding multiple machines
2. **Machine** (`engine/controls/machine/`) - System containing controllers and drivers
3. **Controller** (`engine/controls/controller/`) - Subsystem containing element instances and connections
4. **Element** (`engine/controls/element/`) - Reusable computational unit with inputs, outputs, memory, and code blocks
5. **Function** (`engine/controls/function/`) - Pure computational functions (no state)
6. **Port** (`engine/controls/port/`) - Interface connectors (sockets=input, plugs=output)
7. **Signal** (`engine/controls/signal/`) - Data type definitions with C++ type mappings
8. **Driver** (`engine/controls/driver/`) - Hardware interface components (UDP, Config)

### Runtime System

Per-machine runtime state tracking in `engine/controls/runtime/`:
- **state.hpp** - Runtime status enum (stopped, building, running, error) and tracker
- **build.hpp** - Build/run/stop functions for machines

GCtrl auto-detects UDP ports from machine drivers for debug communication:
- Listens on machine's `send_port` (receives what machine sends)
- Sends to machine's `listen_port` (where machine listens)

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

## Programming Guide

### Naming Conventions

- Use short, single-word names when possible
- No abbreviations unless widely known (e.g., `id`, `ui`, `config`)
- Getters and setters share the same name:
  ```cpp
  float value();        // getter
  void value(float v);  // setter
  ```

### Code Style

- Avoid lambda functions; use named functions or function objects instead
- Prefer references over pointers; use pointers only when null state is needed

### UI Layer (`engine/ui/`)

- Create inline wrappers in the `ui` namespace for ImGui functions
- Application code uses `std::string`, UI wrappers handle conversion to `const char*`
- Icons are centralized in `ui::icon::` namespace (`engine/ui/icons.hpp`)
  ```cpp
  // Icons only (single glyph)
  ui::icon::trash
  ui::icon::play

  // Button labels (icon + text, ready to use)
  ui::icon::remove   // "🗑 Delete"
  ui::icon::run      // "▶ Run"
  ```

### Strings

- Use `std::string` throughout application code
- UI layer handles conversion to C-strings internally
- String concatenation happens in application code, not at call sites

### Libraries (`lib/`)

Open source libraries included in the project are open for extension and bug fixes when it simplifies application code and keeps it clean.

## Contributing

Contributors must sign the CLA. Use feature branches and pull requests. License: GPL v3.0.
