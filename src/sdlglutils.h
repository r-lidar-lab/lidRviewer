#ifndef SDLGLUTILS_H
#define SDLGLUTILS_H

#include <SDL2/SDL.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

int XPMFromImage(const char * imagefile, const char * XPMfile);
SDL_Cursor * cursorFromXPM(const char * xpm[]);

#endif //SDLGLUTILS_H
