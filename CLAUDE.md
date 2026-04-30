# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

**NarutoClientv8** is a custom OTClientV8-based game client for a private Tibia-protocol server (`aerwix.com:7171`). It is a C++ + Lua hybrid: the engine is C++, all game UI and logic run as Lua modules loaded at startup via `init.lua`.

## Building

Uses CMake with vcpkg. Presets are defined in `CMakePresets.json`.

**Windows (Release):**
```sh
cmake --preset windows-release
cmake --build build/windows-release
```

**Windows (ASAN/debug):**
```sh
cmake --preset windows-asan
cmake --build build/windows-asan
```

**Linux (Release/Debug):**
```sh
cmake --preset linux-release   # or linux-debug
cmake --build build/linux-release
```

Requires vcpkg installed and `VCPKG_ROOT` set. The build uses Ninja and optionally sccache/ccache. Output binary is `otclient_dx.exe` (Windows) or `otclient` (Linux).

There is no automated test suite — testing is done by running the client and exercising features.

## Architecture

### Two-Layer Design

```
C++ (src/)            — engine, rendering, networking, Lua VM
Lua (modules/)        — all game logic, UI, and features
```

The C++ side exposes its API to Lua. Everything a player sees is a Lua module.

### C++ Layers (`src/`)

- **`src/framework/`** — platform-agnostic engine: graphics (OpenGL/DirectX), UI widget system, networking (asio), LuaJIT VM, HTTP, audio (OpenAL), proxy
- **`src/client/`** — Tibia-protocol specifics: map, creatures, items, sprites, and the two large protocol files (`protocolgameparse.cpp` ~3600 lines, `protocolgamesend.cpp`)
- **`src/main.cpp`** — bootstraps the framework, then runs `/init.lua`

### Lua Module System (`modules/`)

`init.lua` discovers and loads modules by **priority number** (defined in each `.otmod` manifest):

| Range | Purpose |
|-------|---------|
| 0–99 | Core libraries (`corelib`, `gamelib`) |
| 100–499 | Client UI and menus |
| 500–999 | In-game features |
| 1000–9999 | User mods |

Each module is a directory containing:
- `<name>.otmod` — manifest declaring `name`, `description`, `@onLoad` script, and `dependencies`
- `.lua` files — the module code
- `.otui` files — widget layout definitions (OTML syntax, CSS-like)

Key modules:
- `corelib` — base utilities, JSON, HTTP, keyboard, UI classes; **cannot be reloaded**
- `gamelib` — game-layer abstractions over the C++ client API
- `game_interface` — the main game window and HUD
- `client_entergame` — login/character selection screen
- `game_bot` — automation module
- `updater` — auto-update system

### Asset Pipeline (`data/`, `layouts/`)

- `data/styles/` — global OTML style sheets
- `data/shaders/` — GLSL shaders
- `layouts/mobile/` and `layouts/retro/` — theme overrides that shadow files in `data/`
- Sprites are loaded from `.spr`/`.dat` files in `data/things/`

## Key Development Patterns

### Adding a new feature
Create a new module directory under `modules/`. Give it a `.otmod` with a priority in the 500–999 range and list any dependencies. All game API calls go through the C++ bindings exposed to Lua (see `src/framework/luaengine/` and `src/client/luafunctions.cpp`).

### OTML (OTClient Markup Language)
Used for both config files (`data/config.otml`) and UI layouts (`.otui`). Syntax is indentation-based, similar to YAML with CSS-like property names. Widget trees are defined in `.otui`; styling rules in `.otmod`'s `@onLoad` or separate style sheets.

### Server / connection config
Server address, updater URL, and crash-reporter URL are set at the top of `init.lua`.

### C++ ↔ Lua boundary
New C++ functionality exposed to Lua must be registered in the appropriate `luafunctions.cpp` or `luabindings.cpp` file in `src/client/` or `src/framework/`. Follow the existing `registerClass` / `registerMethod` pattern used throughout those files.
