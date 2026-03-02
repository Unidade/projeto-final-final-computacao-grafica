#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "core/game_state.h"
#include "core/avatarEnemy.h"
#include "graphics/drawlevel.h"
#include "level/levelmetrics.h"
#include "utils/utils.h"
#include <cstdio>

// =====================
// CONFIG / CONSTANTES
// =====================

// Config do grid
static const float TILE = 4.0f;           // tamanho do tile no mundo
static const float WALL_THICKNESS = 1.0f; // 1.0 = walls fill full tile (no gaps)
static const float CEILING_H = 10.0f;     // teto alto
static const float WALL_H = 12.0f;        // paredes altas
static const float EPS_Y = 0.001f;        // evita z-fighting

static const GLfloat kAmbientOutdoor[] = {0.06f, 0.06f, 0.08f, 1.0f}; // base uniforme
static const GLfloat kAmbientIndoor[] = {0.05f, 0.05f, 0.07f, 1.0f};  // interior uniforme

// ======================
// CONFIG ÚNICA DO CULLING (XZ)
// ======================
static float gCullHFovDeg = 210.0f;     // FOV amplo: evitar paredes sumindo nas bordas
static float gCullNearTiles = 3.0f;     // perto não faz culling angular
static float gCullMaxDistTiles = 35.0f; // render mais longe — corrige paredes piscando




// Retorna TRUE se deve renderizar o objeto no plano XZ (distância + cone de FOV)
// - Usa as configs globais gCull*
// - Usa forward já normalizado (fwdx,fwdz) e flag hasFwd
static inline bool isVisibleXZ(float objX, float objZ,
                               float camX, float camZ,
                               bool hasFwd, float fwdx, float fwdz)
{
    float vx = objX - camX;
    float vz = objZ - camZ;
    float distSq = vx * vx + vz * vz;

    // 0) Distância máxima (se habilitada)
    if (gCullMaxDistTiles > 0.0f)
    {
        float maxDist = gCullMaxDistTiles * TILE;
        float maxDistSq = maxDist * maxDist;
        if (distSq > maxDistSq)
            return false;
    }

    // 1) Dentro do near: não faz culling angular
    float nearDist = gCullNearTiles * TILE;
    float nearDistSq = nearDist * nearDist;
    if (distSq <= nearDistSq)
        return true;

    // 2) Sem forward válido: não faz culling angular
    if (!hasFwd)
        return true;

    // 3) Cone por FOV horizontal
    float cosHalf = std::cos(deg2rad(gCullHFovDeg * 0.5f));

    float invDist = 1.0f / std::sqrt(distSq);
    float nvx = vx * invDist;
    float nvz = vz * invDist;

    float dot = clampf(nvx * fwdx + nvz * fwdz, -1.0f, 1.0f);
    return dot >= cosHalf;
}

static void bindTexture0(GLuint tex)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
}

static float hash01(float x)
{
    float s = sinf(x * 12.9898f) * 43758.5453f;
    return s - floorf(s);
}

static float flickerFluorescente(float t)
{
    (void)t;
    return 1.0f; // No flicker: uniform lighting
}

static void setIndoorLampAt(float x, float z, float intensity)
{
    GLfloat pos[] = {x, CEILING_H - 0.05f, z, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    GLfloat diff[] = {
        1.20f * intensity,
        1.22f * intensity,
        1.28f * intensity,
        1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff);

    GLfloat amb[] = {
        1.10f * intensity,
        1.10f * intensity,
        1.12f * intensity,
        1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
}

static void beginIndoor(float wx, float wz, float time)
{
    glDisable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientIndoor);

    glEnable(GL_LIGHT1);

    float f = flickerFluorescente(time);
    float intensity = 1.2f * f;

    setIndoorLampAt(wx, wz, intensity);
}

static void endIndoor()
{
    glDisable(GL_LIGHT1);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
    glEnable(GL_LIGHT0);
}

static void desenhaQuadTeto(float x, float z, float tile, float tilesUV, GLuint texTeto)
{
    float half = tile * 0.5f;

    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, texTeto);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f); glVertex3f(x - half, CEILING_H, z - half);
    glTexCoord2f(tilesUV, 0.0f); glVertex3f(x + half, CEILING_H, z - half);
    glTexCoord2f(tilesUV, tilesUV); glVertex3f(x + half, CEILING_H, z + half);
    glTexCoord2f(0.0f, tilesUV); glVertex3f(x - half, CEILING_H, z + half);
    glEnd();
}

// Desenha o chão subdividido em NxN para que o per-vertex lighting
// mostre spotlights com borda nítida. Sem subdivisão, apenas 4 vértices
// por tile tornam qualquer spotlight invisível.
static void desenhaQuadChao(float x, float z, float tile, float tilesUV)
{
    const int N = 8; // subdivisões por lado (81 vértices por tile)
    float half = tile * 0.5f;
    float step = tile / N;
    float uvStp = tilesUV / N;

    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int zi = 0; zi < N; zi++)
    {
        for (int xi = 0; xi < N; xi++)
        {
            float x0 = (x - half) + xi * step;
            float x1 = (x - half) + (xi + 1) * step;
            float z0 = (z - half) + zi * step;
            float z1 = (z - half) + (zi + 1) * step;

            float u0 = xi * uvStp;
            float u1 = (xi + 1) * uvStp;
            float v0 = zi * uvStp;
            float v1 = (zi + 1) * uvStp;

            glBegin(GL_QUADS);
            glTexCoord2f(u0, v1);
            glVertex3f(x0, EPS_Y, z1);
            glTexCoord2f(u1, v1);
            glVertex3f(x1, EPS_Y, z1);
            glTexCoord2f(u1, v0);
            glVertex3f(x1, EPS_Y, z0);
            glTexCoord2f(u0, v0);
            glVertex3f(x0, EPS_Y, z0);
            glEnd();
        }
    }
}

static void desenhaTileChao(float x, float z, GLuint texChaoX, bool /*temTeto*/, GLuint texTeto)
{
    glUseProgram(0);

    glColor3f(1, 1, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texChaoX);

    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
    desenhaQuadTeto(x, z, TILE, 2.0f, texTeto);
}

// --- Desenha parede FACE POR FACE (paredes finas) ---
static void desenhaParedePorFace(float x, float z, GLuint texParedeX, int f)
{
    float half = TILE * 0.5f * WALL_THICKNESS;

    glUseProgram(0);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texParedeX);

    float tilesX = 1.0f;
    float tilesY = 2.0f;

    glBegin(GL_QUADS);

    switch (f)
    {
    case 0: // z+ (Frente)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x - half, 0.0f, z + half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x + half, 0.0f, z + half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x + half, WALL_H, z + half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x - half, WALL_H, z + half);
        break;

    case 1: // z- (Trás)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x + half, 0.0f, z - half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x - half, 0.0f, z - half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x - half, WALL_H, z - half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x + half, WALL_H, z - half);
        break;

    case 2: // x+ (Direita)
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x + half, 0.0f, z + half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x + half, 0.0f, z - half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x + half, WALL_H, z - half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x + half, WALL_H, z + half);
        break;

    case 3: // x- (Esquerda)
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x - half, 0.0f, z - half);
        glTexCoord2f(tilesX, 0.0f);
        glVertex3f(x - half, 0.0f, z + half);
        glTexCoord2f(tilesX, tilesY);
        glVertex3f(x - half, WALL_H, z + half);
        glTexCoord2f(0.0f, tilesY);
        glVertex3f(x - half, WALL_H, z - half);
        break;
    }
    glEnd();
}

// Wrapper para desenhar o cubo todo (parede outdoor, fina)
static void desenhaParedeCuboCompleto(float x, float z, GLuint texParedeX)
{
    desenhaParedePorFace(x, z, texParedeX, 0);
    desenhaParedePorFace(x, z, texParedeX, 1);
    desenhaParedePorFace(x, z, texParedeX, 2);
    desenhaParedePorFace(x, z, texParedeX, 3);

    float half = TILE * 0.5f * WALL_THICKNESS;
    glBindTexture(GL_TEXTURE_2D, texParedeX);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, WALL_H, z + half);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + half, WALL_H, z + half);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + half, WALL_H, z - half);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x - half, WALL_H, z - half);
    glEnd();
}

// Desenha a porta de saída — fechada: quad vertical na face norte do tile (contra a parede sul);
// aberta: apenas chão.
static void desenhaTileDoor(float x, float z, const RenderAssets &r, bool doorOpen)
{
    // Chão sob a porta (sempre presente)
    desenhaTileChao(x, z, r.texChao, false, r.texTeto);

    if (doorOpen)
        return; // porta aberta: só o chão, sem obstrução

    // Porta fechada: quad vertical na face NORTE do tile (face voltada para o jogador
    // que se aproxima pelo interior do mapa).
    float half    = TILE * 0.5f;
    float doorW   = TILE;          // ocupa toda a largura do tile
    float doorH   = WALL_H * 0.85f;
    float hw      = doorW * 0.5f;
    float faceZ   = z - half;      // face norte = limite sul da célula interior vizinha

    glUseProgram(0);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, r.texDoor);

    // Visível dos dois lados (jogador pode ver de perto)
    glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 1); glVertex3f(x - hw, 0.0f, faceZ);
    glTexCoord2f(1, 1); glVertex3f(x + hw, 0.0f, faceZ);
    glTexCoord2f(1, 0); glVertex3f(x + hw, doorH, faceZ);
    glTexCoord2f(0, 0); glVertex3f(x - hw, doorH, faceZ);
    glEnd();
    glEnable(GL_CULL_FACE);
}


static void desenhaTileSangue(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progSangue);

    GLint locTime = glGetUniformLocation(r.progSangue, "uTime");
    GLint locStr = glGetUniformLocation(r.progSangue, "uStrength");
    GLint locSpd = glGetUniformLocation(r.progSangue, "uSpeed");
    GLint locTex = glGetUniformLocation(r.progSangue, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locSpd, 2.0f, 1.3f);

    bindTexture0(r.texSangue);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
    desenhaQuadTeto(x, z, TILE, 2.0f, r.texTeto);
}

// --- Checa vizinhos ---
static char getTileAt(const MapLoader &map, int tx, int tz)
{
    const auto &data = map.data();
    const int H = map.getHeight();

    if (tz < 0 || tz >= H)
        return '0';
    if (tx < 0 || tx >= (int)data[tz].size())
        return '0';

    return data[tz][tx];
}

static void drawFace(float wx, float wz, int face, char neighbor, GLuint texParedeInternaX, float time)
{
    bool outside = (neighbor != '1' && neighbor != '2' && neighbor != '3');

    if (outside)
    {
        glDisable(GL_LIGHT1);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
        glEnable(GL_LIGHT0);

        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
    }
    else if (neighbor != '2')
    {
        beginIndoor(wx, wz, time);
        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
        endIndoor();
    }
}

void drawLevel(const MapLoader &map, float px, float pz, float dx, float dz, const RenderAssets &r, float time, bool doorOpen)
{
    const auto &data = map.data();
    const int H = map.getHeight();

    LevelMetrics m = LevelMetrics::fromMap(map, TILE);

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    for (int z = 0; z < H; z++)
    {
        for (int x = 0; x < (int)data[z].size(); x++)
        {
            float wx, wz;
            m.tileCenter(x, z, wx, wz);

            // CULLING ÚNICO (cenário)
            if (!isVisibleXZ(wx, wz, px, pz, hasFwd, fwdx, fwdz))
                continue;

            char c = data[z][x];

            // 'D' é tratado separadamente — não inclua aqui para que o bloco
            // else if (c == 'D') abaixo seja alcançado e a porta seja desenhada.
            bool isEntity = (c == 'J' || c == 'T' || c == 'M' || c == 'K' ||
                             c == 'G' || c == 'H' || c == 'V' || c == 'Y' ||
                             c == 'E' || c == 'F' || c == 'I' || c == 'P');

            if (isEntity)
            {
                char viz1 = getTileAt(map, x + 1, z);
                char viz2 = getTileAt(map, x - 1, z);
                char viz3 = getTileAt(map, x, z + 1);
                char viz4 = getTileAt(map, x, z - 1);

                bool isIndoor = (viz1 == '3' || viz1 == '2' ||
                                 viz2 == '3' || viz2 == '2' ||
                                 viz3 == '3' || viz3 == '2' ||
                                 viz4 == '3' || viz4 == '2');

                if (isIndoor)
                {
                    beginIndoor(wx, wz, time);
                    desenhaTileChao(wx, wz, r.texChaoInterno, true, r.texTeto);
                    endIndoor();
                }
                else
                {
                    desenhaTileChao(wx, wz, r.texChao, false, r.texTeto);
                }
            }
            else if (c == '0')
            {
                desenhaTileChao(wx, wz, r.texChao, false, r.texTeto);
            }
            else if (c == '3')
            {
                beginIndoor(wx, wz, time);
                desenhaTileChao(wx, wz, r.texChaoInterno, true, r.texTeto);
                endIndoor();
            }
            else if (c == '1')
            {
                desenhaParedeCuboCompleto(wx, wz, r.texParede);
            }
            else if (c == '2')
            {
                char vizFrente = getTileAt(map, x, z + 1);
                char vizTras = getTileAt(map, x, z - 1);
                char vizDireita = getTileAt(map, x + 1, z);
                char vizEsq = getTileAt(map, x - 1, z);

                drawFace(wx, wz, 0, vizFrente, r.texParedeInterna, time);
                drawFace(wx, wz, 1, vizTras, r.texParedeInterna, time);
                drawFace(wx, wz, 2, vizDireita, r.texParedeInterna, time);
                drawFace(wx, wz, 3, vizEsq, r.texParedeInterna, time);
            }
            else if (c == 'D')
            {
                desenhaTileDoor(wx, wz, r, doorOpen);
            }
            else if (c == 'B')
            {
                desenhaTileSangue(wx, wz, r, time);
            }
        }
    }
}

static void drawSprite(float x, float z, float w, float h, GLuint tex, float camX, float camZ)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1, 1, 1);

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    float ddx = camX - x;
    float ddz = camZ - z;
    float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;

    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    float hw = w * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(hw, h, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-hw, h, 0.0f);
    glEnd();

    glPopMatrix();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}

// Desenha inimigos e itens
void drawEntities(const std::vector<Enemy> &enemies, const std::vector<Item> &items,
                  float camX, float camZ, float dx, float dz, const RenderAssets &r)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    // --- ITENS ---
    for (const auto &item : items)
    {
        if (!item.active)
            continue;

        if (!isVisibleXZ(item.x, item.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        if (item.type == ITEM_HEALTH)
            drawSprite(item.x, item.z, 0.7f, 0.7f, r.texHealth, camX, camZ);
        else if (item.type == ITEM_BATTERY)
            drawSprite(item.x, item.z, 0.7f, 0.7f, r.texBattery, camX, camZ);
        else if (item.type == ITEM_KEY)
        {
            int kl = (item.keyLevel >= 1 && item.keyLevel <= 3) ? (item.keyLevel - 1) : 0;
            GLuint texK = r.texKey[kl];
            if (texK) drawSprite(item.x, item.z, 0.7f, 0.7f, texK, camX, camZ);
        }
    }

    // --- INIMIGOS (todos são 3D Avatar) ---
    for (const auto &en : enemies)
    {
        if (en.state == STATE_DEAD)
            continue;

        if (!isVisibleXZ(en.x, en.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        // Todos os inimigos são renderizados como Avatar 3D (GLB)
        AvatarEnemyInstance inst;
        inst.x = en.x;
        inst.z = en.z;
        inst.hurtTimer = en.hurtTimer;
        inst.active = true;

        inst.rot_y = AvatarSystem::lookAtRotation(en.x, en.z, camX, camZ);

        AvatarSystem::renderInstance(inst);
    }

    glEnable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
}

// ---------------------------------------------------------------------------
// Desenha postes de luz como pilar + esfera no topo
// ---------------------------------------------------------------------------
void drawLightPosts(const std::vector<LightPost> &posts,
                    float camX, float camZ, float dx, float dz)
{
    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    // aperção do cone visual: deve bater com GL_SPOT_CUTOFF
    const float CONE_DEG = 55.0f;
    const float PI = 3.14159265f;
    const float coneRad = tanf(CONE_DEG * PI / 180.0f) * CEILING_H;
    const int CONE_SEGS = 24;

    for (const auto &p : posts)
    {
        if (!isVisibleXZ(p.x, p.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        glPushMatrix();
        glTranslatef(p.x, 0.0f, p.z);

        // -------- CONE DE LUZ SEMITRANSPARENTE --------
        if (p.active && p.intensity > 0.05f)
        {
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);

            float alpha = 0.13f * p.intensity;
            float apex  = CEILING_H - 0.15f;

            // -- Cone lateral (do teto ao chão) --
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(1.0f, 0.96f, 0.65f, alpha * 2.2f); // ápice mais brilhante
            glVertex3f(0.0f, apex, 0.0f);
            for (int i = 0; i <= CONE_SEGS; i++)
            {
                float ang = i * 2.0f * PI / CONE_SEGS;
                float bx  = cosf(ang) * coneRad;
                float bz  = sinf(ang) * coneRad;
                glColor4f(1.0f, 0.95f, 0.5f, 0.0f); // borda totalmente transparente
                glVertex3f(bx, 0.02f, bz);
            }
            glEnd();

            // -- Disco no chão: blending ADITIVO para parecer luz real --
            // Brilhante no centro, fadeout até o raio exato do cone
            glBlendFunc(GL_ONE, GL_ONE); // aditivo: soma luminosidade
            glBegin(GL_TRIANGLE_FAN);
            // Centro: amarelo-branco quente
            glColor4f(0.55f * p.intensity, 0.50f * p.intensity, 0.20f * p.intensity, 1.0f);
            glVertex3f(0.0f, 0.01f, 0.0f);
            for (int i = 0; i <= CONE_SEGS; i++)
            {
                float ang = i * 2.0f * PI / CONE_SEGS;
                float bx  = cosf(ang) * coneRad;
                float bz  = sinf(ang) * coneRad;
                glColor4f(0.0f, 0.0f, 0.0f, 1.0f); // borda: nenhuma contribuição
                glVertex3f(bx, 0.01f, bz);
            }
            glEnd();

            // -- Anel (circunferência) na borda do cone no chão --
            // Marca visualmente o limite exato da zona iluminada
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < CONE_SEGS; i++)
            {
                float ang = i * 2.0f * PI / CONE_SEGS;
                float bx  = cosf(ang) * coneRad;
                float bz  = sinf(ang) * coneRad;
                glColor4f(0.9f, 0.85f, 0.4f, 0.55f * p.intensity); // anel dourado
                glVertex3f(bx, 0.015f, bz);
            }
            glEnd();
            glLineWidth(1.0f);

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }


        // -------- PILAR: do chão até o teto --------
        float pillarH = CEILING_H - 0.05f;
        if (p.active && p.intensity > 0.05f)
        {
            float v = 0.5f + 0.4f * p.intensity;
            glColor3f(v * 0.55f, v * 0.50f, v * 0.20f);
        }
        else
        {
            glColor3f(0.15f, 0.15f, 0.15f);
        }
        glPushMatrix();
        glTranslatef(0.0f, pillarH * 0.5f, 0.0f);
        glScalef(0.10f, pillarH, 0.10f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // -------- LÂMPADA COLADA NO TETO --------
        if (p.active && p.intensity > 0.05f)
        {
            float v = p.intensity;
            glColor3f(1.0f * v, 0.96f * v, 0.65f * v);
        }
        else
        {
            glColor3f(0.06f, 0.06f, 0.06f);
        }
        glPushMatrix();
        glTranslatef(0.0f, CEILING_H - 0.15f, 0.0f);
        glutSolidSphere(0.22f, 10, 10);
        glPopMatrix();

        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// Spotlight no teto apontando para baixo — foco com borda bem definida.
// Requer que o chão esteja subdividido (veja desenhaQuadChao) para que
// o per-vertex lighting mostre o círculo de luz com nitidez.
// ---------------------------------------------------------------------------
void setPostLightEachFrame(float postX, float postZ, float intensity, bool enabled)
{
    if (!enabled || intensity < 0.01f)
    {
        glDisable(GL_LIGHT3);
        return;
    }

    glEnable(GL_LIGHT3);

    // Posição exatamente no teto
    GLfloat pos[] = {postX, CEILING_H - 0.05f, postZ, 1.0f};

    // Cor quente e intensa; sem ambient (luz focada
    GLfloat diff[] = {3.5f * intensity, 3.2f * intensity, 2.0f * intensity, 1.0f};
    GLfloat amb[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat spec[] = {0.0f, 0.0f, 0.0f, 1.0f};

    // Spotlight reto para baixo
    GLfloat dir[] = {0.0f, -1.0f, 0.0f};

    glLightfv(GL_LIGHT3, GL_POSITION, pos);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT3, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT3, GL_SPECULAR, spec);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, dir);

    // 55° de abertura → raio no chão ≈ tan(55°) * 2.75 ≈ 3.9 unidades (~1 tile)
    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 55.0f);
    // Expoente 48: borda definida mas condizente com o cone maior
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 48.0f);

    // Sem atenuação por distância — o cone já delimita o foco
    glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.0f);
    glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.0f);
}
