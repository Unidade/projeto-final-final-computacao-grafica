#pragma once
#include "level/level.h"

// Valida se todos os pickups e porta são alcançáveis a partir do spawn ('9').
// Faz um BFS/flood-fill pelo mapa e loga avisos para qualquer elemento
// inacessível. Retorna true se tudo for alcançável, false se houver problemas.
// Útil somente em tempo de desenvolvimento — não bloqueia o carregamento.
bool validateLevel(const Level& lvl, int levelNumber);
