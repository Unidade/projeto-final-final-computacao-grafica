// core/game_state.h
#pragma once
#include "core/game_enums.h"
#include "core/light_system.h"
#include <GL/glew.h>

struct PlayerState
{
    int health = 100;
    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    // --- Doom-Light ---
    float batteryCharge       = 100.0f; // 0-100
    float batteryDrainRate    = 8.0f;   // unidades/s quando ligada
    float batteryRechargeRate = 3.0f;   // unidades/s quando desligada
    float darknessDamageTimer = 0.0f;   // acumulador de intervalo de dano

    // --- Luzes Apagadas: battery collectible objective ---
    int batteriesCollected = 0;

    // --- Luzes Apagadas: per-level key (hasLevelKey[level] = true when collected) ---
    bool hasLevelKey[4] = {false, false, false, false};
};

struct WeaponAnim
{
    WeaponState state = WeaponState::W_IDLE;
    float timer = 0.0f;
};

struct RenderAssets
{
    GLuint texChao = 0, texParede = 0, texSangue = 0, texDoor = 0;
    GLuint texChaoInterno = 0, texParedeInterna = 0, texTeto = 0, texMenuBG = 0, texGameOver = 0;

    GLuint texEnemies[5] = {0};
    GLuint texEnemiesRage[5] = {0};
    GLuint texEnemiesDamage[5] = {0};

    GLuint texHealth = 0;
    GLuint texBattery = 0;
    GLuint texKey[3] = {0, 0, 0};

    GLuint progSangue = 0;
};

struct GameContext
{
    GameState state = GameState::MENU_INICIAL;
    PlayerState player;
    WeaponAnim weapon;
    float time = 0.0f;

    bool flashlightOn = true;

    LightSystem lightSystem; // ciclo ON→FLICKER→OFF

    RenderAssets r;
};
