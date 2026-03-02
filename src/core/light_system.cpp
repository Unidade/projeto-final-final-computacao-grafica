#include "core/light_system.h"
#include <cmath>
#include <cfloat>

// ---------------------------------------------------------------------------
// Alternating Groups Light System
// Group A = even-indexed posts, Group B = odd-indexed posts
// Cycle: A-ON/B-OFF → A-FLICKER/B-FLICKER → A-OFF/B-ON → ... repeat
// ---------------------------------------------------------------------------

static void applyGroupStates(LightSystem& sys, std::vector<LightPost>& posts)
{
    for (size_t i = 0; i < posts.size(); i++)
    {
        bool isGroupA = (i % 2 == 0);
        LightCycleState state = isGroupA ? sys.stateA : sys.stateB;

        switch (state)
        {
        case LightCycleState::ON:
            posts[i].active    = true;
            posts[i].intensity = 1.0f;
            break;

        case LightCycleState::FLICKER:
            posts[i].active = true;
            {
                float flick = sinf(sys.timer * 14.0f + posts[i].x * 3.7f + posts[i].z * 2.3f);
                posts[i].intensity = (flick > -0.3f) ? (0.5f + flick * 0.35f) : 0.05f;
            }
            break;

        case LightCycleState::OFF:
            posts[i].active    = false;
            posts[i].intensity = 0.0f;
            break;
        }
    }
}

void lightSystemUpdate(LightSystem& sys, std::vector<LightPost>& posts, float dt)
{
    sys.timer += dt;

    // State machine: A-ON/B-OFF phase
    if (sys.stateA == LightCycleState::ON && sys.stateB == LightCycleState::OFF)
    {
        if (sys.timer >= sys.durationON)
        {
            // Transition to FLICKER (both groups flicker as warning)
            sys.stateA = LightCycleState::FLICKER;
            sys.stateB = LightCycleState::FLICKER;
            sys.timer = 0.0f;
        }
    }
    else if (sys.stateA == LightCycleState::OFF && sys.stateB == LightCycleState::ON)
    {
        if (sys.timer >= sys.durationON)
        {
            // Transition to FLICKER
            sys.stateA = LightCycleState::FLICKER;
            sys.stateB = LightCycleState::FLICKER;
            sys.timer = 0.0f;
        }
    }
    else if (sys.stateA == LightCycleState::FLICKER && sys.stateB == LightCycleState::FLICKER)
    {
        if (sys.timer >= sys.durationFLICKER)
        {
            // Swap: whichever was ON before flicker becomes OFF, and vice versa
            // We track this with cycleCount: even = A-ON first, odd = B-ON first
            sys.cycleCount++;
            if (sys.cycleCount % 2 == 0)
            {
                sys.stateA = LightCycleState::ON;
                sys.stateB = LightCycleState::OFF;
            }
            else
            {
                sys.stateA = LightCycleState::OFF;
                sys.stateB = LightCycleState::ON;
            }
            sys.timer = 0.0f;
        }
    }

    applyGroupStates(sys, posts);
}

LightCycleState lightSystemGetState(const LightSystem& sys)
{
    // Return the "worst" state for ambient purposes:
    // if either group is flickering, report flicker
    // if both off (shouldn't happen), report off
    if (sys.stateA == LightCycleState::FLICKER || sys.stateB == LightCycleState::FLICKER)
        return LightCycleState::FLICKER;
    // Some posts are always on in alternating mode
    return LightCycleState::ON;
}

// ---------------------------------------------------------------------------
// Distance to nearest active post
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

bool isPositionInSafeZone(const std::vector<LightPost>& posts, float px, float pz, float safeRadius)
{
    return nearestActivePostDist(posts, px, pz) <= safeRadius;
}
