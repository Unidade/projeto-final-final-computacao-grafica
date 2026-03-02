#pragma once
#include <cmath>

namespace GameConfig
{
    constexpr float TILE_SIZE = 4.0f;
    constexpr float PLAYER_EYE_Y = 1.5f;
    constexpr int TIMER_MS = 16; // ~60fps

    // fração do tile usada como raio do jogador (colisão)
    constexpr float PLAYER_RADIUS_FACTOR = 0.35f;
    constexpr float PLAYER_STEPS = 0.15f;

    // --- Luzes Apagadas: Safe Zone ---
    // Safe zone radius = spotlight cone radius on floor
    // = tan(55°) * CEILING_H(5.0) ≈ 7.14
    constexpr float CEILING_H = 5.0f;
    constexpr float CONE_ANGLE_DEG = 55.0f;
    // Can't use tanf in constexpr, so pre-compute: tan(55° * π/180) ≈ 1.4281
    constexpr float SAFE_ZONE_RADIUS = 7.14f;

    // Luzes Apagadas: batteries needed to activate elevator
    constexpr int BATTERIES_REQUIRED = 14;

    // Light cycle tuning (used by LightSystem)
    constexpr float LIGHT_CYCLE_ON_SECONDS = 6.0f;
    constexpr float LIGHT_CYCLE_FLICKER_SECONDS = 2.0f;
}
