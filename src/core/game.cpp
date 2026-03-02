// #include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cfloat>

#include "core/game_enums.h"
#include "core/game_state.h"

#include "core/game.h"

#include "level/level.h"
#include "core/light_system.h"

#include "core/camera.h"
#include "input/input.h"
#include "input/keystate.h"

#include "graphics/drawlevel.h"
#include "graphics/skybox.h"
#include "graphics/hud.h"
#include "graphics/menu.h"
#include "graphics/lighting.h"

#include "core/movement.h"
#include "core/player.h"
#include "core/entities.h"

#include "audio/audio_system.h"

#include "utils/assets.h"
#include "core/config.h"
#include "level/level_validation.h"

#include "core/window.h"
#include "core/avatarEnemy.h"

#include <GL/glew.h>
#include <GL/glut.h>

static HudTextures gHudTex;
static GameContext g;

constexpr int MAX_MAGAZINE = 12;

// --- Assets / Level ---
static GameAssets gAssets;
Level gLevel;
static AudioSystem gAudioSys;

GameContext &gameContext() { return g; }

AudioSystem &gameAudio() { return gAudioSys; }

Level &gameLevel() { return gLevel; }

static int countBatteriesInLevel(const Level &lvl)
{
    int count = 0;
    for (const auto &item : lvl.items)
    {
        if (item.type == ITEM_BATTERY)
            count++;
    }
    return count;
}

GameState gameGetState() { return g.state; }

void gameSetState(GameState s) { g.state = s; }

void gameTogglePause()
{
    if (g.state == GameState::JOGANDO)
        g.state = GameState::PAUSADO;
    else if (g.state == GameState::PAUSADO)
        g.state = GameState::JOGANDO;
}

void gameToggleFlashlight()
{
    // Só permite ligar se tiver bateria suficiente
    if (!g.flashlightOn && g.player.batteryCharge > 5.0f)
        g.flashlightOn = true;
    else if (g.flashlightOn)
        g.flashlightOn = false;
}

// --- INIT ---
bool gameInit(const char *mapPath)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.005f, 0.005f, 0.02f, 1.0f);

    setupSunLightOnce();
    setupIndoorLightOnce();
    setupFlashlightOnce();

    // Inicializa GL_LIGHT3 (postes de luz)
    glEnable(GL_LIGHT3);
    GLfloat zero4[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT3, GL_DIFFUSE, zero4);
    glLightfv(GL_LIGHT3, GL_AMBIENT, zero4);
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.0f);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.0f);
    glDisable(GL_LIGHT3);

    std::srand((unsigned)std::time(nullptr));

    if (!loadAssets(gAssets))
        return false;

    g.r.texChao = gAssets.texChao;
    g.r.texParede = gAssets.texParede;
    g.r.texSangue = gAssets.texSangue;
    g.r.texDoor = gAssets.texDoor;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;

    g.r.texMenuBG = gAssets.texMenuBG;
    g.r.texGameOver = gAssets.texGameOver;

    gHudTex.texLinternOn = gAssets.texLinternOn;
    gHudTex.texLinternOff = gAssets.texLinternOff;
    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;
    for (int i = 0; i < 3; i++)
        gHudTex.texKeyHud[i] = gAssets.texKey[i];
    gHudTex.texBattery0 = gAssets.texBattery0;
    gHudTex.texBattery25 = gAssets.texBattery25;
    gHudTex.texBattery50 = gAssets.texBattery50;
    gHudTex.texBattery75 = gAssets.texBattery75;
    gHudTex.texBattery100 = gAssets.texBattery100;
    gHudTex.progBatteryFlash = gAssets.progBatteryFlash;

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.texHealth = gAssets.texHealth;
    g.r.texBattery0 = gAssets.texBattery0;
    g.r.texBattery25 = gAssets.texBattery25;
    g.r.texBattery50 = gAssets.texBattery50;
    g.r.texBattery75 = gAssets.texBattery75;
    g.r.texBattery100 = gAssets.texBattery100;
    for (int i = 0; i < 3; i++)
        g.r.texKey[i] = gAssets.texKey[i];

    g.r.progSangue = gAssets.progSangue;
    g.r.progTransition = gAssets.progTransition;

    // Carrega os modelos 3D dos inimigos avatar (GLB) por tipo
    bool okBasic = AvatarSystem::loadModelForType(EnemyType::BASIC, "assets/enemies/inimigo_fase.glb");
    bool okStalker = AvatarSystem::loadModelForType(EnemyType::STALKER, "assets/enemies/inimigo_fase_2.glb");
    bool okBoss = AvatarSystem::loadModelForType(EnemyType::BOSS, "assets/enemies/inimigo_fase_3.glb");
    if (!okBasic)
    {
        printf("[Game] Warning: Could not load basic avatar enemy model.\n");
    }
    if (!okStalker)
    {
        printf("[Game] Warning: Could not load stalker avatar enemy model.\n");
    }
    if (!okBoss)
    {
        printf("[Game] Warning: Could not load boss avatar enemy model.\n");
    }
    if (!okBasic && !okStalker && !okBoss)
    {
        printf("[Game] Warning: No avatar enemy models loaded. Avatar enemies will not be rendered.\n");
    }

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

    // Valida alcancabilidade de todos os elementos do nivel (dev)
    validateLevel(gLevel, 1);

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);

    // Audio init + ambient + enemy sources
    audioInit(gAudioSys, gLevel);

    g.state = GameState::MENU_INICIAL;
    g.time = 0.0f;
    g.levelTime = 0.0f;
    g.player = PlayerState{};
    g.weapon = WeaponAnim{};
    g.flashlightOn = true;
    gLevel.batteriesCollectedInMap = 0;

    return true;
}

// Reinicia o jogo (volta ao Level 1)
void gameReset()
{
    g.player.health = 100;

    g.player.damageAlpha = 0.0f;
    g.player.healthAlpha = 0.0f;
    g.player.batteryCharge = 100.0f;
    g.player.darknessDamageTimer = 0.0f;
    g.player.batteriesCollected = 0;
    gLevel.batteriesCollectedInMap = 0;
    for (int i = 0; i < 4; i++)
        g.player.hasLevelKey[i] = false;

    g.weapon.state = WeaponState::W_IDLE;
    g.weapon.timer = 0.0f;
    g.flashlightOn = true;
    g.levelTime = 0.0f;

    g.lightSystem.stateA = LightCycleState::ON;
    g.lightSystem.stateB = LightCycleState::OFF;
    g.lightSystem.timer = 0.0f;
    g.lightSystem.cycleCount = 0;

    // Reload level 1
    loadLevel(gLevel, "maps/level1.txt", GameConfig::TILE_SIZE);
    gLevel.currentLevel = 1;
    validateLevel(gLevel, 1);
    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;
    audioInit(gAudioSys, gLevel);
}

void gameUpdate(float dt)
{
    g.time += dt;

    // Lógica de transição de fase
    if (g.state == GameState::LEVEL_TRANSITION)
    {
        g.transitionTimer += dt;
        if (g.transitionTimer >= 2.0f) // 2 segundos total ate carregar novo mapa
        {
            gLevel.currentLevel = g.nextLevelToLoad;
            char mapPath[64];
            std::snprintf(mapPath, sizeof(mapPath), "maps/level%d.txt", gLevel.currentLevel);
            int savedLevel = gLevel.currentLevel;
            
            if (loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
            {
                gLevel.currentLevel = savedLevel;
                applySpawn(gLevel, camX, camZ);
                camY = GameConfig::PLAYER_EYE_Y;
                validateLevel(gLevel, savedLevel);
                
                g.lightSystem.stateA = LightCycleState::ON;
                g.lightSystem.stateB = LightCycleState::OFF;
                g.lightSystem.timer = 0.0f;
                g.lightSystem.cycleCount = 0;
                g.levelTime = 0.0f;
                gLevel.batteriesCollectedInMap = 0;
                g.doorMessageTimer = 0.0f;
                g.doorMessageText  = nullptr;
                audioInit(gAudioSys, gLevel);
                
                g.state = GameState::JOGANDO;
            }
            else
            {
                g.state = GameState::MENU_INICIAL;
            }
        }
        return;
    }

    // 1. SE NÃO ESTIVER JOGANDO, NÃO RODA A LÓGICA DO JOGO
    if (g.state != GameState::JOGANDO)
    {
        return;
    }

    g.levelTime += dt; // avanca apenas enquanto JOGANDO

    atualizaMovimento();

    AudioListener L;
    L.pos = {camX, camY, camZ};
    {
        float ry = yaw * 3.14159f / 180.0f;
        float rp = pitch * 3.14159f / 180.0f;
        L.forward = {cosf(rp) * sinf(ry), sinf(rp), -cosf(rp) * cosf(ry)};
    }
    L.up = {0.0f, 1.0f, 0.0f};
    L.vel = {0.0f, 0.0f, 0.0f};

    bool moving = (keyW || keyA || keyS || keyD);
    audioUpdate(gAudioSys, gLevel, L, dt, moving, g.player.health);

    if (g.player.damageAlpha > 0.0f)
    {
        g.player.damageAlpha -= dt * 0.5f;
        if (g.player.damageAlpha < 0.0f)
            g.player.damageAlpha = 0.0f;
    }
    if (g.player.healthAlpha > 0.0f)
    {
        g.player.healthAlpha -= dt * 0.9f;
        if (g.player.healthAlpha < 0.0f)
            g.player.healthAlpha = 0.0f;
    }

    updateEntities(dt);
    updateWeaponAnim(dt);

    // --- DOOM-LIGHT: ciclo de luzes ---
    lightSystemUpdate(g.lightSystem, gLevel.posts, dt);

    // --- DOOM-LIGHT: bateria da lanterna ---
    if (g.flashlightOn)
    {
        g.player.batteryCharge -= g.player.batteryDrainRate * dt;
        if (g.player.batteryCharge <= 0.0f)
        {
            g.player.batteryCharge = 0.0f;
            g.flashlightOn = false; // apaga automaticamente
        }
    }
    else
    {
        g.player.batteryCharge += g.player.batteryRechargeRate * dt;
        if (g.player.batteryCharge > 100.0f)
            g.player.batteryCharge = 100.0f;
    }

    // --- DOOR EXIT CHECK ---
    // O jogador precisa apertar E perto da porta. Se os requisitos forem atendidos,
    // a porta abre e o nivel transiciona; caso contrario, exibe mensagem de feedback.

    if (g.doorMessageTimer > 0.0f)
        g.doorMessageTimer -= dt;
    else
        g.doorMessageText = nullptr;

    // Decai timer do flash de baterias
    if (g.allBatteriesFlashTimer > 0.0f)
        g.allBatteriesFlashTimer -= dt;

    if (gLevel.hasDoor && !gLevel.doorOpen)
    {
        float ddx = camX - gLevel.doorX;
        float ddz = camZ - gLevel.doorZ;
        // Raio de 6 unidades (1.5 tiles) — o jogador fica a ~4 unid do centro da porta
        bool nearDoor = (ddx * ddx + ddz * ddz < 36.0f);

        // Mostra dica "Pressione E" quando perto da porta
        if (nearDoor && g.doorMessageTimer <= 0.0f)
        {
            g.doorMessageText  = "Pressione E para interagir";
            g.doorMessageTimer = 0.1f; // refreshes each frame player stays close
        }

        if (nearDoor && keyE)
        {
            keyE = false; // consume o pressionamento para nao repetir

            bool hasBatteries = (gLevel.batteriesCollectedInMap >= gLevel.batteriesRequiredInMap);
            int cl = gLevel.currentLevel;
            bool hasKey = (cl >= 1 && cl <= 3 && g.player.hasLevelKey[cl]);

            if (!hasBatteries && !hasKey)
            {
                g.doorMessageText = "Colete as baterias e a chave!";
                g.doorMessageTimer = 3.0f;
                audioPlayPumpClick(gAudioSys);
            }
            else if (!hasBatteries)
            {
                g.doorMessageText = "Colete todas as baterias!";
                g.doorMessageTimer = 3.0f;
                audioPlayPumpClick(gAudioSys);
            }
            else if (!hasKey)
            {
                g.doorMessageText = "Voce precisa da chave!";
                g.doorMessageTimer = 3.0f;
                audioPlayPumpClick(gAudioSys);
            }
            else
            {
                // Requisitos atendidos: abre a porta
                gLevel.doorOpen = true;
            }
        }
    }

    // Quando a porta estiver aberta e o jogador passar por dentro, transiciona de nivel
    if (gLevel.hasDoor && gLevel.doorOpen)
    {
        float ddx = camX - gLevel.doorX;
        float ddz = camZ - gLevel.doorZ;
        if (ddx * ddx + ddz * ddz < 4.0f)
        {
            if (gLevel.currentLevel >= 3)
            {
                g.state = GameState::VITORIA;
            }
            else
            {
                g.state = GameState::LEVEL_TRANSITION;
                g.transitionTimer = 0.0f;
                g.nextLevelToLoad = gLevel.currentLevel + 1;
            }
        }
    }


    // 3. CHECAGEM DE GAME OVER
    if (g.player.health <= 0)
    {
        g.state = GameState::GAME_OVER;
        g.player.damageAlpha = 1.0f;
    }
}

// Função auxiliar para desenhar o mundo 3D (Inimigos, Mapa, Céu)
void drawWorld3D()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // LIGAR O 3D
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    // Configuração da Câmera
    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;
    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);
    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    // Desenha o cenário
    setSunDirectionEachFrame();
    setFlashlightEachFrame(camX, camY, camZ, dirX, dirY, dirZ, g.flashlightOn);

    // --- Luz do poste mais próximo ---
    {
        float bestDist = FLT_MAX;
        const LightPost *best = nullptr;
        for (const auto &p : gLevel.posts)
        {
            if (!p.active || p.intensity < 0.05f)
                continue;
            float ddx = camX - p.x, ddz = camZ - p.z;
            float d = sqrtf(ddx * ddx + ddz * ddz);
            if (d < bestDist)
            {
                bestDist = d;
                best = &p;
            }
        }
        if (best)
            setPostLightEachFrame(best->x, best->z, best->intensity, best->active);
        else
            setPostLightEachFrame(0, 0, 0, false);
    }

    // Ambient: uniforme, visível sem light posts; safe posts acrescentam luz
    {
        LightCycleState ls = lightSystemGetState(g.lightSystem);
        if (ls == LightCycleState::ON)
        {
            GLfloat amb[] = {0.07f, 0.07f, 0.09f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        }
        else if (ls == LightCycleState::FLICKER)
        {
            GLfloat amb[] = {0.05f, 0.05f, 0.07f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        }
        else
        {
            GLfloat amb[] = {0.04f, 0.04f, 0.06f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        }
    }

    drawSkydome(camX, camY, camZ);
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time, gLevel.doorOpen);
    drawEntities(gLevel.enemies, gLevel.items, camX, camZ, dirX, dirZ, g.r);
    drawLightPosts(gLevel.posts, camX, camZ, dirX, dirZ);
}

// FUNÇÃO PRINCIPAL DE DESENHO (REFATORADA: usa menuRender / pauseMenuRender / hudRenderAll)
void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Monta o estado do HUD a partir das variáveis globais do jogo
    HudState hs;
    hs.playerHealth = g.player.health;
    hs.batteriesCollected = gLevel.batteriesCollectedInMap;
    hs.batteriesRequired = gLevel.batteriesRequiredInMap;
    int cl = gLevel.currentLevel;
    hs.currentLevel = cl;
    hs.hasLevelKey = (cl >= 1 && cl <= 3) && g.player.hasLevelKey[cl];
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.flashlightOn = g.flashlightOn;
    // Mensagem de porta bloqueada (fade baseado no timer)
    hs.doorMessageAlpha = (g.doorMessageTimer > 0.0f) ? std::fmin(g.doorMessageTimer, 1.0f) : 0.0f;
    hs.doorMessage = (hs.doorMessageAlpha > 0.0f) ? g.doorMessageText : nullptr;
    // Flash de baterias completas
    hs.allBatteriesFlashAlpha = (g.allBatteriesFlashTimer > 0.0f)
        ? std::fmin(g.allBatteriesFlashTimer / 2.5f, 1.0f) : 0.0f;

    // --- ESTADO: MENU INICIAL ---
    if (g.state == GameState::MENU_INICIAL)
    {
        // A imagem de fundo já contém o texto de instrução;
        // aqui não desenhamos título nem subtítulo extras.
        menuRender(janelaW, janelaH, g.time, "", "", g.r);
    }
    // --- ESTADO: GAME OVER ---
    else if (g.state == GameState::GAME_OVER)
    {
        menuRenderGameOver(janelaW, janelaH, g.time, g.r);
    }
    // --- ESTADO: VITORIA ---
    else if (g.state == GameState::VITORIA)
    {
        // Garante que a música de vitória esteja tocando (e chase/ambient estejam silenciosos)
        {
            auto &a = gameAudio();
            if (a.ok && a.bufVictory && a.srcVictory)
            {
                if (!a.victoryPlaying)
                {
                    // Silencia música de chase e ambiente
                    if (a.srcChase) a.engine.setSourceGain(a.srcChase, 0.0f);
                    if (a.srcAmbient) a.engine.setSourceGain(a.srcAmbient, 0.0f);
                    a.engine.stop(a.srcChase);

                    a.engine.setSourceGain(a.srcVictory, AudioTuning::MASTER * AudioTuning::AMBIENT_GAIN);
                    a.engine.play(a.srcVictory);
                    a.victoryPlaying = true;
                }
            }
        }

        drawWorld3D();
        menuRender(janelaW, janelaH, g.time, "VOCE ESCAPOU!", "Pressione ENTER para Jogar Novamente", g.r);
    }
    // --- ESTADO: PAUSADO ---
    else if (g.state == GameState::PAUSADO)
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);
        pauseMenuRender(janelaW, janelaH, g.time, g.r);
    }
    // --- ESTADO: LEVEL TRANSITION ---
    else if (g.state == GameState::LEVEL_TRANSITION)
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, false, false, false);
        float progress = std::fmin(g.transitionTimer / 2.0f, 1.0f);
        levelTransitionRender(janelaW, janelaH, g.time, progress, g.nextLevelToLoad, g.r);
    }
    // --- ESTADO: JOGANDO ---
    else
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);
        desenhaTutorial(g.levelTime, janelaW, janelaH);
        menuMeltRenderOverlay(janelaW, janelaH, g.time);
    }

    glutSwapBuffers();
}
