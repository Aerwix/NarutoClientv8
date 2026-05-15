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

**Windows (Release + ASAN/debug):**
```sh
cmake --preset windows-release-asan
cmake --build build/windows-release-asan
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

Important `.otmod` flags:
- `sandboxed: true` — module runs in its own isolated Lua environment (most game modules)
- `reloadable: false` — module cannot be hot-reloaded (e.g. `corelib`, `updater`)
- `autoload: true` — module loads automatically without being pulled in by a dependency
- `load-later:` — lists modules to load after this one finishes (used by `game_interface` to sequence all HUD modules)

Key modules:
- `corelib` — base utilities, JSON, HTTP, keyboard, UI classes; **cannot be reloaded**
- `gamelib` — game-layer abstractions over the C++ client API
- `game_interface` — the main game window and HUD; orchestrates all other in-game modules via `load-later`
- `game_console` — chat/channel system
- `game_bot` — automation module (sandboxed executor with its own script runner)
- `game_topbar` — top HUD bar (Tibia 12-style)
- `game_healthinfo` — HP/mana bars and condition icons overlay
- `client_entergame` — login/character selection screen
- `updater` — auto-update system

### Asset Pipeline (`data/`, `layouts/`)

- `data/styles/` — global OTML style sheets
- `data/shaders/` — GLSL shaders
- `layouts/mobile/` and `layouts/retro/` — theme overrides that shadow files in `data/`
- Sprites are loaded from `.spr`/`.dat` files in `data/things/`
- Images referenced from Lua/OTUI use paths like `/images/game/...`

## Key Development Patterns

### Adding a new feature
Create a new module directory under `modules/`. Give it a `.otmod` with a priority in the 500–999 range and list any dependencies. All game API calls go through the C++ bindings exposed to Lua (see `src/framework/luaengine/` and `src/client/luafunctions_client.cpp`).

### Module lifecycle
Every module with a UI follows the same pattern: `init()` creates widgets and calls `connect()` to bind game events; `terminate()` calls `disconnect()` and destroys widgets. Widgets are loaded with `g_ui.loadUI('name', parentWidget)` or created with `g_ui.createWidget('StyleName', parent)`.

### Anchoring UI to the game panels
In-game panels attach to one of three anchors exposed by `game_interface`:
- `modules.game_interface.getRightPanel()` — right sidebar
- `modules.game_interface.getLeftPanel()` — left sidebar (bot window, etc.)
- `modules.game_interface.getMapPanel()` — over the game map (overlays)

### OTML (OTClient Markup Language)
Used for both config files (`data/config.otml`) and UI layouts (`.otui`). Syntax is indentation-based, similar to YAML with CSS-like property names. Widget trees are defined in `.otui`; styling rules in `.otmod`'s `@onLoad` or separate style sheets.

### Server / connection config
Server address, `APP_NAME`, `APP_VERSION`, and service URLs (updater, crash reporter, stats) are set in the `Services` and `Servers` tables at the top of `init.lua`. The updater only activates when `Services.updater` is non-empty and the client is running from a `.zip` archive.

### C++ ↔ Lua boundary
New C++ functionality exposed to Lua must be registered in the appropriate `luafunctions_client.cpp` or `luabindings.cpp` file in `src/client/` or `src/framework/`. Follow the existing `registerClass` / `registerMethod` pattern used throughout those files.
