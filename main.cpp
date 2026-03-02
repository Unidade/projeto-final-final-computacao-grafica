#include <GL/glew.h>
#include <GL/glut.h>
#include <cstdio>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

#include "core/app.h"
#include "core/window.h"
#include "core/game.h"

// Set working directory to executable location on Windows
void setWorkingDirectory()
{
#ifdef _WIN32
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0)
    {
        // Find last backslash and terminate there to get directory
        char* lastSlash = strrchr(exePath, '\\');
        if (lastSlash)
        {
            *lastSlash = '\0';
            SetCurrentDirectoryA(exePath);
        }
    }
#endif
}

int main(int argc, char **argv)
{
    setWorkingDirectory();
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(janelaW, janelaH);
    glutCreateWindow("Luzes Apagadas");

#ifdef _WIN32
    // Start in fullscreen mode on Windows
    glutFullScreen();
    fullScreen = true;
#endif

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::printf("Erro GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }

    appInit();

    if (!gameInit("maps/level1.txt"))
        return 1;

    glutMainLoop();
    return 0;
}
