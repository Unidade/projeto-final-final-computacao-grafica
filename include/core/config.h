#pragma once
#include <cmath>

namespace GameConfig
{
    constexpr float TILE_SIZE = 4.0f;
    constexpr float PLAYER_EYE_Y = 1.5f;
    constexpr int TIMER_MS = 16; // ~60fps

    // fração do tile usada como raio do jogador (colisão)
    constexpr float PLAYER_RADIUS_FACTOR = 0.35f;
    constexpr float PLAYER_STEPS = 0.11f;

    // --- Luzes Apagadas: Safe Zone ---
    constexpr float CEILING_H = 10.0f;
    constexpr float CONE_ANGLE_DEG = 55.0f;
    // Smaller radius = less overlap, must traverse dark between posts
    constexpr float SAFE_ZONE_RADIUS = 9.0f;
    // Min distance between light posts (units) to avoid overlapping safe zones
    constexpr float MIN_POST_DISTANCE = 24.0f;

    // Luzes Apagadas: batteries needed to activate elevator
    constexpr int BATTERIES_REQUIRED = 14;

    // Light cycle tuning (used by LightSystem)
    constexpr float LIGHT_CYCLE_ON_SECONDS = 6.0f;
    constexpr float LIGHT_CYCLE_FLICKER_SECONDS = 2.0f;
}
