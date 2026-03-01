# ✅ Checklist Técnico: Doom-Light

Transformação do DoomLike FPS → Jogo de Terror de Luzes ("Pique-esconde com a escuridão").

> [!IMPORTANT]
> Cada task é a **menor unidade atômica** possível. Siga na ordem exata para não quebrar o código funcional.

---

## 🗺️ Referência Rápida do Código Base

| Componente | Arquivo Principal | Papel |
|---|---|---|
| Map Parser | [maploader.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/maploader.cpp) | Lê [.txt](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/maps/map1.txt), detecta `9` (spawn) |
| Entity Spawn | [level.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/level.cpp) | Spawna Enemy/Item por char (`J,T,M,K,G,H,A`) |
| Entity Update | [entities.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp) | IA inimigos + pickup itens |
| Rendering | [drawlevel.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp) | Tiles + sprites de entidades |
| Lighting | [lighting.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/lighting.cpp) | GL_LIGHT0 (sol), GL_LIGHT1 (indoor), GL_LIGHT2 (lanterna) |
| Game Loop | [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) | Init / Update / Render |
| State | [game_state.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/game_state.h) | `GameContext`, `PlayerState`, `RenderAssets` |
| Weapon | [weapon.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/weapon.h) | Shotgun (reload/fire/anim) |
| HUD | [hud.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/hud.h) | Barra HP, munição, arma, overlays |
| Shaders | [blood.frag](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/shaders/blood.frag), [lava.frag](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/shaders/lava.frag) | GLSL 1.20 effects |

### Chars do Mapa Atual (map1.txt)
| Char | Significado | Ação Doom-Light |
|---|---|---|
| `1` | Parede outdoor | **Manter** |
| `2` | Parede indoor | **Manter** |
| `0` | Chão outdoor | **Manter** |
| `3` | Chão indoor | **Manter** |
| `9` | Spawn jogador | **Manter** |
| `L` | Lava | **Manter** (hazard ambiental) |
| `B` | Sangue | **Manter** (decorativo) |
| `J,T,M,K,G` | Inimigos tipo 0-4 | **Remover** |
| `H` | Health kit | **Manter** |
| `A` | Munição | **Remover** |
| `P` | Poste de Luz | **NOVO** ⭐ |

---

## 🛠️ Fase 1: Limpeza — Remover Combate

**Meta**: Jogo compila e roda sem armas/inimigos, mantendo movimentação e mapa intactos.

### 1.1 — Neutralizar Spawn de Inimigos

- [ ] **1.1.1** Em [level.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/level.cpp) L49-L75: Comentar/remover o bloco `if (enemyType != -1)` que faz `lvl.enemies.push_back(e)`.
- [ ] **1.1.2** Em [level.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/level.cpp) L48: Comentar/remover as linhas de detecção `if (c == 'J') enemyType = 0; ... if (c == 'K') enemyType = 4;`.
- [ ] **1.1.3** Compilar e confirmar: jogo abre sem crash, mapa renderiza sem sprites de inimigos.

### 1.2 — Neutralizar Spawn de Munição

- [ ] **1.2.1** Em [level.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/level.cpp) L89-L97: Comentar/remover o bloco `else if (c == 'A')` que cria `ITEM_AMMO`.
- [ ] **1.2.2** Compilar e confirmar: nenhum item de munição aparece no mapa.

### 1.3 — Desligar IA de Inimigos

- [ ] **1.3.1** Em [entities.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp) L34: No [updateEntities()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp#28-141), envolver o loop `for (auto& en : lvl.enemies)` num `if (!lvl.enemies.empty())` guard ou simplesmente comentar todo o bloco (linhas 34-117).
- [ ] **1.3.2** Compilar e confirmar: sem crashes por vetor vazio.

### 1.4 — Desligar Renderização de Inimigos/Itens de Munição

- [ ] **1.4.1** Em [drawlevel.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp) L524-L544: No [drawEntities()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp#495-549), comentar o bloco inteiro de `// --- INIMIGOS ---` (loop de enemies).
- [ ] **1.4.2** Em [drawlevel.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp) L520-L521: No [drawEntities()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp#495-549), dentro do loop de itens, comentar o `else if (item.type == ITEM_AMMO)` para não renderizar munição.
- [ ] **1.4.3** Compilar e testar: mapa renderiza limpo, sem sprites de inimigos nem munição.

### 1.5 — Desligar Arma do Jogador

- [ ] **1.5.1** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) L202: Comentar a chamada `updateWeaponAnim(dt)` em [gameUpdate()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#163-211).
- [ ] **1.5.2** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) L236: No [drawWorld3D()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#212-238), verificar se [drawEntities()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp#495-549) ainda funciona sem enemies (vetor vazio → OK, loop não executa).
- [ ] **1.5.3** Localizar onde `playerTryAttack()` é chamada (provavelmente em [input.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/input/input.cpp) ou no handler de input) e comentar a chamada.
- [ ] **1.5.4** Localizar onde `playerTryReload()` é chamada e comentar.
- [ ] **1.5.5** No HUD: em [gameRender()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#239-298) L290, mudar `hudRenderAll(... true, true, true)` para `hudRenderAll(... true, false, true)` — o `showWeapon=false` esconde a arma na tela.
- [ ] **1.5.6** Compilar e testar: HUD mostra HP mas não mostra arma, clicks não fazem nada.

### 1.6 — Limpar Ammo do PlayerState

- [ ] **1.6.1** Em [game_state.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/game_state.h) L12-L13: Comentar `currentAmmo` e `reserveAmmo` do `PlayerState` (ou manter mas ignorar).
  - **Alternativa segura**: Apenas ignorar na UI — não precisamos deletar campos por enquanto para evitar erros de compilação em cascata.
- [ ] **1.6.2** No HUD, zerar a exibição de munição: ajustar `hs.currentAmmo = 0; hs.reserveAmmo = 0;` em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) na montagem do `HudState`.

### 1.7 — Limpar Referências de Áudio de Inimigos

- [ ] **1.7.1** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) L143: Verificar se `audioInit(gAudioSys, gLevel)` crashia sem inimigos. Se sim, guardar os áudios de inimigos numa condição.
- [ ] **1.7.2** Em [entities.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp) L108: A chamada `audioPlayHurt(audio)` ficou inerte (bloco de ataque comentado). Confirmar.

### ✅ Marco Fase 1
> Jogo roda com mapa completo, jogador se move, lanterna funciona, mas **sem inimigos, armas ou munição**.

---

## 🛠️ Fase 2: Sistema de Postes de Luz (Dados + Visual)

**Meta**: Caractere `P` no mapa spawna postes de luz visíveis.

### 2.1 — Definir Struct LightPost

- [ ] **2.1.1** Criar novo header [include/core/lightpost.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/lightpost.h):
```cpp
#pragma once

struct LightPost {
    float x, z;       // Posição mundo (centro do tile)
    bool active;       // Se está aceso agora
    float intensity;   // 0.0 = apagado, 1.0 = máximo
};
```

### 2.2 — Adicionar `P` ao Mapa

- [ ] **2.2.1** Editar [maps/map1.txt](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/maps/map1.txt): Substituir alguns `0` ou `3` por `P` em posições estratégicas (ex: corredores, interseções). Colocar 4-6 postes pelo mapa.
- [ ] **2.2.2** Garantir que os `P` estão em posições caminháveis (onde antes era `0` ou `3`).

### 2.3 — Parser do `P` no Level Loader

- [ ] **2.3.1** Em [level.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/level/level.h): Adicionar `#include "core/lightpost.h"` e `std::vector<LightPost> posts;` na struct [Level](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/level/level.h#7-14).
- [ ] **2.3.2** Em [level.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/level.cpp): Adicionar `lvl.posts.clear();` junto com `enemies.clear()` e `items.clear()`.
- [ ] **2.3.3** Em [level.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/level/level.cpp): No loop de scan do mapa, adicionar:
```cpp
else if (c == 'P') {
    LightPost lp;
    lp.x = wx;
    lp.z = wz;
    lp.active = true;
    lp.intensity = 1.0f;
    lvl.posts.push_back(lp);
}
```
- [ ] **2.3.4** Compilar e testar com printf/breakpoint: `lvl.posts.size()` deve ser > 0.

### 2.4 — Tratar `P` como Chão Caminhável

- [ ] **2.4.1** Em [entities.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp) [isWalkable()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp#7-27) L22: Adicionar `if (c == 'P') return true;` (ou adaptar a lógica para que `P` não bloqueie).
- [ ] **2.4.2** Em [drawlevel.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp) L389-L391: Adicionar `'P'` à lista de `isEntity` para que o tile de chão seja desenhado sob o poste.
- [ ] **2.4.3** No [drawLevel()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/drawlevel.h#10-11) L389: A condição `isEntity` já trata esses tiles como chão. Adicionar `|| c == 'P'` nessa condição.
- [ ] **2.4.4** Compilar e testar: jogador pode caminhar em tiles `P`, chão renderiza.

### 2.5 — Desenhar Visual do Poste (Placeholder)

- [ ] **2.5.1** Em [drawlevel.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/drawlevel.h): Adicionar declaração: `void drawLightPosts(const std::vector<LightPost>& posts, float camX, float camZ, float dx, float dz);`
- [ ] **2.5.2** Em [drawlevel.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp): Implementar `drawLightPosts()`:
```cpp
void drawLightPosts(const std::vector<LightPost>& posts,
                    float camX, float camZ, float dx, float dz)
{
    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    for (const auto& p : posts)
    {
        if (!isVisibleXZ(p.x, p.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        glPushMatrix();
        glTranslatef(p.x, 0.0f, p.z);

        // Poste (pilar fino)
        glDisable(GL_TEXTURE_2D);
        if (p.active && p.intensity > 0.1f) {
            glColor3f(0.9f, 0.85f, 0.3f); // amarelado se aceso
        } else {
            glColor3f(0.2f, 0.2f, 0.2f);  // cinza se apagado
        }

        glPushMatrix();
        glTranslatef(0.0f, 1.5f, 0.0f);
        glScalef(0.15f, 3.0f, 0.15f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Lâmpada no topo
        if (p.active && p.intensity > 0.1f) {
            glColor3f(1.0f, 1.0f, 0.7f);
        } else {
            glColor3f(0.1f, 0.1f, 0.1f);
        }
        glPushMatrix();
        glTranslatef(0.0f, 3.2f, 0.0f);
        glutSolidSphere(0.25f, 8, 8);
        glPopMatrix();

        glEnable(GL_TEXTURE_2D);
        glColor3f(1, 1, 1);
        glPopMatrix();
    }
}
```
- [ ] **2.5.3** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [drawWorld3D()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#212-238): Adicionar `drawLightPosts(gLevel.posts, camX, camZ, dirX, dirZ);` após [drawLevel()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/drawlevel.h#10-11).
- [ ] **2.5.4** Incluir `"core/lightpost.h"` e `<vector>` onde necessário no [drawlevel.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/drawlevel.cpp).
- [ ] **2.5.5** Compilar e testar: postes visíveis como pilares no mapa.

### ✅ Marco Fase 2
> Postes de luz visíveis nos tiles `P`. Jogador caminha normalmente sobre eles.

---

## 🛠️ Fase 3: Ciclo de Luz e Máquina de Estados

**Meta**: Luzes dos postes ciclam entre ON → FLICKER → OFF com tempo, afetando iluminação real (GL_LIGHT).

### 3.1 — Definir a Máquina de Estados da Luz

- [ ] **3.1.1** Criar header [include/core/light_system.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/light_system.h):
```cpp
#pragma once
#include "core/lightpost.h"
#include <vector>

enum class LightCycleState {
    ON,       // Luzes acesas (safe)
    FLICKER,  // Piscando (aviso)
    OFF       // Apagadas (perigo!)
};

struct LightSystem {
    LightCycleState state = LightCycleState::ON;
    float timer = 0.0f;

    // Durações de cada fase (em segundos)
    float durationON      = 8.0f;
    float durationFLICKER = 3.0f;
    float durationOFF     = 5.0f;
};

void lightSystemUpdate(LightSystem& sys, std::vector<LightPost>& posts, float dt);
LightCycleState lightSystemGetState(const LightSystem& sys);
```

### 3.2 — Implementar o Update do Ciclo

- [ ] **3.2.1** Criar [src/core/light_system.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/light_system.cpp):
```cpp
#include "core/light_system.h"
#include <cmath>
#include <cstdlib>

void lightSystemUpdate(LightSystem& sys, std::vector<LightPost>& posts, float dt) {
    sys.timer += dt;

    switch (sys.state) {
    case LightCycleState::ON:
        // Todas acesas com intensity = 1.0
        for (auto& p : posts) { p.active = true; p.intensity = 1.0f; }

        if (sys.timer >= sys.durationON) {
            sys.state = LightCycleState::FLICKER;
            sys.timer = 0.0f;
        }
        break;

    case LightCycleState::FLICKER:
        // Pisca aleatoriamente
        for (auto& p : posts) {
            p.active = true;
            float flick = sinf(sys.timer * 12.0f + p.x * 3.7f + p.z * 2.3f);
            p.intensity = (flick > 0.0f) ? 0.8f : 0.15f;
        }

        if (sys.timer >= sys.durationFLICKER) {
            sys.state = LightCycleState::OFF;
            sys.timer = 0.0f;
        }
        break;

    case LightCycleState::OFF:
        // Todas apagadas
        for (auto& p : posts) { p.active = false; p.intensity = 0.0f; }

        if (sys.timer >= sys.durationOFF) {
            sys.state = LightCycleState::ON;
            sys.timer = 0.0f;
        }
        break;
    }
}

LightCycleState lightSystemGetState(const LightSystem& sys) {
    return sys.state;
}
```

### 3.3 — Registrar no CMake

- [ ] **3.3.1** Em [CMakeLists.txt](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/CMakeLists.txt): Adicionar `src/core/light_system.cpp` à lista de sources.

### 3.4 — Integrar no Game Loop

- [ ] **3.4.1** Em [game_state.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/game_state.h): Adicionar `#include "core/light_system.h"` e incluir `LightSystem lightSystem;` na struct `GameContext`.
- [ ] **3.4.2** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [gameUpdate()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#163-211): **Antes** de [updateEntities()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/entities.cpp#28-141), adicionar:
```cpp
lightSystemUpdate(g.lightSystem, gLevel.posts, dt);
```
- [ ] **3.4.3** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [gameReset()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#146-162): Adicionar reset:
```cpp
g.lightSystem.state = LightCycleState::ON;
g.lightSystem.timer = 0.0f;
```
- [ ] **3.4.4** Compilar e testar: observar no console/debug que `state` muda ON→FLICKER→OFF ciclicamente.

### 3.5 — Luzes Afetam Iluminação OpenGL

- [ ] **3.5.1** Em [lighting.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/lighting.h): Adicionar declaração:
```cpp
void setPostLightEachFrame(float postX, float postZ, float intensity, bool enabled);
```
- [ ] **3.5.2** Em [lighting.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/lighting.cpp): Implementar usando `GL_LIGHT3` (ou reutilizar `GL_LIGHT1`):
```cpp
void setPostLightEachFrame(float postX, float postZ, float intensity, bool enabled) {
    if (!enabled || intensity < 0.01f) {
        glDisable(GL_LIGHT3);
        return;
    }
    glEnable(GL_LIGHT3);
    GLfloat pos[] = { postX, 3.0f, postZ, 1.0f }; // posição no topo do poste
    GLfloat diff[] = { 1.5f * intensity, 1.4f * intensity, 0.8f * intensity, 1.0f };
    GLfloat amb[]  = { 0.3f * intensity, 0.28f * intensity, 0.15f * intensity, 1.0f };
    glLightfv(GL_LIGHT3, GL_POSITION, pos);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT3, GL_AMBIENT, amb);
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.15f);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.05f);
}
```
- [ ] **3.5.3** Em [lighting.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/lighting.cpp): Adicionar `setupPostLightOnce()` (setup do GL_LIGHT3 no init).
- [ ] **3.5.4** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [gameInit()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#70-145): Chamar `setupPostLightOnce()` após [setupFlashlightOnce()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/lighting.cpp#39-58).
- [ ] **3.5.5** Em [drawWorld3D()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#212-238) ou no [drawLevel()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/drawlevel.h#10-11): Para cada frame, encontrar o **poste ativo mais próximo** do jogador e chamar `setPostLightEachFrame(post.x, post.z, post.intensity, post.active)`.
  - **Nota**: OpenGL fixed pipeline suporta apenas ~8 luzes. Usar apenas o poste mais próximo em GL_LIGHT3 é suficiente.
- [ ] **3.5.6** Compilar e testar: quando luzes ON, área ao redor dos postes fica iluminada. Quando OFF, escuridão total (apenas lanterna).

### 3.6 — Escurecer Ambiente Global durante OFF

- [ ] **3.6.1** Em [drawWorld3D()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#212-238) em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp): Antes de [drawLevel()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/drawlevel.h#10-11), conforme o `LightCycleState`:
  - `ON`: Ambient global normal (`0.045f`).
  - `FLICKER`: Ambient reduzido (`0.02f`).
  - `OFF`: Ambient quase zero (`0.005f`) — escuridão quase total.
- [ ] **3.6.2** Implementar ajustando `glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ...)` com valores condicionais.
- [ ] **3.6.3** Compilar e testar: durante OFF o mundo fica **muito escuro**, somente a lanterna ilumina.

### ✅ Marco Fase 3
> Luzes ciclam ON→FLICKER→OFF. Mundo fica escuro durante OFF. Postes emitem luz real via OpenGL.

---

## 🛠️ Fase 4: Mecânica de Dano por Escuridão + Lanterna Limitada

**Meta**: Jogador toma dano contínuo quando longe de postes acesos. Lanterna tem bateria limitada.

### 4.1 — Calcular Distância ao Poste Seguro Mais Próximo

- [ ] **4.1.1** Em [light_system.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/light_system.h): Adicionar:
```cpp
float nearestActivePostDist(const std::vector<LightPost>& posts, float playerX, float playerZ);
bool playerIsInSafeZone(const std::vector<LightPost>& posts, float playerX, float playerZ, float safeRadius);
```
- [ ] **4.1.2** Em [light_system.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/light_system.cpp): Implementar:
```cpp
float nearestActivePostDist(const std::vector<LightPost>& posts, float px, float pz) {
    float minDist = 99999.0f;
    for (const auto& p : posts) {
        if (!p.active || p.intensity < 0.1f) continue;
        float dx = px - p.x, dz = pz - p.z;
        float d = sqrtf(dx*dx + dz*dz);
        if (d < minDist) minDist = d;
    }
    return minDist;
}

bool playerIsInSafeZone(const std::vector<LightPost>& posts, float px, float pz, float safeRadius) {
    return nearestActivePostDist(posts, px, pz) <= safeRadius;
}
```

### 4.2 — Dano Contínuo por Escuridão

- [ ] **4.2.1** Definir constantes de gameplay (novo header ou em [config.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/config.h)):
```cpp
const float SAFE_ZONE_RADIUS = 8.0f;   // raio do poste seguro (em unidades mundo)
const float DARKNESS_DPS = 5.0f;        // dano por segundo na escuridão
const float DARKNESS_DAMAGE_INTERVAL = 0.5f; // intervalo entre pulsos de dano
```
- [ ] **4.2.2** Em [game_state.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/game_state.h) `PlayerState`: Adicionar `float darknessDamageTimer = 0.0f;`.
- [ ] **4.2.3** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [gameUpdate()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#163-211): Após `lightSystemUpdate()`, adicionar lógica de dano:
```cpp
bool safe = playerIsInSafeZone(gLevel.posts, camX, camZ, SAFE_ZONE_RADIUS);
bool flashlightProtects = g.flashlightOn && g.player.batteryCharge > 0.0f;

if (!safe && !flashlightProtects) {
    g.player.darknessDamageTimer += dt;
    if (g.player.darknessDamageTimer >= DARKNESS_DAMAGE_INTERVAL) {
        g.player.health -= (int)(DARKNESS_DPS * DARKNESS_DAMAGE_INTERVAL);
        g.player.damageAlpha = 0.6f;
        g.player.darknessDamageTimer = 0.0f;
    }
} else {
    g.player.darknessDamageTimer = 0.0f;
}
```
- [ ] **4.2.4** Compilar e testar: ficar longe de um poste durante ON = safe. Ficar longe durante OFF = toma dano.

### 4.3 — Lanterna com Bateria Limitada

- [ ] **4.3.1** Em [game_state.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/game_state.h) `PlayerState`: Adicionar campos:
```cpp
float batteryCharge = 100.0f;      // 0-100
float batteryDrainRate = 8.0f;     // unidades/segundo quando ligada
float batteryRechargeRate = 3.0f;  // unidades/segundo quando desligada
```
- [ ] **4.3.2** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [gameUpdate()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#163-211): Adicionar lógica de bateria:
```cpp
if (g.flashlightOn) {
    g.player.batteryCharge -= g.player.batteryDrainRate * dt;
    if (g.player.batteryCharge <= 0.0f) {
        g.player.batteryCharge = 0.0f;
        g.flashlightOn = false; // Desliga automaticamente
    }
} else {
    g.player.batteryCharge += g.player.batteryRechargeRate * dt;
    if (g.player.batteryCharge > 100.0f) g.player.batteryCharge = 100.0f;
}
```
- [ ] **4.3.3** Em [gameToggleFlashlight()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#65-69): Só permitir ligar se `batteryCharge > 5.0f` (mínimo para ligar):
```cpp
void gameToggleFlashlight() {
    if (!g.flashlightOn && g.player.batteryCharge > 5.0f) {
        g.flashlightOn = true;
    } else if (g.flashlightOn) {
        g.flashlightOn = false;
    }
}
```
- [ ] **4.3.4** Em [gameReset()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#146-162): Resetar `g.player.batteryCharge = 100.0f;`.
- [ ] **4.3.5** Compilar e testar: lanterna gasta bateria, desliga ao zerar, recarrega quando desligada.

### 4.4 — Lanterna Protege contra Dano (breve)

- [ ] **4.4.1** A lógica de 4.2.3 já verifica `flashlightProtects`. Confirmar que `flashlightOn && batteryCharge > 0` impede dano.
- [ ] **4.4.2** Testar: ligar lanterna no escuro = sem dano. Bateria acaba = dano começa.

### ✅ Marco Fase 4
> Jogador toma dano contínuo no escuro. Lanterna protege mas gasta bateria. Postes acesos criam safe zones.

---

## 🛠️ Fase 5: HUD, Feedback Visual e Polimento

**Meta**: Jogador tem feedback claro do estado das luzes, bateria, e perigo.

### 5.1 — Barra de Bateria no HUD

- [ ] **5.1.1** Em [hud.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/graphics/hud.h) `HudState`: Adicionar `float batteryPercent = 100.0f;`.
- [ ] **5.1.2** Em [game.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp) [gameRender()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#239-298): Na montagem do `HudState`, adicionar `hs.batteryPercent = g.player.batteryCharge;`.
- [ ] **5.1.3** Localizar o [.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/main.cpp) do HUD ([src/graphics/hud.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/graphics/hud.cpp)) e em `hudRenderAll()`: Adicionar desenho de barra de bateria (retângulo GL que diminui):
```cpp
// Barra de bateria (canto inferior direito, acima da HP)
float batteryW = 100.0f * (state.batteryPercent / 100.0f);
glColor3f(0.2f, 0.8f, 1.0f); // azul
glBegin(GL_QUADS);
// ... posicionar no canto
glEnd();
```
- [ ] **5.1.4** Compilar e testar: barra azul diminui conforme bateria gasta.

### 5.2 — Indicador de Estado das Luzes

- [ ] **5.2.1** Em `HudState`: Adicionar `int lightCycleState = 0;` (0=ON, 1=FLICKER, 2=OFF).
- [ ] **5.2.2** Em [gameRender()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#239-298): Setar `hs.lightCycleState = (int)g.lightSystem.state;`.
- [ ] **5.2.3** No HUD: Desenhar ícone/texto indicando estado:
  - ON → "🟢 SAFE" (verde)
  - FLICKER → "🟡 WARNING" (amarelo, piscando)
  - OFF → "🔴 DANGER" (vermelho)
- [ ] **5.2.4** Compilar e testar: indicador muda conforme ciclo.

### 5.3 — Overlay de Escuridão (Vinheta)

- [ ] **5.3.1** Quando `LightCycleState::OFF` **e** jogador fora de safe zone: Renderizar um quad fullscreen semitransparente preto com alpha progressivo (vinheta de terror).
- [ ] **5.3.2** Em [gameRender()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#239-298), estado JOGANDO: Após `hudRenderAll()`, se condição de perigo:
```cpp
if (g.lightSystem.state == LightCycleState::OFF) {
    float darkAlpha = 0.4f; // base darkness overlay
    // ... desenhar quad 2D semitransparente
}
```
- [ ] **5.3.3** Compilar e testar: tela fica progressivamente escura durante OFF.

### 5.4 — Som Ambiente de Horror (Opcional)

- [ ] **5.4.1** Adicionar som de heartbeat/tensão quando `LightCycleState::OFF`.
- [ ] **5.4.2** Em `audioUpdate()`: Checar o light state e tocar som ambiente de terror.
- [ ] **5.4.3** Parar heartbeat quando luzes voltam a ON.

### 5.5 — Tela de Vitória (Opcional)

- [ ] **5.5.1** Definir condição de vitória: ex. sobreviver N ciclos completos, ou chegar a um tile de saída `X`.
- [ ] **5.5.2** Em [game_enums.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/game_enums.h): Adicionar `VITORIA` ao enum `GameState`.
- [ ] **5.5.3** Em [gameUpdate()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#163-211): Checar condição de vitória e setar `g.state = GameState::VITORIA`.
- [ ] **5.5.4** Em [gameRender()](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/game.cpp#239-298): Renderizar tela de vitória similar ao GAME_OVER.

### ✅ Marco Fase 5
> HUD mostra bateria e estado das luzes. Feedback visual de perigo. Jogo completo e jogável!

---

## 🏗️ Ordem de Compilação / Arquivos Novos

| Arquivo | Tipo | Fase |
|---|---|---|
| [include/core/lightpost.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/lightpost.h) | `[NEW]` Header | Fase 2 |
| [include/core/light_system.h](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/include/core/light_system.h) | `[NEW]` Header | Fase 3 |
| [src/core/light_system.cpp](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/src/core/light_system.cpp) | `[NEW]` Source | Fase 3 |
| [CMakeLists.txt](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/CMakeLists.txt) | `[MODIFY]` | Fase 3 |
| [maps/map1.txt](file:///c:/Users/rayan/projeto-final-final-computacao-grafica/maps/map1.txt) | `[MODIFY]` | Fase 2 |

## Verificação

### Teste por Fase
Cada marco de fase deve ser validado com:
1. **Compilação limpa** sem warnings críticos (`cmake --build . --config Debug`)
2. **Execução** sem crash (jogo abre, mapa carrega)
3. **Gameplay** correspondente à meta da fase (descrita em cada marco)

### Teste Manual
- **Fase 1**: Abrir jogo, andar pelo mapa. Nenhum inimigo, nenhuma arma na tela, nenhuma munição.
- **Fase 2**: Verificar postes visíveis como "pilares" amarelos nos tiles P.
- **Fase 3**: Esperar ~8s (ON) → luzes piscam → apagam → voltam. Observar escuridão/iluminação.
- **Fase 4**: Ficar longe de postes durante OFF → HP diminui. Ligar lanterna → HP para de cair. Bateria zera → lanterna desliga.
- **Fase 5**: HUD mostra barra de bateria, indicador de estado.
