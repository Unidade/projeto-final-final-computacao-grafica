#version 120

varying vec2  vTexCoord;
varying float vWorldX;
varying float vWorldZ;

void main()
{
    vTexCoord = gl_MultiTexCoord0.xy;

    // Salva a posição XZ em world space para o fragment shader.
    // Como o chão é desenhado diretamente em world space (sem model matrix),
    // gl_Vertex.xz == coordenadas do mundo.
    vWorldX = gl_Vertex.x;
    vWorldZ = gl_Vertex.z;

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
