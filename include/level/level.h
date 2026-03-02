#pragma once
#include "level/maploader.h"
#include "level/levelmetrics.h"
#include "core/entities.h"
#include "core/lightpost.h"
#include <vector>

struct Level
{
    MapLoader map;
    LevelMetrics metrics;
    std::vector<Enemy> enemies;
    std::vector<Item> items;
    std::vector<LightPost> posts;

    // Exit door position
    bool hasDoor = false;
    float doorX = 0.0f, doorZ = 0.0f;

    // Objetivo local do mapa atual (baterias)
    int batteriesRequiredInMap = 0;
    int batteriesCollectedInMap = 0;

    // Current level (1-3)
    int currentLevel = 1;

    // Progresso de baterias no mapa atual
    int batteriesCollectedInMap = 0;
    int batteriesRequiredInMap = 0;
};

bool loadLevel(Level &lvl, const char *mapPath, float tileSize);
void applySpawn(const Level &lvl, float &camX, float &camZ);
