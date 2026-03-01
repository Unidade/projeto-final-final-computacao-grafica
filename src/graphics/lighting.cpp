#include "graphics/lighting.h"
#include <GL/glew.h>
#include <GL/glut.h>

void setupIndoorLightOnce()
{
    glEnable(GL_LIGHT1);
    GLfloat lampDiffuse[]  = {1.7f, 1.7f, 1.8f, 1.0f};
    GLfloat lampSpecular[] = {0, 0, 0, 1.0f};
    GLfloat lampAmbient[]  = {0.98f, 0.99f, 1.41f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lampDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lampSpecular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lampAmbient);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.6f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.06f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.02f);
    glDisable(GL_LIGHT1);
}

void setupSunLightOnce()
{
    glEnable(GL_LIGHT0);
    GLfloat sceneAmbient[] = {0.06f, 0.05f, 0.08f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);
    GLfloat sunDiffuse[] = {0.18f, 0.18f, 0.24f, 1.0f};
    GLfloat sunSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void setSunDirectionEachFrame()
{
    GLfloat sunDir[] = {0.15f, 1.0f, 0.15f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, sunDir);
}

void setupFlashlightOnce()
{
    glEnable(GL_LIGHT2);

    GLfloat ambient[]  = {0.00f, 0.00f, 0.00f, 1.0f};
    GLfloat diffuse[]  = {1.70f, 1.65f, 1.45f, 1.0f};
    GLfloat specular[] = {0.65f, 0.60f, 0.48f, 1.0f};

    glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, specular);

    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.45f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.08f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.028f);

    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 28.0f);
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 24.0f);
}

void setFlashlightEachFrame(float camX, float camY, float camZ, float dirX, float dirY, float dirZ, bool enabled)
{
    if (!enabled)
    {
        glDisable(GL_LIGHT2);
        return;
    }

    glEnable(GL_LIGHT2);

    GLfloat position[] = {camX, camY - 0.12f, camZ, 1.0f};
    GLfloat direction[] = {dirX, dirY - 0.05f, dirZ};

    glLightfv(GL_LIGHT2, GL_POSITION, position);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction);
}
