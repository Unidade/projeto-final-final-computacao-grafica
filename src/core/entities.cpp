#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "core/config.h"
#include "core/light_system.h"
#include "audio/audio_system.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

bool isWalkable(float x, float z)
{
    auto& lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size()) return false;
    if (tx < 0 || tx >= (int)data[tz].size()) return false;

    char c = data[tz][tx];
    // Walls block movement
    if (c == '1' || c == '2') return false;

    return true;
}

void updateEntities(float dt)
{
    auto& g = gameContext();
    auto& lvl = gameLevel();
    auto& audio = gameAudio();

    for (auto& en : lvl.enemies)
    {
        if (en.state == STATE_DEAD)
        {
            en.respawnTimer -= dt;
            if (en.respawnTimer <= 0.0f)
            {
                en.state = STATE_IDLE;
                en.hp = 100;
                en.x = en.startX;
                en.z = en.startZ;
                en.hurtTimer = 0.0f;
                en.wanderTimer = 0.0f;  // pick fresh wander dir
            }
            continue;
        }

        if (en.hurtTimer > 0.0f) en.hurtTimer -= dt;

        // --- LUZES APAGADAS: Enemies freeze inside safe zones ---
        // If the enemy is inside the illuminated area of any active light post,
        // it cannot move or attack — it stays idle.
        bool enemyInSafeZone = isPositionInSafeZone(
            lvl.posts, en.x, en.z, GameConfig::SAFE_ZONE_RADIUS);

        if (enemyInSafeZone)
        {
            // Enemy is in the light — frozen, can't chase
            en.state = STATE_IDLE;
            continue;
        }

        // --- LUZES APAGADAS: Monster only hunts when player is in darkness ---
        bool playerInSafeZone = playerIsInSafeZone(
            lvl.posts, camX, camZ, GameConfig::SAFE_ZONE_RADIUS);
        bool playerVisibleToMonster = !playerInSafeZone && !g.flashlightOn;

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        switch (en.state)
        {
        case STATE_IDLE:
        {
            if (playerVisibleToMonster && dist < ENEMY_VIEW_DIST)
            {
                en.state = STATE_CHASE;
                break;
            }
            // Wander: pick new direction when timer expires or direction invalid
            bool needNewDir = (en.wanderTimer <= 0.0f) ||
                             (en.wanderDirX == 0.0f && en.wanderDirZ == 0.0f);
            if (needNewDir)
            {
                float angle = (float)(std::rand() % 360) * (3.14159265f / 180.0f);
                en.wanderDirX = std::cos(angle);
                en.wanderDirZ = std::sin(angle);
                float range = WANDER_DIR_CHANGE_MAX - WANDER_DIR_CHANGE_MIN;
                en.wanderTimer = WANDER_DIR_CHANGE_MIN + (float)(std::rand() % 100) / 100.0f * range;
            }
            else
            {
                en.wanderTimer -= dt;
            }
            float moveStep = ENEMY_WANDER_SPEED * dt;
            float nextX = en.x + en.wanderDirX * moveStep;
            float nextZ = en.z + en.wanderDirZ * moveStep;
            bool nextInSafeZoneX = isPositionInSafeZone(
                lvl.posts, nextX, en.z, GameConfig::SAFE_ZONE_RADIUS);
            bool nextInSafeZoneZ = isPositionInSafeZone(
                lvl.posts, en.x, nextZ, GameConfig::SAFE_ZONE_RADIUS);
            bool canMoveX = isWalkable(nextX, en.z) && !nextInSafeZoneX;
            bool canMoveZ = isWalkable(en.x, nextZ) && !nextInSafeZoneZ;
            if (canMoveX) en.x = nextX;
            else en.wanderDirX = -en.wanderDirX;  // bounce off wall
            if (canMoveZ) en.z = nextZ;
            else en.wanderDirZ = -en.wanderDirZ;
            break;
        }

        case STATE_CHASE:
            if (!playerVisibleToMonster)
            {
                en.state = STATE_IDLE; // Player entered light — monster loses track
                en.wanderTimer = 0.0f; // pick new wander dir immediately
            }
            else if (dist < ENEMY_ATTACK_DIST)
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.5f;
            }
            else if (dist > ENEMY_VIEW_DIST * 1.5f)
            {
                en.state = STATE_IDLE;
                en.wanderTimer = 0.0f;
            }
            else
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                float moveStep = ENEMY_SPEED * dt;

                float nextX = en.x + dirX * moveStep;
                float nextZ = en.z + dirZ * moveStep;

                // Block movement into safe zones (enemies stop at light boundary)
                bool nextInSafeZoneX = isPositionInSafeZone(
                    lvl.posts, nextX, en.z, GameConfig::SAFE_ZONE_RADIUS);
                bool nextInSafeZoneZ = isPositionInSafeZone(
                    lvl.posts, en.x, nextZ, GameConfig::SAFE_ZONE_RADIUS);

                if (isWalkable(nextX, en.z) && !nextInSafeZoneX) en.x = nextX;
                if (isWalkable(en.x, nextZ) && !nextInSafeZoneZ) en.z = nextZ;
            }
            break;

        case STATE_ATTACK:
            if (!playerVisibleToMonster)
            {
                en.state = STATE_IDLE; // Player entered light — can't attack
                en.wanderTimer = 0.0f;
            }
            else if (dist > ENEMY_ATTACK_DIST)
            {
                en.state = STATE_CHASE;
            }
            else
            {
                en.attackCooldown -= dt;
                if (en.attackCooldown <= 0.0f)
                {
                    g.player.health -= 10;
                    en.attackCooldown = 1.0f;
                    g.player.damageAlpha = 1.0f;
                    audioPlayHurt(audio);
                }
            }
            break;

        default:
            break;
        }
    }

    for (auto& item : lvl.items)
    {
        if (!item.active)
        {
            item.respawnTimer -= dt;
            if (item.respawnTimer <= 0.0f) item.active = true;
            continue;
        }

        float dx = camX - item.x;
        float dz = camZ - item.z;

        if (dx * dx + dz * dz < 1.0f)
        {
            item.active = false;

            if (item.type == ITEM_HEALTH)
            {
                item.respawnTimer = 15.0f;
                g.player.health += 50;
                if (g.player.health > 100) g.player.health = 100;
                g.player.healthAlpha = 1.0f;
            }
            else if (item.type == ITEM_BATTERY)
            {
                item.respawnTimer = 999999.0f;
                g.player.batteriesCollected++;
                audioPlayBatteryPickup(audio);
            }
            else if (item.type == ITEM_KEY)
            {
                item.respawnTimer = 999999.0f;
                int lvlIdx = lvl.currentLevel;
                if (lvlIdx >= 1 && lvlIdx <= 3)
                    g.player.hasLevelKey[lvlIdx] = true;
                audioPlayBatteryPickup(audio); // reuse pickup sound
            }
        }
    }
}
