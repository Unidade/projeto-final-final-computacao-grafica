#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "core/config.h"
#include "core/light_system.h"
#include "audio/audio_system.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cfloat>

EnemyType enemyTypeFromInt(int typeIndex)
{
    switch (typeIndex)
    {
    case 2:
        return EnemyType::BOSS;
    case 1:
    case 4:
        return EnemyType::STALKER;
    case 0:
    case 3:
    default:
        return EnemyType::BASIC;
    }
}

float getEnemySpeed(EnemyType t)
{
    switch (t)
    {
    case EnemyType::STALKER:
        return ENEMY_SPEED * 1.2f;
    case EnemyType::BOSS:
        return ENEMY_SPEED * 1.5f;
    case EnemyType::BASIC:
    default:
        return ENEMY_SPEED;
    }
}

float getEnemyViewDist(EnemyType t)
{
    switch (t)
    {
    case EnemyType::STALKER:
        return ENEMY_VIEW_DIST * 1.3f;
    case EnemyType::BOSS:
        // Effectively global on typical map sizes, still finite for audio falloff
        return ENEMY_VIEW_DIST * 4.0f;
    case EnemyType::BASIC:
    default:
        return ENEMY_VIEW_DIST;
    }
}

float getEnemyWanderSpeed(EnemyType t)
{
    switch (t)
    {
    case EnemyType::STALKER:
        return ENEMY_WANDER_SPEED * 1.1f;
    case EnemyType::BOSS:
        return ENEMY_WANDER_SPEED * 1.2f;
    case EnemyType::BASIC:
    default:
        return ENEMY_WANDER_SPEED;
    }
}

float getEnemyAggroMemory(EnemyType t)
{
    switch (t)
    {
    case EnemyType::STALKER:
        return 2.5f; // seconds of extra chase after losing visibility
    case EnemyType::BOSS:
        return 4.0f;
    case EnemyType::BASIC:
    default:
        return 0.0f;
    }
}

bool isWalkable(float x, float z)
{
    auto &lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto &data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size())
        return false;
    if (tx < 0 || tx >= (int)data[tz].size())
        return false;

    char c = data[tz][tx];
    // Walls block movement
    if (c == '1' || c == '2')
        return false;
    // Enemies should not stand exactly on light posts (P tile) to
    // avoid getting visually "inside" the post mesh.
    if (c == 'P')
        return false;

    return true;
}

void updateEntities(float dt)
{
    auto &g = gameContext();
    auto &lvl = gameLevel();
    auto &audio = gameAudio();

    for (auto &en : lvl.enemies)
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
                en.wanderTimer = 0.0f; // pick fresh wander dir
            }
            continue;
        }

        if (en.hurtTimer > 0.0f)
            en.hurtTimer -= dt;

        // --- LUZES APAGADAS: Monster only hunts quando o jogador está claramente no escuro ---
        // Usamos um raio levemente MAIOR para a IA, para não começar chase cedo demais
        // quando o jogador ainda parece estar sob a luz do poste.
        bool playerInSafeZoneForAI = playerIsInSafeZone(
            lvl.posts, camX, camZ, GameConfig::SAFE_ZONE_RADIUS * 1.15f);

        bool playerVisibleToMonster = !playerInSafeZoneForAI && !g.flashlightOn;

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        // Procura o poste ativo mais próximo para lógica de retreat
        const LightPost *nearestPost = nullptr;
        float nearestPostDistSq = FLT_MAX;
        for (const auto &p : lvl.posts)
        {
            // Para lógica de IA/safe zone, basta o poste estar ativo.
            // O brilho (intensity) pode piscar livremente sem afetar
            // quais postes contam como "mais próximos".
            if (!p.active)
                continue;

            float pdx = en.x - p.x;
            float pdz = en.z - p.z;
            float d2 = pdx * pdx + pdz * pdz;
            if (d2 < nearestPostDistSq)
            {
                nearestPostDistSq = d2;
                nearestPost = &p;
            }
        }
        float dPost = nearestPost ? std::sqrt(nearestPostDistSq) : FLT_MAX;
        float viewDist = getEnemyViewDist(en.typeEnum);

        bool isBoss = (en.typeEnum == EnemyType::BOSS);

        // Para o boss, a lanterna não oferece proteção — apenas os postes/safe zones.
        bool canSeePlayerNow = playerVisibleToMonster;
        if (isBoss)
            canSeePlayerNow = !playerInSafeZoneForAI;

        // Se o inimigo já está dentro de uma safe zone ativa, ele deve
        // imediatamente evitar ficar ali (não pode caçar/atacar de dentro da luz).
        bool enemyInSafeZone = nearestPost &&
                               (dPost <= GameConfig::SAFE_ZONE_RADIUS);

        // "Near" inclui a própria borda da safe zone – usamos um pequeno buffer
        // para empurrar o inimigo alguns passos para fora.
        bool nearActivePost = nearestPost &&
                              (dPost < GameConfig::SAFE_ZONE_RADIUS * 1.1f);

        bool retreatDesired = false;

        // Regra forte: se o jogador está em uma safe zone ativa:
        // - Qualquer inimigo que esteja dentro ou muito perto da safe zone entra em RETREAT.
        // - Inimigos em CHASE/ATTACK fora do raio voltam para IDLE (não podem continuar caçando).
        if (playerInSafeZoneForAI)
        {
            if (enemyInSafeZone || nearActivePost)
            {
                en.state = STATE_RETREAT;
            }
            else if (en.state == STATE_CHASE || en.state == STATE_ATTACK)
            {
                en.state = STATE_IDLE;
                en.chaseMemoryTimer = 0.0f;
                en.wanderTimer = 0.0f;
            }
        }
        else
        {
            // Comportamento normal de retreat quando o jogador não está claramente em safe zone.
            if (enemyInSafeZone)
            {
                retreatDesired = true;
            }
            else if (nearActivePost)
            {
                if (isBoss)
                {
                    // BOSS só respeita safe zones de poste, não a lanterna:
                    // ele recua quando o JOGADOR está protegido na luz.
                    retreatDesired = playerInSafeZoneForAI;
                }
                else
                {
                    // Inimigos comuns também são "espantados" quando perdem visão
                    // do jogador perto de um poste (ex: jogador liga a lanterna).
                    retreatDesired = playerInSafeZoneForAI ||
                                     (!playerVisibleToMonster &&
                                      dist < GameConfig::SAFE_ZONE_RADIUS * 1.5f);
                }
            }

            if (retreatDesired && en.state != STATE_DEAD)
            {
                en.state = STATE_RETREAT;
            }
        }

        switch (en.state)
        {
        case STATE_IDLE:
        {
            if (canSeePlayerNow && dist < viewDist)
            {
                en.state = STATE_CHASE;
                en.chaseMemoryTimer = getEnemyAggroMemory(en.typeEnum);
                break;
            }
            // Wander: pick new direction when timer expires or direction invalid.
            // Evita ficar indo e voltando na mesma linha: novo vetor não deve ser
            // praticamente oposto ao anterior.
            bool needNewDir = (en.wanderTimer <= 0.0f) ||
                              (en.wanderDirX == 0.0f && en.wanderDirZ == 0.0f);
            if (needNewDir)
            {
                float prevX = en.wanderDirX;
                float prevZ = en.wanderDirZ;

                float angle = 0.0f;
                float newX = 0.0f, newZ = 0.0f;
                bool chosen = false;

                for (int attempts = 0; attempts < 8; ++attempts)
                {
                    angle = (float)(std::rand() % 360) * (3.14159265f / 180.0f);
                    newX = std::cos(angle);
                    newZ = std::sin(angle);

                    // Se não havia direção anterior, qualquer uma serve
                    if (prevX == 0.0f && prevZ == 0.0f)
                    {
                        chosen = true;
                        break;
                    }

                    float dot = newX * prevX + newZ * prevZ;
                    // Evita escolher direção quase oposta (dot ~ -1.0)
                    if (dot > -0.5f)
                    {
                        chosen = true;
                        break;
                    }
                }

                if (!chosen)
                {
                    newX = std::cos(angle);
                    newZ = std::sin(angle);
                }

                en.wanderDirX = newX;
                en.wanderDirZ = newZ;

                float range = WANDER_DIR_CHANGE_MAX - WANDER_DIR_CHANGE_MIN;
                en.wanderTimer = WANDER_DIR_CHANGE_MIN +
                                 (float)(std::rand() % 100) / 100.0f * range;
            }
            else
            {
                en.wanderTimer -= dt;
            }
            float moveStep = getEnemyWanderSpeed(en.typeEnum) * dt;
            float nextX = en.x + en.wanderDirX * moveStep;
            float nextZ = en.z + en.wanderDirZ * moveStep;
            bool nextInSafeZoneX = isPositionInSafeZone(
                lvl.posts, nextX, en.z, GameConfig::SAFE_ZONE_RADIUS);
            bool nextInSafeZoneZ = isPositionInSafeZone(
                lvl.posts, en.x, nextZ, GameConfig::SAFE_ZONE_RADIUS);
            bool canMoveX = isWalkable(nextX, en.z) && !nextInSafeZoneX;
            bool canMoveZ = isWalkable(en.x, nextZ) && !nextInSafeZoneZ;
            if (canMoveX)
            {
                en.x = nextX;
            }
            else
            {
                // Em vez de ficar apenas invertendo eixo (back-and-forth),
                // força uma nova direção no próximo frame.
                en.wanderTimer = 0.0f;
            }
            if (canMoveZ)
            {
                en.z = nextZ;
            }
            else
            {
                en.wanderTimer = 0.0f;
            }
            break;
        }

        case STATE_CHASE:
        {
            float memory = getEnemyAggroMemory(en.typeEnum);

            // Apenas inimigos comuns usam memória de perseguição; o boss persegue
            // continuamente enquanto o jogador não estiver em uma safe zone.
            if (!isBoss && !playerVisibleToMonster && memory > 0.0f && !playerInSafeZoneForAI)
            {
                if (en.chaseMemoryTimer <= 0.0f)
                    en.chaseMemoryTimer = memory;
                else
                    en.chaseMemoryTimer -= dt;
            }

            bool lostPlayer = false;
            if (isBoss)
            {
                // O boss só “perde” o jogador quando ele entra numa safe zone.
                if (playerInSafeZoneForAI)
                    lostPlayer = true;
            }
            else
            {
                lostPlayer = !playerVisibleToMonster &&
                             (en.chaseMemoryTimer <= 0.0f || playerInSafeZoneForAI);
            }
            if (lostPlayer)
            {
                en.state = STATE_IDLE; // Player protegido ou longe demais
                en.wanderTimer = 0.0f; // pick new wander dir immediately
                en.chaseMemoryTimer = 0.0f;
                break;
            }

            if (dist < ENEMY_ATTACK_DIST)
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.5f;
            }
            else if (dist > viewDist * 1.5f && en.typeEnum != EnemyType::BOSS)
            {
                en.state = STATE_IDLE;
                en.wanderTimer = 0.0f;
            }
            else
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                float moveStep = getEnemySpeed(en.typeEnum) * dt;

                float nextX = en.x + dirX * moveStep;
                float nextZ = en.z + dirZ * moveStep;

                // Block movement into safe zones (enemies stop at light boundary)
                bool nextInSafeZoneX = isPositionInSafeZone(
                    lvl.posts, nextX, en.z, GameConfig::SAFE_ZONE_RADIUS);
                bool nextInSafeZoneZ = isPositionInSafeZone(
                    lvl.posts, en.x, nextZ, GameConfig::SAFE_ZONE_RADIUS);

                if (isWalkable(nextX, en.z) && !nextInSafeZoneX)
                    en.x = nextX;
                if (isWalkable(en.x, nextZ) && !nextInSafeZoneZ)
                    en.z = nextZ;
            }
            break;
        }

        case STATE_ATTACK:
        {
            bool canAttackNow = playerVisibleToMonster;
            if (isBoss)
                canAttackNow = !playerInSafeZoneForAI;

            if (!canAttackNow)
            {
                en.state = STATE_IDLE; // Player entrou em proteção — não pode atacar
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
                    en.attackCooldown = 0.8f;
                    g.player.damageAlpha = 1.0f;
                    audioPlayHurt(audio);
                }
            }
            break;
        }

        case STATE_RETREAT:
        {
            if (!nearestPost)
            {
                en.state = STATE_IDLE;
                en.wanderTimer = 0.0f;
                break;
            }

            // Empurra o inimigo para longe do poste mais próximo, sem entrar em paredes
            float awayX = en.x - nearestPost->x;
            float awayZ = en.z - nearestPost->z;
            float len = std::sqrt(awayX * awayX + awayZ * awayZ);
            if (len < 0.0001f)
            {
                // Direção aleatória se estiver exatamente em cima do poste
                float ang = (float)(std::rand() % 360) * (3.14159265f / 180.0f);
                awayX = std::cos(ang);
                awayZ = std::sin(ang);
                len = 1.0f;
            }
            awayX /= len;
            awayZ /= len;

            float moveStep = getEnemyWanderSpeed(en.typeEnum) * dt;
            if (en.typeEnum == EnemyType::BOSS)
                moveStep = getEnemySpeed(en.typeEnum) * dt;

            float nextX = en.x + awayX * moveStep;
            float nextZ = en.z + awayZ * moveStep;

            bool canMoveX = isWalkable(nextX, en.z);
            bool canMoveZ = isWalkable(en.x, nextZ);

            if (canMoveX)
                en.x = nextX;
            if (canMoveZ)
                en.z = nextZ;

            // Quando estiver um pouco além da borda segura, volta para IDLE
            if (dPost >= GameConfig::SAFE_ZONE_RADIUS * 1.1f)
            {
                en.state = STATE_IDLE;
                en.wanderTimer = 0.0f;
            }
            break;
        }

        default:
            break;
        }
    }

    for (auto &item : lvl.items)
    {
        if (!item.active)
        {
            item.respawnTimer -= dt;
            if (item.respawnTimer <= 0.0f)
                item.active = true;
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
                if (g.player.health > 100)
                    g.player.health = 100;
                g.player.healthAlpha = 1.0f;
            }
            else if (item.type == ITEM_BATTERY)
            {
                item.respawnTimer = 999999.0f;
                g.player.batteriesCollected++;
                lvl.batteriesCollectedInMap++;
                audioPlayBatteryPickup(audio);
                // Flash verde ao completar todas as baterias
                if (lvl.batteriesCollectedInMap >= lvl.batteriesRequiredInMap
                    && lvl.batteriesRequiredInMap > 0)
                {
                    g.allBatteriesFlashTimer = 2.5f;
                    g.doorMessageText  = "Todas as baterias coletadas!";
                    g.doorMessageTimer = 3.0f;
                }
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
