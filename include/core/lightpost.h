#pragma once

struct LightPost {
    float x, z;      // Posição mundo (centro do tile)
    bool active;     // Se está aceso agora
    float intensity; // 0.0 = apagado, 1.0 = máximo brilho
};
