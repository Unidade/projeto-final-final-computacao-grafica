#pragma once

bool isWalkable(float x, float z);
void updateEntities(float dt);

// Configurações da IA
const float ENEMY_SPEED = 4.0f;
const float ENEMY_WANDER_SPEED = 1.5f;
const float ENEMY_VIEW_DIST = 24.0f;      // base view distance (type-specific modifiers apply)
const float ENEMY_ATTACK_DIST = 1.5f;
const float WANDER_DIR_CHANGE_MIN = 1.2f; // slightly more active wandering
const float WANDER_DIR_CHANGE_MAX = 3.0f;

enum EnemyState
{
    STATE_IDLE,
    STATE_CHASE,
    STATE_ATTACK,
    STATE_RETREAT,
    STATE_DEAD
};

// High-level behavioral type for enemies.
// The underlying int index is still used for texture arrays, but behaviors group only 3 types.
enum class EnemyType
{
    BASIC   = 0,
    STALKER = 1,
    BOSS    = 2
};

// Behavior tuning helpers (implemented in entities.cpp)
EnemyType enemyTypeFromInt(int typeIndex);
float getEnemySpeed(EnemyType t);
float getEnemyViewDist(EnemyType t);
float getEnemyWanderSpeed(EnemyType t);
float getEnemyAggroMemory(EnemyType t);

struct Enemy
{
    int type;
    EnemyType typeEnum = EnemyType::BASIC;
    float x, z;       // Posição no mundo
    float hp;         // Vida
    EnemyState state; // Estado atual (IA)
    float startX, startZ;

    float respawnTimer;
    // Animação
    int animFrame;
    float animTimer;

    // NOVO: Tempo de espera entre um ataque e outro
    float attackCooldown;

    // Tempo que ele fica com a textura de dano
    float hurtTimer;

    // Flag para inimigo 3D avatar (GLB) - todos os inimigos são avatars
    bool isAvatar;

    // Chase memory (for stalker-like enemies)
    float chaseMemoryTimer = 0.0f;

    // Wandering: random movement when idle
    float wanderDirX = 0.0f;
    float wanderDirZ = 0.0f;
    float wanderTimer = 0.0f;
};

enum ItemType
{
    ITEM_HEALTH,
    ITEM_BATTERY,
    ITEM_KEY
};

struct Item
{
    float x, z;
    ItemType type;
    bool active; // Se false, já foi pego

    float respawnTimer;

    int keyLevel = 0; // for ITEM_KEY: 1, 2, or 3 (which level this key unlocks)
};