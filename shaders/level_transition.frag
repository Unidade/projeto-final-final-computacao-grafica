#version 120

uniform float uTime;
uniform float uProgress;    // 0.0 (início) to 1.0 (completamente preto)
uniform vec2  uResolution;

// Pseudo-falso random para glio/ruído
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = gl_TexCoord[0].st;
    
    // Calcula dist. do centro (vignette ovalado)
    vec2 center = uv - 0.5;
    center.x *= uResolution.x / uResolution.y;
    float dist = length(center);
    
    // Adiciona ruído estático na medição de distância
    float noise = rand(uv + uTime * 0.5);
    
    // Threshold diminui de 1.5 (afastado) até -0.5 (completamente fechado)
    float threshold = 1.2 - (uProgress * 1.8);
    
    // Valor interpolado de "escuridão", onde threshold + noise define a borda
    float edge = dist + noise * 0.25;
    float alpha = smoothstep(threshold - 0.1, threshold + 0.15, edge);
    
    // Se o uProgress for muito alto, garante que a tela fique 100% preta
    if (uProgress > 0.95) {
        alpha = 1.0;
    }
    
    gl_FragColor = vec4(0.0, 0.0, 0.0, alpha);
}
