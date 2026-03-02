#include "graphics/hud.h"
#include "graphics/ui_text.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <string>
#include <cstdlib>

static void begin2D(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end2D()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

static void drawCrosshair(int w, int h)
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    begin2D(w, h);

    float cx = w / 2.0f;
    float cy = h / 2.0f;
    float size = 8.0f;

    glColor3f(0.95f, 0.92f, 0.65f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(cx - size, cy); glVertex2f(cx + size, cy);
    glVertex2f(cx, cy - size); glVertex2f(cx, cy + size);
    glEnd();

    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy);
    glEnd();

    end2D();

    glPopAttrib();
}

static void drawDamageOverlay(int w, int h, GLuint texDamage, float alpha)
{
    if (alpha <= 0.0f || texDamage == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texDamage);
    glColor4f(1, 1, 1, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(0, 0);
    glTexCoord2f(1, 1); glVertex2f((float)w, 0);
    glTexCoord2f(1, 0); glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0); glVertex2f(0, (float)h);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

static void drawHealthOverlay(int w, int h, GLuint texHealth, float alpha)
{
    if (alpha <= 0.0f || texHealth == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texHealth);
    glColor4f(1, 1, 1, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(0, 0);
    glTexCoord2f(1, 1); glVertex2f((float)w, 0);
    glTexCoord2f(1, 0); glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0); glVertex2f(0, (float)h);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

static void drawFlashlightHUD(int w, int h, const HudTextures& tex, bool flashlightOn)
{
    GLuint currentTex = flashlightOn ? tex.texLinternOn : tex.texLinternOff;
    if (currentTex == 0) return;

    begin2D(w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, currentTex);
    glColor4f(0.85f, 0.85f, 0.82f, 1);

    float iconH = h * 0.2f;
    float iconW = iconH;
    float x = (w - iconW) / 2.0f;
    float y = h * 0.02f;

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(x, y);
    glTexCoord2f(1, 1); glVertex2f(x + iconW, y);
    glTexCoord2f(1, 0); glVertex2f(x + iconW, y + iconH);
    glTexCoord2f(0, 0); glVertex2f(x, y + iconH);
    glEnd();
    glDisable(GL_BLEND);
    end2D();
}

static void drawStatusBar(int w, int h, const HudTextures& tex, const HudState& s)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    begin2D(w, h);

    float hBar = h * 0.10f; // Altura da barra

    // --- 1. FUNDO DA BARRA ---
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.12f, 0.10f, 0.10f, 0.92f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f((float)w, 0);
        glVertex2f((float)w, hBar); glVertex2f(0, hBar);
    glEnd();
    glDisable(GL_BLEND);

    // --- 2. BORDAS ---
    glLineWidth(2.0f);
    glColor3f(0.7f, 0.7f, 0.75f);
    glBegin(GL_LINES); glVertex2f(0, hBar); glVertex2f((float)w, hBar); glEnd();

    glColor3f(0.2f, 0.2f, 0.25f);
    glBegin(GL_LINES); glVertex2f(w / 2.0f, 0); glVertex2f(w / 2.0f, hBar); glEnd();

    // --- CONFIGURAÇÃO DE TAMANHOS (ESCALAS) ---
    // AQUI ESTÃO OS AJUSTES QUE VOCÊ PEDIU:
    float scaleVida = 0.0020f * hBar;    // AUMENTEI (Texto VIDA grande)
    float scalePequeno = 0.0015f * hBar; // DIMINUI (Texto Baterias bem pequeno)
    float scaleNum = 0.0022f * hBar;     // Números (Médio)
    
    float colLbl[3] = {1.0f, 0.8f, 0.5f}; // Cor bege/dourada

    // ==========================================
    // 3. VIDA (MAIOR)
    // ==========================================
    float xTextHealth = w * 0.05f;
    float yLblHealth = hBar * 0.35f; // Ajustei levemente a altura
    
    glColor3fv(colLbl);
    uiDrawStrokeText(xTextHealth, yLblHealth, "VIDA", scaleVida); // Usa escala maior

    // Barra de Vida
    float barH = hBar * 0.35f;
    float barY = (hBar - barH) / 2.0f;
    // Empurrei a barra um pouco mais pra direita para não bater no texto "VIDA" maior
    float barX = xTextHealth + (w * 0.10f); 
    float barMaxW = (w * 0.35f) - barX;

    // Fundo da barra
    glColor4f(0, 0, 0, 1);
    glBegin(GL_QUADS);
        glVertex2f(barX, barY); glVertex2f(barX + barMaxW, barY);
        glVertex2f(barX + barMaxW, barY + barH); glVertex2f(barX, barY + barH);
    glEnd();

    // Vida Colorida
    float pct = (float)s.playerHealth / 100.0f;
    if (pct < 0) pct = 0; if (pct > 1) pct = 1;
    
    if (pct > 0.6f) glColor3f(0.0f, 0.8f, 0.0f);      
    else if (pct > 0.3f) glColor3f(1.0f, 0.8f, 0.0f); 
    else glColor3f(0.8f, 0.0f, 0.0f);                 

    glBegin(GL_QUADS);
        glVertex2f(barX, barY);
        glVertex2f(barX + (barMaxW * pct), barY);
        glVertex2f(barX + (barMaxW * pct), barY + barH);
        glVertex2f(barX, barY + barH);
    glEnd();

    // ==========================================
    // 4. BATERIAS (MENOR)
    // ==========================================
    float xBatArea = w * 0.60f; 
    
    float batW = hBar * 0.40f;  
    float batH = hBar * 0.22f;  
    float batY = hBar * 0.45f;  
    float nubW = batW * 0.1f;   
    
    // Desenha corpo da bateria
    glLineWidth(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f); 
    glBegin(GL_LINE_LOOP);
        glVertex2f(xBatArea, batY);
        glVertex2f(xBatArea + batW, batY);
        glVertex2f(xBatArea + batW, batY + batH);
        glVertex2f(xBatArea, batY + batH);
    glEnd();

    // Desenha polo positivo
    glBegin(GL_QUADS);
        glVertex2f(xBatArea + batW, batY + batH * 0.25f);
        glVertex2f(xBatArea + batW + nubW, batY + batH * 0.25f);
        glVertex2f(xBatArea + batW + nubW, batY + batH * 0.75f);
        glVertex2f(xBatArea + batW, batY + batH * 0.75f);
    glEnd();

    // Preenchimento
    float fillPct = (float)s.batteriesCollected / (float)s.batteriesRequired;
    if(fillPct > 1.0f) fillPct = 1.0f;
    if(fillPct > 0.0f) {
        glColor3f(0.0f, 0.9f, 0.0f);
        float pad = 2.0f; 
        glBegin(GL_QUADS);
            glVertex2f(xBatArea + pad, batY + pad);
            glVertex2f(xBatArea + pad + ((batW - 2*pad) * fillPct), batY + pad);
            glVertex2f(xBatArea + pad + ((batW - 2*pad) * fillPct), batY + batH - pad);
            glVertex2f(xBatArea + pad, batY + batH - pad);
        glEnd();
    }
    // NÚMERO
    glColor3f(0.6f, 0.9f, 0.4f); 
    glPushMatrix();
        float numX = xBatArea + batW + nubW + (w * 0.015f); 
        float numY = batY + (batH * 0.2f); 
        glTranslatef(numX, numY, 0);
        glScalef(scaleNum, scaleNum, 1);
        
        std::string sBat = std::to_string(s.batteriesCollected) + "/" + std::to_string(s.batteriesRequired);
        for (char c : sBat) glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
glPopMatrix();

    // LEGENDA (Aqui usamos a scalePequeno)
    glColor3fv(colLbl);
    // Usei "BATERIAS" em vez de "BATERIAS (FASE)" pra ajudar a caber, mas se quiser manter o texto longo, ele vai ficar pequeno o suficiente agora.
    const char* textoBat = "BATERIAS"; 
    float lblBatW = uiStrokeTextWidthScaled(textoBat, scalePequeno);
    float centerGroup = xBatArea + (batW + nubW + 40.0f) / 2.0f; 
    
    // Desenha usando scalePequeno
    uiDrawStrokeText(centerGroup - (lblBatW/2.0f), hBar * 0.15f, textoBat, scalePequeno);

    // ==========================================
    // 5. CHAVE
    // ==========================================
    float xKeyCenter = w * 0.85f;

    // Legenda CHAVE (Usa scalePequeno também para combinar)
    glColor3fv(colLbl);
    float lblKeyW = uiStrokeTextWidthScaled("CHAVE", scalePequeno);
    uiDrawStrokeText(xKeyCenter - (lblKeyW/2.0f), hBar * 0.15f, "CHAVE", scalePequeno);

    // Ícone da Chave
    int kl = (s.currentLevel >= 1 && s.currentLevel <= 3) ? (s.currentLevel - 1) : 0;
    GLuint keyTex = tex.texKeyHud[kl];
    float keySize = hBar * 0.45f;
    float keyYPos = hBar * 0.40f;

    if (keyTex && s.hasLevelKey)
    {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, keyTex);
     glColor3f(1, 1, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(xKeyCenter - keySize/2, keyYPos);
            glTexCoord2f(1, 1); glVertex2f(xKeyCenter + keySize/2, keyYPos);
            glTexCoord2f(1, 0); glVertex2f(xKeyCenter + keySize/2, keyYPos + keySize);
            glTexCoord2f(0, 0); glVertex2f(xKeyCenter - keySize/2, keyYPos + keySize);
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        glColor3f(0.4f, 0.4f, 0.4f);
        glPushMatrix();
            glTranslatef(xKeyCenter - (100.0f * scaleNum)/2.0f, keyYPos, 0);
            glScalef(scaleNum, scaleNum, 1);
            glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, '?');
        glPopMatrix();
    }

    end2D();
    glPopAttrib();
}


// Exibe mensagem centralizada na tela quando a porta está bloqueada
static void drawDoorMessage(int w, int h, const char* msg, float alpha)
{
    if (!msg || alpha <= 0.0f)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    begin2D(w, h);

    float scale = 0.00045f * h;
    float textW = uiStrokeTextWidthScaled(msg, scale);
    float pad   = h * 0.018f;
    float lineH = h * 0.040f;
    float bW    = textW + pad * 4.0f;
    float bH    = lineH + pad * 2.5f;
    float bx    = (w - bW) / 2.0f;
    float by    = h * 0.48f;

    // Fundo escuro semitransparente
    glColor4f(0.05f, 0.0f, 0.0f, 0.75f * alpha);
    glBegin(GL_QUADS);
    glVertex2f(bx,      by);
    glVertex2f(bx + bW, by);
    glVertex2f(bx + bW, by + bH);
    glVertex2f(bx,      by + bH);
    glEnd();

    // Borda vermelha
    glLineWidth(2.0f);
    glColor4f(0.9f, 0.1f, 0.1f, alpha);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx,      by);
    glVertex2f(bx + bW, by);
    glVertex2f(bx + bW, by + bH);
    glVertex2f(bx,      by + bH);
    glEnd();

    // Texto branco
    glColor4f(1.0f, 0.9f, 0.8f, alpha);
    float tx = bx + (bW - textW) / 2.0f;
    float ty = by + pad + lineH * 0.15f;
    uiDrawStrokeText(tx, ty, msg, scale);

    end2D();
    glPopAttrib();
}

// Flash verde de tela cheia quando todas as baterias são coletadas.
// Usa um vignette de bordas pulsado com blending aditivo para imitar um glow.
static void drawAllBatteriesFlash(int w, int h, float alpha)
{
    if (alpha <= 0.0f)
        return;

    // Pulso: faz alpha oscilar para efeito de brilho vivo
    // (usa o próprio alpha que já decai linearmente de 1→0 em 2.5s)
    // Para os primeiros 0.3s força alpha máximo para impacto imediato
    float pulse = alpha; // já está em [0,1]

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    begin2D(w, h);

    float fw = (float)w, fh = (float)h;
    float vigW = fw * 0.35f; // largura do vignette nas bordas

    // Blending aditivo: adiciona verde ao que já existe na tela
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Borda esquerda
    glBegin(GL_QUADS);
    glColor4f(0.0f, 1.0f, 0.1f, 0.55f * pulse);
    glVertex2f(0,    0);    glVertex2f(0,    fh);
    glColor4f(0.0f, 1.0f, 0.1f, 0.0f);
    glVertex2f(vigW, fh);   glVertex2f(vigW, 0);
    glEnd();

    // Borda direita
    glBegin(GL_QUADS);
    glColor4f(0.0f, 1.0f, 0.1f, 0.0f);
    glVertex2f(fw - vigW, 0);  glVertex2f(fw - vigW, fh);
    glColor4f(0.0f, 1.0f, 0.1f, 0.55f * pulse);
    glVertex2f(fw,        fh); glVertex2f(fw,        0);
    glEnd();

    // Borda superior
    glBegin(GL_QUADS);
    glColor4f(0.0f, 1.0f, 0.1f, 0.55f * pulse);
    glVertex2f(0,  0);  glVertex2f(fw, 0);
    glColor4f(0.0f, 1.0f, 0.1f, 0.0f);
    glVertex2f(fw, vigW); glVertex2f(0, vigW);
    glEnd();

    // Borda inferior
    glBegin(GL_QUADS);
    glColor4f(0.0f, 1.0f, 0.1f, 0.0f);
    glVertex2f(0,  fh - vigW); glVertex2f(fw, fh - vigW);
    glColor4f(0.0f, 1.0f, 0.1f, 0.55f * pulse);
    glVertex2f(fw, fh);        glVertex2f(0,  fh);
    glEnd();

    end2D();
    glPopAttrib();
}

void desenhaTutorial(float time, int w, int h)
{
    // Nada apos os dois paineis terem sumido
    if (time > 12.0f)
        return;

    // --- Alpha de cada painel ---
    // Caixa 1: visivel 0-5s, fade out 5-6s
    float alpha1 = 0.0f;
    if (time < 5.0f)
        alpha1 = 1.0f;
    else if (time < 6.0f)
        alpha1 = 1.0f - (time - 5.0f); // 1s de fade

    // Caixa 2: visivel 6-11s, fade out 11-12s
    float alpha2 = 0.0f;
    if (time >= 6.0f && time < 11.0f)
        alpha2 = 1.0f;
    else if (time >= 11.0f && time < 12.0f)
        alpha2 = 1.0f - (time - 11.0f); // 1s de fade

    if (alpha1 <= 0.0f && alpha2 <= 0.0f)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    begin2D(w, h);

    // --- Escalas e metricas comuns ---
    float scale1 = 0.00035f * h;
    float scale2 = 0.00025f * h;
    float pad    = h * 0.015f;
    float lineH  = h * 0.030f;

    // Helper: desenha uma caixa com duas linhas de texto
    auto drawBox = [&](float alpha, const char* t1, const char* t2)
    {
        if (alpha <= 0.0f) return;

        float t1W = uiStrokeTextWidthScaled(t1, scale1);
        float t2W = uiStrokeTextWidthScaled(t2, scale2);
        float bW  = (t2W > t1W ? t2W : t1W) + pad * 4.0f;
        float bH  = lineH * 2.0f + pad * 3.0f;
        float bx  = (w - bW) / 2.0f;
        float by  = h * 0.75f;

        // Fundo escuro
        glColor4f(0.0f, 0.0f, 0.0f, 0.60f * alpha);
        glBegin(GL_QUADS);
        glVertex2f(bx,      by);      glVertex2f(bx + bW, by);
        glVertex2f(bx + bW, by + bH); glVertex2f(bx,      by + bH);
        glEnd();

        // Linha de titulo (branco)
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        uiDrawStrokeText(bx + (bW - t1W) / 2.0f, by + pad + lineH * 1.2f, t1, scale1);

        // Linha de detalhe (cinza claro)
        glColor4f(0.85f, 0.85f, 0.85f, alpha);
        uiDrawStrokeText(bx + (bW - t2W) / 2.0f, by + pad * 0.8f,         t2, scale2);
    };

    drawBox(alpha1,
            "CUIDADO: Fique na luz!",
            "Sair da luz atrai o monstro e voce sera atacado.");

    drawBox(alpha2,
            "OBJETIVO:",
            "Colete as baterias e a chave, depois procure a saida.");

    end2D();
    glPopAttrib();
}

void hudRenderAll(
    int screenW,
    int screenH,
    const HudTextures& tex,
    const HudState& state,
    bool showCrosshair,
    bool showWeapon,
    bool showStatusBar)
{
    // Ordem: flashlight -> barra -> mira -> overlays
    if (showWeapon)  drawFlashlightHUD(screenW, screenH, tex, state.flashlightOn);
    if (showStatusBar) drawStatusBar(screenW, screenH, tex, state);

    if (showCrosshair) drawCrosshair(screenW, screenH);

    drawDoorMessage(screenW, screenH, state.doorMessage, state.doorMessageAlpha);
    drawAllBatteriesFlash(screenW, screenH, state.allBatteriesFlashAlpha);
    drawDamageOverlay(screenW, screenH, tex.texDamage, state.damageAlpha);
    drawHealthOverlay(screenW, screenH, tex.texHealthOverlay, state.healthAlpha);
}
