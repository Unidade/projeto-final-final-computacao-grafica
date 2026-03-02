#pragma once

#include "audio/audio_engine.h"
#include "audio/audio_tuning.h"
#include "core/camera.h" // Vec3
#include <vector>

// Forward decl
struct Level;

// Listener (posição/orientação do player)
struct AudioListener {
    Vec3 pos;
    Vec3 vel;
    Vec3 forward;
    Vec3 up;
};

// Estado do sistema de áudio (tudo que era global no game.cpp)
struct AudioSystem {
    AudioEngine engine;
    bool ok = false;

    // Buffers
    ALuint bufAmbient = 0;
    ALuint bufStep = 0;
    ALuint bufHurt = 0;
    ALuint bufClickReload = 0;
    ALuint bufKill = 0;

    ALuint bufMonsterChase = 0;
    ALuint bufMonsterAttack = 0;
    ALuint bufMonsterIdle = 0;
    ALuint bufMonsterSpot = 0;

    ALuint bufBreath = 0;

    ALuint bufEnemy = 0;
    ALuint bufEnemyScream = 0;

    // Sources
    ALuint srcAmbient = 0;
    ALuint srcStep = 0;
    ALuint srcHurt = 0;
    ALuint srcClickReload = 0;
    ALuint srcKill = 0;

    ALuint srcBreath = 0;

    bool stepPlaying = false;

    // Inimigos
    std::vector<ALuint> srcEnemies;       // loop 3D por inimigo (usado para chase/idle)
    std::vector<ALuint> srcEnemyScreams;  // one-shot 3D por inimigo
    std::vector<ALuint> srcEnemySpots;    // one-shot 3D por inimigo
    std::vector<ALuint> srcEnemyAttacks;  // one-shot 3D por inimigo
    std::vector<float> enemyScreamTimer;  // cooldown randômico
    std::vector<int> enemyPrevState;      // detectar mudança de estado
};

// -------- API do módulo --------

// Inicializa OpenAL, carrega buffers, cria sources "globais" e prepara vetores por inimigo.
// Chame depois do level já estar carregado (para saber quantos inimigos existem).
void audioInit(AudioSystem& a, const Level& level);

// Atualiza listener, passos, loops de inimigos, lava, respiração, screams e kill-detect.
void audioUpdate(
    AudioSystem& a,
    const Level& level,
    const AudioListener& listener,
    float dt,
    bool playerMoving,
    int playerHealth
);

// SFX diretos disparados por gameplay
void audioPlayPumpClick(AudioSystem& a);
void audioPlayHurt(AudioSystem& a);
void audioPlayKillAt(AudioSystem& a, float x, float z);
void audioPlayBatteryPickup(AudioSystem& a);
