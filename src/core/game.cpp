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
    g.r.texLava = gAssets.texLava;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;

    g.r.texSkydome = gAssets.texSkydome;
    g.r.texMenuBG = gAssets.texMenuBG;

    gHudTex.texHudFundo = gAssets.texHudFundo;
    gHudTex.texGunHUD = gAssets.texGunHUD;

    gHudTex.texLinternOn = gAssets.texLinternOn;
    gHudTex.texLinternOff = gAssets.texLinternOff;
    gHudTex.texGunDefault = gAssets.texGunDefault;
    gHudTex.texGunFire1 = gAssets.texGunFire1;
    gHudTex.texGunFire2 = gAssets.texGunFire2;
    gHudTex.texGunReload1 = gAssets.texGunReload1;
    gHudTex.texGunReload2 = gAssets.texGunReload2;

    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.texHealth = gAssets.texHealth;
    g.r.texAmmo = gAssets.texAmmo;

    g.r.progSangue = gAssets.progSangue;
    g.r.progLava = gAssets.progLava;

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
    g.player = PlayerState{};
    g.weapon = WeaponAnim{};
    g.flashlightOn = true;

    return true;
}

// Reinicia o jogo
void gameReset()
{
    g.player.health = 100;
    g.player.currentAmmo = 12;
    g.player.reserveAmmo = 25;

    g.player.damageAlpha        = 0.0f;
    g.player.healthAlpha        = 0.0f;
    g.player.batteryCharge      = 100.0f;
    g.player.darknessDamageTimer= 0.0f;

    g.weapon.state = WeaponState::W_IDLE;
    g.weapon.timer = 0.0f;
    g.flashlightOn = true;

    g.lightSystem.state = LightCycleState::ON;
    g.lightSystem.timer = 0.0f;

    // Respawna o jogador
    applySpawn(gLevel, camX, camZ);
}

void gameUpdate(float dt)
{
    g.time += dt;

    // 1. SE NÃO ESTIVER JOGANDO, NÃO RODA A LÓGICA DO JOGO
    if (g.state != GameState::JOGANDO)
    {
        return;
    }

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

    // --- DOOM-LIGHT: dano por escuridão ---
    // Jogador é protegido se estiver perto de um poste aceso OU com lanterna ligada
    static const float SAFE_ZONE_RADIUS   = 8.0f;  // unidades mundo
    static const float DARKNESS_DPS       = 6.0f;  // dano por segundo
    static const float DAMAGE_INTERVAL    = 0.6f;  // pulso de dano (s)

    bool nearPost       = playerIsInSafeZone(gLevel.posts, camX, camZ, SAFE_ZONE_RADIUS);
    bool flashProtects  = g.flashlightOn && g.player.batteryCharge > 0.0f;

    if (!nearPost && !flashProtects)
    {
        g.player.darknessDamageTimer += dt;
        if (g.player.darknessDamageTimer >= DAMAGE_INTERVAL)
        {
            int dmg = (int)(DARKNESS_DPS * DAMAGE_INTERVAL);
            g.player.health     -= dmg;
            g.player.damageAlpha = 0.55f;
            g.player.darknessDamageTimer = 0.0f;
        }
    }
    else
    {
        g.player.darknessDamageTimer = 0.0f;
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

    // Ajuste ambient global conforme estado das luzes
    {
        LightCycleState ls = lightSystemGetState(g.lightSystem);
        if (ls == LightCycleState::ON) {
            GLfloat amb[] = {0.045f, 0.045f, 0.06f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        } else if (ls == LightCycleState::FLICKER) {
            GLfloat amb[] = {0.02f, 0.02f, 0.03f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        } else { // OFF
            GLfloat amb[] = {0.005f, 0.005f, 0.008f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        }
    }

    drawSkydome(camX, camY, camZ, g.r);
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
    hs.currentAmmo = g.player.currentAmmo;
    hs.reserveAmmo = g.player.reserveAmmo;
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.weaponState = g.weapon.state;
    hs.flashlightOn = g.flashlightOn;

    // --- ESTADO: MENU INICIAL ---
    if (g.state == GameState::MENU_INICIAL)
    {
        // menuRender já cuida do fogo (update + render)
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para Jogar", g.r);
    }
    // --- ESTADO: GAME OVER ---
    else if (g.state == GameState::GAME_OVER)
    {
        // Fundo 3D congelado
        drawWorld3D();

        // OVERLAY DO MELT por cima do jogo
        // menuMeltRenderOverlay(janelaW, janelaH, g.time);

        // Tela do game over por cima (com fogo)
        menuRender(janelaW, janelaH, g.time, "GAME OVER", "Pressione ENTER para Reiniciar", g.r);
    }
    // --- ESTADO: PAUSADO ---
    else if (g.state == GameState::PAUSADO)
    {
        // 1) Mundo 3D congelado
        drawWorld3D();

        // 2) HUD normal (arma + barra + mira + overlays)
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);

        // 3) Menu escuro por cima
        pauseMenuRender(janelaW, janelaH, g.time);
    }
    // --- ESTADO: JOGANDO ---
    else // JOGANDO
    {
        // 1) Mundo 3D
        drawWorld3D();

        // 2) HUD completo
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);

        menuMeltRenderOverlay(janelaW, janelaH, g.time);
    }

    glutSwapBuffers();
}
