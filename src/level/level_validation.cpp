#include "level/level_validation.h"
#include "level/maploader.h"
#include <vector>
#include <queue>
#include <cstdio>

// Retorna true se o caractere 'c' é passável para fins de validação BFS.
// Deve espelhar exatamente as regras de movement.cpp (isWallTile):
//   '1' = parede externa sólida — bloqueante
//   '2' = parede fina indoor   — bloqueante
// Tudo mais ('0', '3', entidades, 'D', etc.) é passável.
static bool isPassable(char c)
{
    return (c != '1' && c != '2');
}

// BFS a partir da posição de spawn (tile '9').
// Retorna um grid 2D de células visitadas.
static std::vector<std::vector<bool>> bfsReachable(const MapLoader& map)
{
    int H = map.getHeight();
    const auto& data = map.data();

    // Encontra spawn
    int spawnRow = -1, spawnCol = -1;
    for (int z = 0; z < H && spawnRow < 0; z++)
    {
        const std::string& row = data[z];
        for (int x = 0; x < (int)row.size(); x++)
        {
            if (row[x] == '9') { spawnRow = z; spawnCol = x; break; }
        }
    }

    // Monta grid de visitados
    std::vector<std::vector<bool>> visited(H);
    for (int z = 0; z < H; z++)
        visited[z].assign(data[z].size(), false);

    if (spawnRow < 0)
    {
        std::printf("[ValidLevel] AVISO: tile de spawn '9' nao encontrado!\n");
        return visited;
    }

    // BFS 4-direcional
    std::queue<std::pair<int,int>> q;
    q.push({spawnRow, spawnCol});
    visited[spawnRow][spawnCol] = true;

    const int dz[] = {-1, 1, 0, 0};
    const int dx[] = { 0, 0,-1, 1};

    while (!q.empty())
    {
        auto [cz, cx] = q.front(); q.pop();
        for (int d = 0; d < 4; d++)
        {
            int nz = cz + dz[d];
            int nx = cx + dx[d];
            if (nz < 0 || nz >= H) continue;
            if (nx < 0 || nx >= (int)data[nz].size()) continue;
            if (visited[nz][nx]) continue;
            if (!isPassable(data[nz][nx])) continue;
            visited[nz][nx] = true;
            q.push({nz, nx});
        }
    }

    return visited;
}

bool validateLevel(const Level& lvl, int levelNumber)
{
    const auto& map = lvl.map;
    int H = map.getHeight();
    const auto& data = map.data();

    std::printf("[ValidLevel] Validando Level %d...\n", levelNumber);

    auto visited = bfsReachable(map);

    bool allOk = true;

    // Verifica cada tile de interesse
    for (int z = 0; z < H; z++)
    {
        const std::string& row = data[z];
        for (int x = 0; x < (int)row.size(); x++)
        {
            char c = row[x];
            const char* label = nullptr;

            if      (c == 'V') label = "Bateria (V)";
            else if (c == 'Y') label = "Chave (Y)";
            else if (c == 'D') label = "Porta (D)";
            else if (c == 'H') label = "Kit Saude (H)";

            if (!label) continue;

            bool reach = (z < H && x < (int)visited[z].size()) && visited[z][x];
            if (reach)
            {
                std::printf("[ValidLevel]   OK  %s em (%d,%d)\n", label, x, z);
            }
            else
            {
                std::printf("[ValidLevel] AVISO: %s em (%d,%d) INATINGIVEL a partir do spawn!\n",
                            label, x, z);
                allOk = false;
            }
        }
    }

    if (allOk)
        std::printf("[ValidLevel] Level %d: todos os elementos sao alcancaveis. ✓\n", levelNumber);
    else
        std::printf("[ValidLevel] Level %d: EXISTEM ELEMENTOS INACESSIVEIS! Verifique acima.\n", levelNumber);

    return allOk;
}
