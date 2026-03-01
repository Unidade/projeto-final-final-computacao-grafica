#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    a.texMenuBG = carregaTextura("assets/menu_bg.png");

    a.texChao = carregaTextura("assets/150.png");
    a.texParede = carregaTextura("assets/190.png");
    a.texSangue = carregaTextura("assets/016.png");
    a.texLava = carregaTextura("assets/179.png");
    a.texChaoInterno = carregaTextura("assets/185.png");
    a.texParedeInterna = carregaTextura("assets/115.png");
    a.texTeto = carregaTextura("assets/082.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");

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
    a.texGunDefault = a.texLinternOn;
    a.texGunFire1 = a.texLinternOn;
    a.texGunFire2 = a.texLinternOn;
    a.texGunReload1 = a.texLinternOff;
    a.texGunReload2 = a.texLinternOff;
    a.texDamage = carregaTextura("assets/damage.png");

    a.texHealth = carregaTextura("assets/health.png");
    a.texAmmo = carregaTextura("assets/066.png");

    a.texSkydome = carregaTextura("assets/menu_bg.png");

    a.texGunHUD = a.texLinternOn;
    a.texHudFundo = carregaTextura("assets/088.png");

    if (!a.texChao || !a.texParede || !a.texSangue || !a.texLava || !a.progSangue ||
        !a.progLava || !a.texHealth || !a.texLinternOn || !a.texLinternOff || !a.texGunDefault || !a.texGunFire1 ||
        !a.texGunFire2 || !a.texSkydome || !a.texGunReload1 || !a.texGunReload2 ||
        !a.texDamage || !a.texAmmo || !a.texHealthOverlay || !a.texEnemies[0] ||
        !a.texEnemiesRage[0] || !a.texEnemiesDamage[0] || !a.texEnemies[1] ||
        !a.texEnemiesRage[1] || !a.texEnemiesDamage[1] || !a.texEnemies[2] ||
        !a.texEnemiesRage[2] || !a.texEnemiesDamage[2] || !a.texGunHUD || !a.texHudFundo || !a.texMenuBG)
    {
        std::printf("ERRO: falha ao carregar algum asset (textura/shader).\n");
        return false;
    }
    return true;
}
