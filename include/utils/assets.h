#pragma once
#include <GL/glew.h>

struct GameAssets
{
    // texturas
    GLuint texMenuBG = 0;
    GLuint texGameOver = 0;
    GLuint texChao = 0;
    GLuint texParede = 0;
    GLuint texSangue = 0;
    GLuint texDoor = 0;
    GLuint texChaoInterno = 0;
    GLuint texParedeInterna = 0;
    GLuint texTeto = 0;
    GLuint texEnemy = 0;
    GLuint texEnemyRage = 0;    // Viu o player
    GLuint texEnemyDamage = 0;  // Leva dano
    GLuint texHealthOverlay = 0; // Tela de cura
    GLuint texHealth = 0;
    GLuint texBattery = 0;
    GLuint texKey[3] = {0, 0, 0}; // Key0=level1, Key1=level2, Key2=level3
    GLuint texLinternOn = 0;
    GLuint texLinternOff = 0;
    GLuint texDamage = 0;

    GLuint texEnemies[5]       = {0, 0, 0, 0, 0};
    GLuint texEnemiesRage[5]   = {0, 0, 0, 0, 0};
    GLuint texEnemiesDamage[5] = {0, 0, 0, 0, 0};

    // shaders
    GLuint progSangue = 0;
};

bool loadAssets(GameAssets &a);
