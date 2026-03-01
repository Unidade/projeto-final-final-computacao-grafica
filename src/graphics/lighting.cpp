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
    GLfloat sceneAmbient[] = {0.45f, 0.30f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);
    GLfloat sunDiffuse[] = {1.4f, 0.55f, 0.30f, 1.0f};
    GLfloat sunSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void setSunDirectionEachFrame()
{
    GLfloat sunDir[] = {0.3f, 1.0f, 0.2f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, sunDir);
}
