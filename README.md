# Luzes Apagadas

Um jogo de tiro em primeira pessoa estilo Doom, desenvolvido com OpenGL (pipeline fixo + GLSL 1.20), GLUT para gerenciamento de janela/entrada, e OpenAL para sistema de áudio.

## Demo

<video src="https://rawcdn.githack.com/Unidade/projeto-final-computacao-grafica-doomlike/521511542854ae876d3ba2f9cac869a6c9ef5eba/demo-projeto-compgrafica.mp4" controls width="100%"></video>

> 💡 **Nota:** Se o vídeo não carregar acima, [clique aqui para assistir](./demo-projeto-compgrafica.mp4)

---

## Arquitetura do Projeto

O projeto está organizado em módulos separados por responsabilidade:

```
projeto-doom-computacao-grafica/
├── main.cpp                 # Ponto de entrada
├── src/                     # Implementações
│   ├── audio/              # Sistema de áudio (OpenAL)
│   ├── core/               # Lógica do jogo, entidades, câmera, movimento
│   ├── graphics/           # Renderização, shaders, texturas, HUD, menu
│   ├── input/              # Entrada do usuário (teclado, mouse)
│   ├── level/              # Carregamento, validação e métricas de mapas
│   ├── utils/              # Utilitários e gerenciamento de assets
│   └── tools/              # Ferramentas auxiliares (validador de níveis)
├── include/                 # Headers organizados por módulo
│   ├── audio/, core/, graphics/, input/, level/, utils/
├── shaders/                 # Shaders GLSL (lava, sangue, postes, transições)
├── maps/                    # Arquivos de mapa (.txt)
├── assets/                  # Texturas, áudios e outros recursos
└── build/                   # Diretório de compilação
```

---

## Requisitos

### Ferramentas de Build
- CMake 3.16+
- Compatível com C++17 (g++, clang++, ou MSVC)

### Bibliotecas

| Biblioteca | Linux (Ubuntu/Debian) | Windows (MSYS2) | Descrição |
|------------|----------------------|-----------------|-----------|
| GLEW | `libglew-dev` | `mingw-w64-x86_64-glew` | Carregamento de extensões OpenGL |
| GLUT/freeglut | `freeglut3-dev` | `mingw-w64-x86_64-freeglut` | Gerenciamento de janela e entrada |
| OpenGL | `libgl1-mesa-dev` | incluído no MinGW | Renderização gráfica |
| OpenAL | `libopenal-dev` | `mingw-w64-x86_64-openal` | Sistema de áudio |

---

## Instalação

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

1. Instale [MSYS2](https://www.msys2.org/)
2. Abra o terminal MSYS2 UCRT64 e execute:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-glew mingw-w64-x86_64-freeglut mingw-w64-x86_64-openal
```

---

## Compilação

### Usando CMake (Recomendado)

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

### Usando VS Code

1. Abra o projeto no VS Code
2. Pressione `Ctrl+Shift+B` para compilar
3. Pressione `F5` para depurar/executar

### Makefile Legado (somente Linux)
```bash
make
make run
```

---

## Controles

| Tecla | Ação |
|-------|------|
| **W, A, S, D** | Movimentação |
| **Mouse** | Olhar ao redor |
| **F** | Alternar lanterna |
| **Alt + Enter** | Alternar tela cheia |
| **ESC** | Sair |

---

## Formato do Mapa

Mapas são definidos em arquivos `.txt` usando caracteres ASCII:

| Caractere | Descrição |
|-----------|-----------|
| `1` | Parede |
| `0` | Chão |
| `9` | Spawn do jogador |
| `L` | Lava (efeito shader) |
| `B` | Sangue (efeito shader) |
| `P` | Poste de luz |
| `V` | Inimigo tipo V |
| `Y` | Inimigo tipo Y |
| `G` | Inimigo tipo G |
| `H` | Item de vida |
| `D` | Portal para próximo nível |

### Exemplo de Mapa
```
1111111111
1000000001
10L0000B01
1000090001
1000000001
1111111111
```

---

## Shaders Disponíveis

- **lava.frag/vert**: Efeito de animação de lava
- **blood.frag/vert**: Efeito de animação de sangue
- **lightpost.frag/vert**: Efeito de postes de luz
- **battery_flash.frag/vert**: Efeito de lanterna
- **level_transition.frag/vert**: Efeito de transição entre níveis
- **melt.frag/vert**: Efeito de derretimento
