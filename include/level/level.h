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

    // Current level (1-3)
    int currentLevel = 1;
};

bool loadLevel(Level &lvl, const char *mapPath, float tileSize);
void applySpawn(const Level &lvl, float &camX, float &camZ);
