#pragma once
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>

// Estrutura de malha 3D do avatar
struct AvatarMesh
{
    std::vector<float> positions; // x,y,z per vertex
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
    float x, z;           // Posição no mundo (xz)
    float rot_y;          // Rotação em Y (graus)
    float hurtTimer;      // Timer de dano (para efeito visual)
    bool active;          // Se está ativo/vivo
    
    AvatarEnemyInstance() : x(0), z(0), rot_y(0), hurtTimer(0), active(true) {}
};

// Sistema global de avatar enemies
namespace AvatarSystem {
    // Modelo global do inimigo (carregado uma vez)
    extern AvatarModel g_avatarModel;
    
    // Carrega o modelo do disco (chamar uma vez no init)
    bool loadModel(const char* path);
    void clearModel();
    
    // Renderiza uma instância do avatar
    void renderInstance(const AvatarEnemyInstance& inst);
    
    // Calcula a rotação para olhar para um ponto
    float lookAtRotation(float fromX, float fromZ, float toX, float toZ);
}
