#include "level/level.h"
#include "core/config.h"
#include "core/lightpost.h"
#include "core/avatarEnemy.h"
#include <cstdio>

// Configurações básicas para spawn
static const float ENEMY_START_HP = 100.0f;

bool loadLevel(Level &lvl, const char *mapPath, float tileSize)
{
    // 1. Carrega o mapa de texto (paredes, chão)
    if (!lvl.map.load(mapPath))
    {
        std::printf("ERRO: nao foi possivel carregar o mapa: %s\n", mapPath);
        return false;
    }

    lvl.metrics = LevelMetrics::fromMap(lvl.map, tileSize);
    
    // Limpa entidades antigas e estado local do mapa se houver
    lvl.enemies.clear();
    lvl.items.clear();
    lvl.posts.clear();
    lvl.hasDoor = false;
    lvl.doorOpen = false; // porta começa fechada
    lvl.batteriesCollectedInMap = 0;
    lvl.batteriesRequiredInMap = 0;
    lvl.spawnX = 0.0f;
    lvl.spawnZ = 0.0f;

    // 2. Escaneia o mapa procurando Entidades (E, H, etc)
    // Precisamos acessar os dados brutos do MapLoader como referência mudável
    // Para isso, vamos assumir que podemos modificar o mapData ou fazemos uma varredura
    // Como o MapLoader::data() retorna const, vamos ter que ser criativos ou 
    // alterar o MapLoader. Mas vamos fazer a logica de posicao aqui.
    
    // Nota: O ideal seria o MapLoader permitir edição, mas vamos iterar cópia
    // Na verdade, o MapLoader guarda strings. Vamos trapacear um pouco:
    // O MapLoader não expõe escrita. Então vamos ler, criar a entidade, e na hora
    // de desenhar/colidir, tratamos E e H como chão (0).
    
    const auto& data = lvl.map.data();
    int H = lvl.map.getHeight();

    for(int z = 0; z < H; z++)
    {
        const std::string& row = data[z];
        for(int x = 0; x < (int)row.size(); x++)
        {
            char c = row[x];
            float wx, wz;
            lvl.metrics.tileCenter(x, z, wx, wz);

            // --- Lógica para inimigos (todos são 3D Avatar GLB) ---
            int enemyType = -1; // -1 significa "não é inimigo"

            if (c == 'J') enemyType = 0;      // Inimigo Tipo 1
            else if (c == 'T') enemyType = 1; // Inimigo Tipo 2 
            else if (c == 'M') enemyType = 2; // Inimigo Tipo 3 
            else if (c == 'G') enemyType = 3; // Inimigo Tipo 4
            else if (c == 'K') enemyType = 4; // Inimigo Tipo 5

            if (enemyType != -1) // Se achou qualquer um dos inimigos
            {
                Enemy e;
                e.type = enemyType;
                e.isAvatar = true;  // TODOS os inimigos são avatar 3D

                e.x = wx;
                e.z = wz;

                // Salva a posição inicial e zera timer
                e.startX = wx; 
                e.startZ = wz;
                e.respawnTimer = 0.0f;

                e.hp = ENEMY_START_HP;
                e.state = STATE_IDLE;
                e.animFrame = 0;
                e.animTimer = 0;
                e.hurtTimer = 0.0f;
                e.attackCooldown = 0.0f;

                lvl.enemies.push_back(e);
            }
            // ----------------------------------------------------------------

            else if (c == 'H') // Health Kit
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_HEALTH;
                i.active = true;
                i.respawnTimer = 0.0f;
                lvl.items.push_back(i);
            }
            else if (c == 'V') // Bateria (Luzes Apagadas)
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_BATTERY;
                i.active = true;
                i.respawnTimer = 0.0f;
                lvl.items.push_back(i);
                lvl.batteriesRequiredInMap++; // conta UMA vez por tile V
            }
            else if (c == 'Y') // Chave do nivel (Luzes Apagadas)
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_KEY;
                i.active = true;
                i.respawnTimer = 0.0f;
                i.keyLevel = lvl.currentLevel;
                lvl.items.push_back(i);
            }
            else if (c == 'P') // Poste de Luz
            {
                float dx, dz, distSq;
                bool tooClose = false;
                for (const auto& p : lvl.posts)
                {
                    dx = wx - p.x;
                    dz = wz - p.z;
                    distSq = dx * dx + dz * dz;
                    if (distSq < GameConfig::MIN_POST_DISTANCE * GameConfig::MIN_POST_DISTANCE)
                    {
                        tooClose = true;
                        break;
                    }
                }
                if (!tooClose)
                {
                    LightPost lp;
                    lp.x = wx;
                    lp.z = wz;
                    lp.active    = true;
                    lp.intensity = 1.0f;
                    lvl.posts.push_back(lp);
                }
            }
            else if (c == 'D') // Exit Door
            {
                lvl.hasDoor = true;
                lvl.doorX = wx;
                lvl.doorZ = wz;
            }
            else if (c == '9') // Spawn do jogador — salva para SafePost zone
            {
                lvl.spawnX = wx;
                lvl.spawnZ = wz;
            }
        }
    }

    return true;
}

void applySpawn(const Level &lvl, float &camX, float &camZ)
{
    lvl.metrics.spawnPos(lvl.map, camX, camZ);
}