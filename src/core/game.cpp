// #include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION,    0.0f);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.0f);
    glDisable(GL_LIGHT3);

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
    for (int i = 0; i < 3; i++) gHudTex.texKeyHud[i] = gAssets.texKey[i];

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.texHealth = gAssets.texHealth;
    g.r.texBattery = gAssets.texBattery;
    for (int i = 0; i < 3; i++) g.r.texKey[i] = gAssets.texKey[i];

    g.r.progSangue    = gAssets.progSangue;

    // Carrega o modelo 3D do inimigo avatar (GLB)
    if (!AvatarSystem::loadModel("assets/enemies/inimigo_fase.glb"))
    {
        printf("[Game] Warning: Could not load avatar enemy model. Avatar enemies will not be rendered.\n");
    }

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

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

    return true;
}

// Reinicia o jogo (volta ao Level 1)
void gameReset()
{
    g.player.health = 100;

    g.player.damageAlpha        = 0.0f;
    g.player.healthAlpha        = 0.0f;
    g.player.batteryCharge      = 100.0f;
    g.player.darknessDamageTimer= 0.0f;
    g.player.batteriesCollected = 0;
    for (int i = 0; i < 4; i++) g.player.hasLevelKey[i] = false;

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
    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;
    audioInit(gAudioSys, gLevel);
}

void gameUpdate(float dt)
{
    g.time += dt;

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
        if (g.player.batteryCharge > 100.0f) g.player.batteryCharge = 100.0f;
    }

    // --- DOOR EXIT CHECK (Luzes Apagadas: need all batteries to use elevator) ---
    static float doorLockedSoundCooldown = 0.0f;
    if (doorLockedSoundCooldown > 0.0f) doorLockedSoundCooldown -= dt;

    if (gLevel.hasDoor)
    {
        float ddx = camX - gLevel.doorX;
        float ddz = camZ - gLevel.doorZ;
        if (ddx * ddx + ddz * ddz < 4.0f) // within 2 units of door
        {
            bool hasBatteries = (g.player.batteriesCollected >= GameConfig::BATTERIES_REQUIRED);
            int cl = gLevel.currentLevel;
            bool hasKey = (cl >= 1 && cl <= 3 && g.player.hasLevelKey[cl]);

            if (!hasBatteries || !hasKey)
            {
                if (doorLockedSoundCooldown <= 0.0f)
                {
                    audioPlayPumpClick(gAudioSys);
                    doorLockedSoundCooldown = 2.0f;
                }
            }
            else if (hasBatteries && hasKey)
            {
            if (gLevel.currentLevel >= 3)
            {
                g.state = GameState::VITORIA; // Won the game!
            }
            else
            {
                // Load next level
                gLevel.currentLevel++;
                char mapPath[64];
                std::snprintf(mapPath, sizeof(mapPath), "maps/level%d.txt", gLevel.currentLevel);
                int savedLevel = gLevel.currentLevel;
                if (loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
                {
                    gLevel.currentLevel = savedLevel;
                    applySpawn(gLevel, camX, camZ);
                    camY = GameConfig::PLAYER_EYE_Y;
                    g.lightSystem.stateA = LightCycleState::ON;
                    g.lightSystem.stateB = LightCycleState::OFF;
                    g.lightSystem.timer = 0.0f;
                    g.lightSystem.cycleCount = 0;
                    g.levelTime = 0.0f; // reinicia tutorial no novo nivel
                    audioInit(gAudioSys, gLevel);
                }
            }
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
        const LightPost* best = nullptr;
        for (const auto& p : gLevel.posts) {
            if (!p.active || p.intensity < 0.05f) continue;
            float ddx = camX - p.x, ddz = camZ - p.z;
            float d = sqrtf(ddx*ddx + ddz*ddz);
            if (d < bestDist) { bestDist = d; best = &p; }
        }
        if (best)
            setPostLightEachFrame(best->x, best->z, best->intensity, best->active);
        else
            setPostLightEachFrame(0, 0, 0, false);
    }

    // Ambient: uniforme, visível sem light posts; safe posts acrescentam luz
    {
        LightCycleState ls = lightSystemGetState(g.lightSystem);
        if (ls == LightCycleState::ON) {
            GLfloat amb[] = {0.07f, 0.07f, 0.09f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        } else if (ls == LightCycleState::FLICKER) {
            GLfloat amb[] = {0.05f, 0.05f, 0.07f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        } else {
            GLfloat amb[] = {0.04f, 0.04f, 0.06f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        }
    }

    drawSkydome(camX, camY, camZ);
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time);
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
    hs.batteriesCollected = g.player.batteriesCollected;
    hs.batteriesRequired = GameConfig::BATTERIES_REQUIRED;
    int cl = gLevel.currentLevel;
    hs.currentLevel = cl;
    hs.hasLevelKey = (cl >= 1 && cl <= 3) && g.player.hasLevelKey[cl];
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.flashlightOn = g.flashlightOn;

    // --- ESTADO: MENU INICIAL ---
    if (g.state == GameState::MENU_INICIAL)
    {
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para Iniciar", g.r);
    }
    // --- ESTADO: GAME OVER ---
    else if (g.state == GameState::GAME_OVER)
    {
        menuRenderGameOver(janelaW, janelaH, g.time, g.r);
    }
    // --- ESTADO: VITORIA ---
    else if (g.state == GameState::VITORIA)
    {
        drawWorld3D();
        menuRender(janelaW, janelaH, g.time, "LUZES APAGADAS — VOCE ESCAPOU!", "Pressione ENTER para Jogar Novamente", g.r);
    }
    // --- ESTADO: PAUSADO ---
    else if (g.state == GameState::PAUSADO)
    {
        drawWorld3D();
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);
        pauseMenuRender(janelaW, janelaH, g.time, g.r);
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
