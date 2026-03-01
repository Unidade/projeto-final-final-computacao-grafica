# DoomLike OpenGL Project

A Doom-like first-person shooter game built with OpenGL (fixed pipeline + GLSL 1.20), GLUT for window/input management, and GLEW for modern OpenGL features.

## Demo
https://github.com/user-attachments/assets/be16fdec-675c-429a-895a-5aeb3071632c

---

## Requirements

### Build Tools
- CMake 3.16+
- C++17 compatible compiler (g++, clang++, or MSVC)

### Essential Libraries

| Library | Linux Package | Windows (MSYS2) | Description |
|---------|---------------|-----------------|-------------|
| GLEW | `libglew-dev` | `mingw-w64-x86_64-glew` | OpenGL extension loading |
| GLUT/freeglut | `freeglut3-dev` | `mingw-w64-x86_64-freeglut` | Window and input management |
| OpenGL | `libgl1-mesa-dev` | included in MinGW | Graphics rendering |
| OpenAL | `libopenal-dev` | `mingw-w64-x86_64-openal` | Audio system |

---

## Installation

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y cmake g++ libglew-dev freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev libopenal-dev
```

### Linux (Arch/Manjaro)

```bash
sudo pacman -S cmake gcc glew freeglut mesa openal
```

### Windows (MSYS2)

1. Install [MSYS2](https://www.msys2.org/)
2. Open MSYS2 UCRT64 terminal and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-freeglut mingw-w64-ucrt-x86_64-openal
```

---

## Building

### Using CMake (Recommended)

**Linux:**
```bash
cmake -B build -S .
cmake --build build
./build/DoomLike
```

**Windows (MinGW):**
```bash
cmake -B build -S . -G "MinGW Makefiles"
cmake --build build
./build/DoomLike.exe
```

### Using VS Code

1. Open the project in VS Code
2. Press `Ctrl+Shift+B` to build
3. Press `F5` to debug/run

### Legacy Makefile (Linux only)
```bash
make
make run
```

---

## Controls

| Key | Action |
|-----|--------|
| **W, A, S, D** | Move |
| **Mouse** | Look around |
| **F** | Toggle flashlight |
| **Alt + Enter** | Toggle fullscreen |
| **ESC** | Exit |

---

## Map Format

Maps are defined in `.txt` files using ASCII characters:

| Character | Description |
|-----------|-------------|
| `1` | Wall |
| `0` | Floor |
| `L` | Lava (shader effect) |
| `B` | Blood (shader effect) |
| `9` | Player spawn |
| `J, T, M, K, G` | Enemy spawn |
| `H` | Health item |
| `A` | Ammo item |

### Example Map
```
1111111111
1000000001
10L0000B01
1000090001
1000000001
1111111111
```
