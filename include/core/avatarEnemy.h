#pragma once
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include "core/entities.h"

// Estrutura de malha 3D do avatar
struct AvatarMesh
{
    std::vector<float>        positions;
    std::vector<unsigned int> indices;
    int index_count = 0;
};

// Modelo 3D de avatar carregado (compartilhado entre instâncias)
struct AvatarModel
{
    std::vector<AvatarMesh> meshes;
    bool loaded = false;
    
    bool load(const char* path);
    void clear();
};

// Instância de um inimigo avatar no mundo
struct AvatarEnemyInstance
{
    float x, z;
    float rot_y;
    float hurtTimer;
    bool active;
    
    AvatarEnemyInstance() : x(0), z(0), rot_y(0), hurtTimer(0), active(true) {}
};

// Sistema global de avatar enemies
namespace AvatarSystem {
    extern AvatarModel g_avatarModel;
    extern AvatarModel g_avatarModels[3];

    bool loadModel(const char* path);
    bool loadModelForType(EnemyType t, const char* path);
    void clearModel();
    void clearModels();
    void renderInstance(const AvatarEnemyInstance& inst);
    void renderInstance(const AvatarEnemyInstance& inst, EnemyType type);
    const AvatarModel* getModelForType(EnemyType t);
    float lookAtRotation(float fromX, float fromZ, float toX, float toZ);
}
