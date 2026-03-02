#pragma once

#include "core/game_enums.h"
#include <GL/glew.h>


struct HudTextures
{
    GLuint texLinternOn = 0;
    GLuint texLinternOff = 0;
    GLuint texDamage = 0;
    GLuint texHealthOverlay = 0;
    GLuint texKeyHud[3] = {0, 0, 0}; // Key0, Key1, Key2 for levels 1-3
};

struct HudState
{
    int playerHealth = 100;
    int batteriesCollected = 0;
    int batteriesRequired = 14;
    bool hasLevelKey = false; // key for current level collected
    int currentLevel = 1;     // to pick correct key icon

    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    bool flashlightOn = true;
};

void hudRenderAll(
    int screenW,
    int screenH,
    const HudTextures& tex,
    const HudState& state,
    bool showCrosshair,
    bool showWeapon,
    bool showStatusBar);

void desenhaTutorial(float time, int w, int h);
