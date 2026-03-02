#pragma once
#include "core/lightpost.h"
#include <vector>

enum class LightCycleState {
    ON,      // Posts deste grupo estão acesas — safe zone ativa
    FLICKER, // Piscando — aviso de troca iminente
    OFF      // Apagadas — perigo
};

struct LightSystem {
    // O sistema alterna DOIS grupos de postes:
    // Grupo A (índices pares) e Grupo B (índices ímpares)
    // Quando A está ON, B está OFF e vice-versa.
    // Entre trocas: breve fase FLICKER.

    LightCycleState stateA = LightCycleState::ON;   // estado do grupo A
    LightCycleState stateB = LightCycleState::OFF;  // estado do grupo B
    float timer = 0.0f;

    // Durações
    float durationON      = 6.0f;  // cada grupo fica ON por este tempo
    float durationFLICKER = 2.0f;  // aviso antes de trocar
    int cycleCount = 0;            // quantas trocas completas aconteceram
};

void             lightSystemUpdate(LightSystem& sys, std::vector<LightPost>& posts, float dt);
LightCycleState  lightSystemGetState(const LightSystem& sys); // retorna estado do grupo A
float            nearestActivePostDist(const std::vector<LightPost>& posts, float px, float pz);
bool             playerIsInSafeZone(const std::vector<LightPost>& posts, float px, float pz, float safeRadius);
bool             isPositionInSafeZone(const std::vector<LightPost>& posts, float px, float pz, float safeRadius);
