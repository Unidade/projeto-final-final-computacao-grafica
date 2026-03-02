#include "audio/audio_system.h"
#include "level/level.h"
#include "core/entities.h" // Enemy states, etc.

#include <cstdio>
#include <cmath>
#include <cstdlib>

// ---------------- helpers internos ----------------

static inline float frand01() {
    return (float)std::rand() / (float)RAND_MAX;
}

static void stopIf(ALuint s, AudioEngine& e) {
    if (s) e.stop(s);
}

static void play2D(AudioSystem& a, ALuint s) {
    if (!a.ok || s == 0) return;
    a.engine.stop(s);
    a.engine.play(s);
}

static void play3DAt(AudioSystem& a, ALuint s, float x, float z) {
    if (!a.ok || s == 0) return;
    a.engine.setSourcePos(s, {x, 0.0f, z});
    a.engine.stop(s);
    a.engine.play(s);
}



// Cria/atualiza arrays de sources de inimigos conforme quantidade no level
static void ensureEnemySources(AudioSystem& a, const Level& level) {
    if (!a.ok || a.bufEnemy == 0) return;

    const size_t need = level.enemies.size();
    if (a.srcEnemies.size() == need) return;

    for (ALuint s : a.srcEnemies) stopIf(s, a.engine);
    a.srcEnemies.assign(need, 0);

    for (size_t i = 0; i < need; ++i) {
        ALuint s = a.engine.createSource(a.bufEnemy, true);
        if (!s) continue;

        alSourcei(s, AL_SOURCE_RELATIVE, AL_FALSE);
        a.engine.setSourceDistance(s,
            AudioTuning::ENEMY_REF_DIST,
            AudioTuning::ENEMY_ROLLOFF,
            AudioTuning::ENEMY_MAX_DIST
        );
        a.engine.setSourceGain(s, AudioTuning::MASTER * AudioTuning::ENEMY_BASE_GAIN);
        a.engine.stop(s);

        a.srcEnemies[i] = s;
    }
}

static void ensureEnemyExtra(AudioSystem& a, const Level& level) {
    if (!a.ok) return;

    const size_t n = level.enemies.size();

    if (a.srcEnemyScreams.size() != n) {
        for (ALuint s : a.srcEnemyScreams) stopIf(s, a.engine);
        a.srcEnemyScreams.assign(n, 0);
        a.enemyScreamTimer.assign(n, 0.0f);
    }
    if (a.srcEnemySpots.size() != n) {
        for (ALuint s : a.srcEnemySpots) stopIf(s, a.engine);
        a.srcEnemySpots.assign(n, 0);
    }
    if (a.srcEnemyAttacks.size() != n) {
        for (ALuint s : a.srcEnemyAttacks) stopIf(s, a.engine);
        a.srcEnemyAttacks.assign(n, 0);
    }

    if (a.enemyPrevState.size() != n) {
        a.enemyPrevState.assign(n, (int)STATE_IDLE);
    }

    // Garante sources por inimigo para cada tipo de som
    for (size_t i = 0; i < n; ++i) {
        // Scream (random)
        if (a.bufEnemyScream && a.srcEnemyScreams[i] == 0) {
            a.srcEnemyScreams[i] = a.engine.createSource(a.bufEnemyScream, false);
            if (a.srcEnemyScreams[i]) {
                alSourcei(a.srcEnemyScreams[i], AL_SOURCE_RELATIVE, AL_FALSE);
                a.engine.setSourceGain(a.srcEnemyScreams[i], AudioTuning::MASTER * AudioTuning::ENEMY_SCREAM_GAIN);
                a.engine.setSourceDistance(a.srcEnemyScreams[i], AudioTuning::ENEMY_REF_DIST, AudioTuning::ENEMY_ROLLOFF * 0.8f, AudioTuning::SCREAM_MAX_AUDIBLE_DIST);
            }
        }
        // Spot (one-shot when seeing player)
        if (a.bufMonsterSpot && a.srcEnemySpots[i] == 0) {
            a.srcEnemySpots[i] = a.engine.createSource(a.bufMonsterSpot, false);
            if (a.srcEnemySpots[i]) {
                alSourcei(a.srcEnemySpots[i], AL_SOURCE_RELATIVE, AL_FALSE);
                a.engine.setSourceGain(a.srcEnemySpots[i], AudioTuning::MASTER * AudioTuning::MONSTER_SPOT_GAIN);
                a.engine.setSourceDistance(a.srcEnemySpots[i], AudioTuning::ENEMY_REF_DIST, AudioTuning::ENEMY_ROLLOFF, AudioTuning::ENEMY_MAX_DIST);
            }
        }
        // Attack (one-shot)
        if (a.bufMonsterAttack && a.srcEnemyAttacks[i] == 0) {
            a.srcEnemyAttacks[i] = a.engine.createSource(a.bufMonsterAttack, false);
            if (a.srcEnemyAttacks[i]) {
                alSourcei(a.srcEnemyAttacks[i], AL_SOURCE_RELATIVE, AL_FALSE);
                a.engine.setSourceGain(a.srcEnemyAttacks[i], AudioTuning::MASTER * AudioTuning::MONSTER_ATTACK_GAIN);
                a.engine.setSourceDistance(a.srcEnemyAttacks[i], AudioTuning::ENEMY_REF_DIST, AudioTuning::ENEMY_ROLLOFF, AudioTuning::ENEMY_MAX_DIST);
            }
        }
    }
}

// ---------------- API ----------------

void audioInit(AudioSystem& a, const Level& level) {
    if (a.ok) return;

    a.ok = a.engine.init();
    if (!a.ok) {
        std::printf("[Audio] Falha ao iniciar OpenAL (seguindo sem audio)\n");
        return;
    }

    a.engine.setDistanceModel();

    // Loads (com fallback mono/estéreo)
    a.bufAmbient = a.engine.loadWav("assets/audio/music.wav");
    if (!a.bufAmbient) a.bufAmbient = a.engine.loadWav("assets/audio/music.wav");

    a.bufShot = a.engine.loadWav("assets/audio/shot_mono.wav");
    if (!a.bufShot) a.bufShot = a.engine.loadWav("assets/audio/shot.wav");

    a.bufStep = a.engine.loadWav("assets/audio/step_mono.wav");
    if (!a.bufStep) a.bufStep = a.engine.loadWav("assets/audio/step.wav");

    a.bufEnemy = a.engine.loadWav("assets/audio/enemy_mono.wav");
    if (!a.bufEnemy) a.bufEnemy = a.engine.loadWav("assets/audio/enemy.wav");

    a.bufReload = a.engine.loadWav("assets/audio/reload_mono.wav");
    if (!a.bufReload) a.bufReload = a.engine.loadWav("assets/audio/reload.wav");

    a.bufClickReload = a.engine.loadWav("assets/audio/click_reload_mono.wav");

    a.bufKill = a.engine.loadWav("assets/audio/kill_mono.wav");

    a.bufEnemyScream = a.engine.loadWav("assets/audio/enemy_scream_mono.wav");

    a.bufHurt = a.engine.loadWav("assets/audio/hurt_mono.wav");
    if (!a.bufHurt) a.bufHurt = a.engine.loadWav("assets/audio/hurt.wav");


    a.bufMonsterChase = a.engine.loadWav("assets/audio/sfx/sfx_monster_chase.wav.wav");
    a.bufMonsterAttack = a.engine.loadWav("assets/audio/sfx/sfx_monster_attack.wav.wav");
    a.bufMonsterIdle = a.engine.loadWav("assets/audio/sfx/sfx_monster_idle.wav.wav");
    a.bufMonsterSpot = a.engine.loadWav("assets/audio/sfx/sfx_monster_spot.mp3");

    a.bufBreath = a.engine.loadWav("assets/audio/breath_mono.wav");

    a.bufGrunt = a.engine.loadWav("assets/audio/grunt_mono.wav");

    // Ambient (2D loop)
    if (a.bufAmbient) {
        a.srcAmbient = a.engine.createSource(a.bufAmbient, true);
        alSourcei(a.srcAmbient, AL_SOURCE_RELATIVE, AL_TRUE);
        a.engine.setSourcePos(a.srcAmbient, {0.0f, 0.0f, 0.0f});
        a.engine.setSourceDistance(a.srcAmbient, 1.0f, 0.0f, 1000.0f);
        a.engine.setSourceGain(a.srcAmbient, AudioTuning::MASTER * AudioTuning::AMBIENT_GAIN);
        a.engine.play(a.srcAmbient);
    }

    // Shot (2D one-shot)
    if (a.bufShot) {
        a.srcShot = a.engine.createSource(a.bufShot, false);
        alSourcei(a.srcShot, AL_SOURCE_RELATIVE, AL_TRUE);
        a.engine.setSourceGain(a.srcShot, AudioTuning::MASTER * AudioTuning::SHOT_GAIN);
    }

    // Step (2D loop, controlado no update)
    if (a.bufStep) {
        a.srcStep = a.engine.createSource(a.bufStep, true);
        alSourcei(a.srcStep, AL_SOURCE_RELATIVE, AL_TRUE);
        a.engine.setSourceGain(a.srcStep, AudioTuning::MASTER * AudioTuning::STEP_GAIN);
    }

    // Reload (2D one-shot)
    if (a.bufReload) {
        a.srcReload = a.engine.createSource(a.bufReload, false);
        alSourcei(a.srcReload, AL_SOURCE_RELATIVE, AL_TRUE);
        a.engine.setSourceGain(a.srcReload, AudioTuning::MASTER * AudioTuning::RELOAD_GAIN);
    }

    // Pump click (2D one-shot)
    if (a.bufClickReload) {
        a.srcClickReload = a.engine.createSource(a.bufClickReload, false);
        if (a.srcClickReload) {
            alSourcei(a.srcClickReload, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(a.srcClickReload, AL_POSITION, 0, 0, 0);
            a.engine.setSourceGain(a.srcClickReload, AudioTuning::MASTER * AudioTuning::PUMP_GAIN);
        }
    }

    // Kill (3D one-shot)
    if (a.bufKill) {
        a.srcKill = a.engine.createSource(a.bufKill, false);
        if (a.srcKill) {
            alSourcei(a.srcKill, AL_SOURCE_RELATIVE, AL_FALSE);
            a.engine.setSourceGain(a.srcKill, AudioTuning::MASTER * AudioTuning::KILL_GAIN);
            a.engine.setSourceDistance(a.srcKill,
                AudioTuning::ENEMY_REF_DIST,
                AudioTuning::ENEMY_ROLLOFF,
                AudioTuning::ENEMY_MAX_DIST
            );
        }
    }

    // Hurt (2D one-shot)
    if (a.bufHurt) {
        a.srcHurt = a.engine.createSource(a.bufHurt, false);
        alSourcei(a.srcHurt, AL_SOURCE_RELATIVE, AL_TRUE);
        a.engine.setSourceGain(a.srcHurt, AudioTuning::MASTER * AudioTuning::DAMAGE_GAIN);
    }



    // Breath (2D loop sempre tocando, ganho ajustado no update)
    if (a.bufBreath) {
        a.srcBreath = a.engine.createSource(a.bufBreath, true);
        if (a.srcBreath) {
            alSourcei(a.srcBreath, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(a.srcBreath, AL_POSITION, 0.0f, 0.0f, 0.0f);
            a.engine.setSourceGain(a.srcBreath, 0.0f);
            a.engine.play(a.srcBreath);
        }
    }

    // Grunt (2D one-shot)
    if (a.bufGrunt) {
        a.srcGrunt = a.engine.createSource(a.bufGrunt, false);
        if (a.srcGrunt) {
            alSourcei(a.srcGrunt, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(a.srcGrunt, AL_POSITION, 0.0f, 0.0f, 0.0f);
            a.engine.setSourceGain(a.srcGrunt, AudioTuning::MASTER * AudioTuning::GRUNT_GAIN);
        }
    }

    // Inimigos
    ensureEnemySources(a, level);
    ensureEnemyExtra(a, level);
}

void audioUpdate(
    AudioSystem& a,
    const Level& level,
    const AudioListener& listener,
    float dt,
    bool playerMoving,
    int playerHp
) {
    if (!a.ok) return;

    // Listener
    a.engine.setListener(listener.pos, listener.vel, listener.forward, listener.up);

    // Step
    if (a.srcStep) {
        if (playerMoving && !a.stepPlaying) {
            alSourcei(a.srcStep, AL_SOURCE_RELATIVE, AL_TRUE);
            a.engine.setSourcePos(a.srcStep, {0.0f, 0.0f, 0.0f});
            a.engine.setSourceDistance(a.srcStep, 1.0f, 0.0f, 1000.0f);
            a.engine.play(a.srcStep);
            a.stepPlaying = true;
        } else if (!playerMoving && a.stepPlaying) {
            a.engine.stop(a.srcStep);
            a.stepPlaying = false;
        }
    }

    // 3. Enemy SFX State Machine + Loops
    ensureEnemySources(a, level);
    ensureEnemyExtra(a, level);

    for (size_t i = 0; i < level.enemies.size(); ++i) {
        const auto& en = level.enemies[i];
        int prev = a.enemyPrevState[i];
        ALuint sLoop = a.srcEnemies[i];

        if (en.state == STATE_DEAD) {
            if (prev != STATE_DEAD) audioPlayKillAt(a, en.x, en.z);
            if (sLoop) a.engine.stop(sLoop);
            a.enemyPrevState[i] = (int)en.state;
            continue;
        }

        // --- Transition Events (One-shots) ---

        // Spot (seeing player)
        if (prev == STATE_IDLE && en.state == STATE_CHASE) {
            ALuint sSpot = a.srcEnemySpots[i];
            if (sSpot) {
                a.engine.setSourcePos(sSpot, {en.x, 0.0f, en.z});
                a.engine.stop(sSpot);
                a.engine.play(sSpot);
            }
        }
        // Attack
        if (prev != STATE_ATTACK && en.state == STATE_ATTACK) {
            ALuint sAtt = a.srcEnemyAttacks[i];
            if (sAtt) {
                a.engine.setSourcePos(sAtt, {en.x, 0.0f, en.z});
                a.engine.stop(sAtt);
                a.engine.play(sAtt);
            }
        }

        // --- Update Loop (Distance-based playback) ---
        if (sLoop) {
            a.engine.setSourcePos(sLoop, {en.x, 0.0f, en.z});

            // Update buffer/gain based on current state
            if (en.state == STATE_CHASE || en.state == STATE_ATTACK) {
                alSourcei(sLoop, AL_BUFFER, a.bufMonsterChase);
                a.engine.setSourceGain(sLoop, AudioTuning::MASTER * AudioTuning::MONSTER_CHASE_GAIN);
            } else {
                alSourcei(sLoop, AL_BUFFER, a.bufMonsterIdle);
                a.engine.setSourceGain(sLoop, AudioTuning::MASTER * AudioTuning::MONSTER_IDLE_GAIN);
            }

            float dx = en.x - listener.pos.x;
            float dz = en.z - listener.pos.z;
            float dist = std::sqrt(dx * dx + dz * dz);

            ALint st = 0;
            alGetSourcei(sLoop, AL_SOURCE_STATE, &st);
            const bool playing = (st == AL_PLAYING);

            if (!playing && dist <= AudioTuning::ENEMY_START_DIST) {
                a.engine.play(sLoop);
            } else if (playing && dist >= AudioTuning::ENEMY_STOP_DIST) {
                a.engine.stop(sLoop);
            }
        }

        a.enemyPrevState[i] = (int)en.state;
    }

    // screams
    if (a.bufEnemyScream && !a.srcEnemyScreams.empty()) {
        for (size_t i = 0; i < level.enemies.size() && i < a.srcEnemyScreams.size(); ++i) {
            const auto& en = level.enemies[i];
            if (en.state == STATE_DEAD) continue;

            // distância para audibilidade
            float dxs = en.x - listener.pos.x;
            float dzs = en.z - listener.pos.z;
            float ds = std::sqrt(dxs * dxs + dzs * dzs);

            // decai mais lento se muito longe
            if (ds > AudioTuning::SCREAM_MAX_AUDIBLE_DIST) {
                a.enemyScreamTimer[i] -= dt * 0.25f;
                continue;
            }

            a.enemyScreamTimer[i] -= dt;
            if (a.enemyScreamTimer[i] > 0.0f) continue;

            // chance de gritar
            if (frand01() <= AudioTuning::SCREAM_CHANCE) {
                ALuint s = a.srcEnemyScreams[i];
                if (s) {
                    a.engine.setSourcePos(s, {en.x, 0.0f, en.z});
                    a.engine.stop(s);
                    a.engine.setSourceGain(s, AudioTuning::MASTER * AudioTuning::ENEMY_SCREAM_GAIN);
                    a.engine.play(s);
                }
            }

            // re-agenda
            float tmin = AudioTuning::ENEMY_SCREAM_MIN_INTERVAL;
            float tmax = AudioTuning::ENEMY_SCREAM_MAX_INTERVAL;
            a.enemyScreamTimer[i] = tmin + (tmax - tmin) * frand01();
        }
    }



    // Breath gain (low HP)
    if (a.srcBreath) {
        if (playerHp > AudioTuning::LOW_HP_THRESHOLD) {
            a.engine.setSourceGain(a.srcBreath, 0.0f);
        } else {
            float t = (AudioTuning::LOW_HP_THRESHOLD - playerHp) / (float)AudioTuning::LOW_HP_THRESHOLD;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float gain = (AudioTuning::MASTER * AudioTuning::BREATH_GAIN) * (0.20f + t * 1.00f);
            a.engine.setSourceGain(a.srcBreath, gain);

            ALint st = 0;
            alGetSourcei(a.srcBreath, AL_SOURCE_STATE, &st);
            if (st != AL_PLAYING) a.engine.play(a.srcBreath);
        }
    }
}

void audioPlayShot(AudioSystem& a) {
    if (!a.ok || a.srcShot == 0) return;

    alSourcei(a.srcShot, AL_SOURCE_RELATIVE, AL_TRUE);
    a.engine.setSourcePos(a.srcShot, {0.0f, 0.0f, 0.0f});
    a.engine.setSourceDistance(a.srcShot, 1.0f, 0.0f, 1000.0f);

    a.engine.stop(a.srcShot);
    a.engine.play(a.srcShot);
}

void audioPlayReload(AudioSystem& a) {
    if (!a.ok || a.srcReload == 0) return;
    a.engine.stop(a.srcReload);
    a.engine.play(a.srcReload);
}

void audioPlayPumpClick(AudioSystem& a) {
    if (!a.ok || a.srcClickReload == 0) return;
    a.engine.stop(a.srcClickReload);
    a.engine.play(a.srcClickReload);
}

void audioPlayHurt(AudioSystem& a) {
    if (!a.ok || a.srcHurt == 0) return;
    a.engine.stop(a.srcHurt);
    a.engine.play(a.srcHurt);
}

void audioPlayKillAt(AudioSystem& a, float x, float z) {
    play3DAt(a, a.srcKill, x, z);
}

void audioPlayBatteryPickup(AudioSystem& a) {
    play2D(a, a.srcClickReload);
}

void audioOnPlayerShot(AudioSystem& a) {
    if (!a.ok) return;
    a.shotsSinceGrunt++;

    if (a.shotsSinceGrunt >= AudioTuning::GRUNT_EVERY_N_SHOTS) {
        a.shotsSinceGrunt = 0;
        if (a.srcGrunt) {
            a.engine.stop(a.srcGrunt);
            a.engine.play(a.srcGrunt);
        }
    }
}
