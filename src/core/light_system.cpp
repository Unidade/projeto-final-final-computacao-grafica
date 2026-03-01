#include "core/light_system.h"
#include <cmath>
#include <cfloat>

// ---------------------------------------------------------------------------
// Update: avança a máquina de estados e atualiza intensity dos postes
// ---------------------------------------------------------------------------
void lightSystemUpdate(LightSystem& sys, std::vector<LightPost>& posts, float dt)
{
    sys.timer += dt;

    switch (sys.state)
    {
    // ---- LIGADO: todas as luzes acesas ----
    case LightCycleState::ON:
        for (auto& p : posts) {
            p.active    = true;
            p.intensity = 1.0f;
        }
        if (sys.timer >= sys.durationON) {
            sys.state = LightCycleState::FLICKER;
            sys.timer = 0.0f;
        }
        break;

    // ---- PISCANDO: aviso de apagamento ----
    case LightCycleState::FLICKER:
        for (auto& p : posts) {
            p.active = true;
            // Oscilação rápida com fase diferente por poste (para não sincronia)
            float flick = sinf(sys.timer * 14.0f + p.x * 3.7f + p.z * 2.3f);
            p.intensity = (flick > -0.3f) ? (0.5f + flick * 0.5f) : 0.05f;
        }
        if (sys.timer >= sys.durationFLICKER) {
            sys.state = LightCycleState::OFF;
            sys.timer = 0.0f;
        }
        break;

    // ---- APAGADO: escuridão total, dano ativo ----
    case LightCycleState::OFF:
        for (auto& p : posts) {
            p.active    = false;
            p.intensity = 0.0f;
        }
        if (sys.timer >= sys.durationOFF) {
            sys.state = LightCycleState::ON;
            sys.timer = 0.0f;
        }
        break;
    }
}

LightCycleState lightSystemGetState(const LightSystem& sys)
{
    return sys.state;
}

// ---------------------------------------------------------------------------
// Distância ao poste ativo mais próximo (retorna FLT_MAX se nenhum ativo)
// ---------------------------------------------------------------------------
float nearestActivePostDist(const std::vector<LightPost>& posts, float px, float pz)
{
    float minDist = FLT_MAX;
    for (const auto& p : posts) {
        if (!p.active || p.intensity < 0.05f) continue;
        float ddx = px - p.x;
        float ddz = pz - p.z;
        float d   = sqrtf(ddx * ddx + ddz * ddz);
        if (d < minDist) minDist = d;
    }
    return minDist;
}

bool playerIsInSafeZone(const std::vector<LightPost>& posts, float px, float pz, float safeRadius)
{
    return nearestActivePostDist(posts, px, pz) <= safeRadius;
}
