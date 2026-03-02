#version 120

uniform sampler2D uTexture;
uniform vec2      uLightPos;    // (x, z) do poste em world space
uniform float     uConeRadius;  // raio do cone na altura do chão
uniform float     uIntensity;   // intensidade atual do poste (0.0 – 1.0)

varying vec2  vTexCoord;
varying float vWorldX;
varying float vWorldZ;

void main()
{
    vec4 texColor = texture2D(uTexture, vTexCoord);

    // Distância radial do fragmento ao eixo do poste (plano XZ)
    float dx   = vWorldX - uLightPos.x;
    float dz   = vWorldZ - uLightPos.y; // uLightPos é vec2(worldX, worldZ)
    float dist = sqrt(dx * dx + dz * dz);

    // t: 0.0 no centro, 1.0 na borda, >1.0 fora do cone
    float t = dist / max(uConeRadius, 0.001);

    // Ambient escuro que replica o GL_LIGHT_MODEL_AMBIENT do pipeline fixo
    // Mantemos o chão quase preto fora da luz, igual às paredes e demais objetos
    float baseDark = 0.06;

    vec3 finalColor;

    if (t < 1.0)
    {
        // smoothstep: 1.0 no centro, 0.0 na borda — curva suave
        float falloff = smoothstep(1.0, 0.0, t);
        // Quadrático: decaimento mais acentuado perto da borda
        falloff = falloff * falloff;

        // Cor quente da lâmpada: branco-amarelada
        vec3 lampColor = vec3(1.05, 0.92, 0.58);

        // Base escura + contribuição da lâmpada
        float lampStrength = uIntensity * falloff * 1.6;
        finalColor = texColor.rgb * (baseDark + lampColor * lampStrength);
    }
    else
    {
        // Fora do cone: apenas o ambient escuro — combina com o resto da cena
        finalColor = texColor.rgb * baseDark;
    }

    // Clamp para não estourar o branco
    finalColor = clamp(finalColor, 0.0, 1.0);

    gl_FragColor = vec4(finalColor, texColor.a);
}
