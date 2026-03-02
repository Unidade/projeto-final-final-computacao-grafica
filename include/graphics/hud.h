#pragma once

#include "core/game_enums.h"
#include <GL/glew.h>


struct HudTextures
{
    GLuint texHudFundo = 0;
    GLuint texGunHUD = 0;

    GLuint texLinternOn = 0;
    GLuint texLinternOff = 0;
    GLuint texGunDefault = 0;
    GLuint texGunFire1 = 0;
    GLuint texGunFire2 = 0;
    GLuint texGunReload1 = 0;
    GLuint texGunReload2 = 0;

    GLuint texDamage = 0;
    GLuint texHealthOverlay = 0;
};

struct HudState
{
    int playerHealth = 100;
    int currentAmmo = 0;
    int reserveAmmo = 0;
    int batteriesCollected = 0;
    int batteriesRequired = 14;

    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    WeaponState weaponState = WeaponState::W_IDLE;
    bool flashlightOn = true;
};

void hudRenderAll(
    int screenW,
    int screenH,
    const HudTextures& tex,
    const HudState& state,
    bool showCrosshair,
    bool showWeapon,
    bool showDoomBar);
