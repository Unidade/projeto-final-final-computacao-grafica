#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    a.texMenuBG = carregaTextura("assets/textures/menu.jpeg");
    if (!a.texMenuBG) a.texMenuBG = carregaTextura("assets/menu_bg.png");
    a.texGameOver = carregaTextura("assets/textures/gameover.jpeg");

    // --- Dark horror textures ---
    a.texChao = carregaTextura("assets/textures/floors/floor_concrete_wet.jpg");
    a.texParede = carregaTextura("assets/textures/walls/wall_concrete_dark.png.jpg");
    a.texSangue = carregaTextura("assets/016.png");
    a.texChaoInterno = carregaTextura("assets/textures/floors/floor_tile_dirty.jpg");
    a.texParedeInterna = carregaTextura("assets/textures/walls/wall_brick_dark.png.png");
    a.texTeto = carregaTextura("assets/082.png");
    a.texDoor = carregaTextura("assets/textures/doors/door_metal_closed.png.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progBatteryFlash = criaShader("shaders/battery_flash.vert", "shaders/battery_flash.frag");
    a.progTransition = criaShader("shaders/level_transition.vert", "shaders/level_transition.frag");

    // --- INIMIGO 0 ('J') ---
    a.texEnemies[0] = carregaTextura("assets/enemies/enemy.png");
    a.texEnemiesRage[0] = carregaTextura("assets/enemies/enemyRage.png");
    a.texEnemiesDamage[0] = carregaTextura("assets/enemies/enemyRageDamage.png");

    // --- INIMIGO 1 ('T') ---
    a.texEnemies[1] = carregaTextura("assets/enemies/enemy2.png");
    a.texEnemiesRage[1] = carregaTextura("assets/enemies/enemyRage2.png");
    a.texEnemiesDamage[1] = carregaTextura("assets/enemies/enemyRageDamage2.png");

    // --- INIMIGO 2 ('M') ---
    a.texEnemies[2] = carregaTextura("assets/enemies/enemy3.png");
    a.texEnemiesRage[2] = carregaTextura("assets/enemies/enemyRage3.png");
    a.texEnemiesDamage[2] = carregaTextura("assets/enemies/enemyRageDamage3.png");

    // --- INIMIGO 3 ('K') ---
    a.texEnemies[3] = carregaTextura("assets/enemies/enemy4.png");
    a.texEnemiesRage[3] = carregaTextura("assets/enemies/enemyRage4.png");
    a.texEnemiesDamage[3] = carregaTextura("assets/enemies/enemyRageDamage4.png");

    // --- INIMIGO 4 ('G') ---
    a.texEnemies[4] = carregaTextura("assets/enemies/enemy5.png");
    a.texEnemiesRage[4] = carregaTextura("assets/enemies/enemyRage5.png");
    a.texEnemiesDamage[4] = carregaTextura("assets/enemies/enemyRageDamage5.png");

    a.texHealthOverlay = carregaTextura("assets/heal.png");
    a.texLinternOn = carregaTextura("assets/linternOn.png");
    a.texLinternOff = carregaTextura("assets/linternOff.png");
    a.texDamage = carregaTextura("assets/damage.png");

    a.texHealth = carregaTextura("assets/health.png");
    a.texBattery0 = carregaTextura("assets/items/battery0.png");
    a.texBattery25 = carregaTextura("assets/items/battery25.png");
    a.texBattery50 = carregaTextura("assets/items/battery50.png");
    a.texBattery75 = carregaTextura("assets/items/battery75.png");
    a.texBattery100 = carregaTextura("assets/items/battery100.png");

    if (!a.texBattery100) a.texBattery100 = a.texHealth;
    if (!a.texBattery0) a.texBattery0 = a.texBattery100;
    if (!a.texBattery25) a.texBattery25 = a.texBattery100;
    if (!a.texBattery50) a.texBattery50 = a.texBattery100;
    if (!a.texBattery75) a.texBattery75 = a.texBattery100;

    a.texKey[0] = carregaTextura("assets/items/Key0_Icon.png");
    a.texKey[1] = carregaTextura("assets/items/Key1_Icon.png");
    a.texKey[2] = carregaTextura("assets/items/Key2_Icon.png");

    if (!a.texChao || !a.texParede || !a.texSangue || !a.progSangue ||
        !a.texHealth || !a.texLinternOn || !a.texLinternOff ||
        !a.texDamage || !a.texHealthOverlay || !a.texEnemies[0] ||
        !a.texEnemiesRage[0] || !a.texEnemiesDamage[0] || !a.texEnemies[1] ||
        !a.texEnemiesRage[1] || !a.texEnemiesDamage[1] || !a.texEnemies[2] ||
        !a.texEnemiesRage[2] || !a.texEnemiesDamage[2] || !a.texMenuBG)
    {
        std::printf("ERRO: falha ao carregar algum asset (textura/shader).\n");
        return false;
    }
    return true;
}
