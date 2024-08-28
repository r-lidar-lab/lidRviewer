#include "sdlglutils.h"

#include <cstring>
#include <cstdlib>

SDL_Cursor * cursorFromXPM(const char * xpm[])
{
    int i, row, col;
    int width, height;
    Uint8 * data;
    Uint8 * mask;
    int hot_x, hot_y;
    SDL_Cursor * cursor = NULL;

    sscanf(xpm[0], "%d %d", &width, &height);
    data = (Uint8*)calloc(width/8*height,sizeof(Uint8));
    mask = (Uint8*)calloc(width/8*height,sizeof(Uint8));

    i = -1;
    for ( row=0; row<height; ++row )
    {
        for ( col=0; col<width; ++col )
        {
            if ( col % 8 )
            {
                data[i] <<= 1;
                mask[i] <<= 1;
            }
            else
            {
                ++i;
                data[i] = mask[i] = 0;
            }
            switch (xpm[4+row][col])
            {
                case 'X':
                data[i] |= 0x01;
                mask[i] |= 0x01;
                break;
                case '.':
                mask[i] |= 0x01;
                break;
                case ' ':
                break;
            }
        }
    }
    sscanf(xpm[4+row], "%d,%d", &hot_x, &hot_y);
    cursor = SDL_CreateCursor(data, mask, width, height, hot_x, hot_y);
    free(data);
    free(mask);
    return cursor;
}

