#pragma once
#include "core/lightpost.h"
#include <vector>

enum class LightCycleState {
    ON,      // Luzes acesas — safe zone ativa
    FLICKER, // Piscando — aviso de apagamento iminente
    OFF      // Apagadas — perigo, dano por escuridão
};

struct LightSystem {
    LightCycleState state = LightCycleState::ON;
    float timer = 0.0f;

    // Durações de cada fase (segundos)
    float durationON      = 8.0f;
    float durationFLICKER = 3.0f;
    float durationOFF     = 5.0f;
};

void             lightSystemUpdate(LightSystem& sys, std::vector<LightPost>& posts, float dt);
LightCycleState  lightSystemGetState(const LightSystem& sys);
float            nearestActivePostDist(const std::vector<LightPost>& posts, float px, float pz);
bool             playerIsInSafeZone(const std::vector<LightPost>& posts, float px, float pz, float safeRadius);
