#include "camera.h"
#include <GL/glu.h>
#include <cmath>
#include "sdlglutils.h"
#include <Rcpp.h>

Camera::Camera()
{
  const char *hand1[] =
  {
    /* width height num_colors chars_per_pixel */
    " 16 16 3 1 ",
    /* colors */
    "X c #000000",
    ". c #ffffff",
    "  c None",
    /* pixels */
    "       XX       ",
    "   XX X..XXX    ",
    "  X..XX..X..X   ",
    "  X..XX..X..X X ",
    "   X..X..X..XX.X",
    "   X..X..X..X..X",
    " XX X.......X..X",
    "X..XX..........X",
    "X...X.........X ",
    " X............X ",
    "  X...........X ",
    "  X..........X  ",
    "   X.........X  ",
    "    X.......X   ",
    "     X......X   ",
    "     X......X   ",
    "0,0"
  };

  const char *hand2[] =
  {
    /* width height num_colors chars_per_pixel */
    " 16 16 3 1 ",
    /* colors */
    "X c #000000",
    ". c #ffffff",
    "  c None",
    /* pixels */
    "                ",
    "                ",
    "                ",
    "                ",
    "    XX XX XX    ",
    "   X..X..X..XX  ",
    "   X........X.X ",
    "    X.........X ",
    "   XX.........X ",
    "  X...........X ",
    "  X...........X ",
    "  X..........X  ",
    "   X.........X  ",
    "    X.......X   ",
    "     X......X   ",
    "     X......X   ",
    "0,0"
  };

  _hand1 = cursorFromXPM(hand1);
  _hand2 = cursorFromXPM(hand2);

  SDL_SetCursor(_hand1);
  holdleft = false;
  holdright = false;
  angleY = 20;
  angleZ = -30;
  deltaX = 0;
  deltaY = 0;
  deltaZ = 0;
  distance = 300;
  changed = true;
  panSensivity = 1;
  rotateSensivity = 0.3;
  zoomSensivity = 30;
}

void Camera::OnMouseMotion(const SDL_MouseMotionEvent & event)
{
  if (holdleft) // Rotate
  {
    angleZ += event.xrel*rotateSensivity;
    angleY += event.yrel*rotateSensivity;

    if (angleY > 90)
      angleY = 90;
    else if (angleY < -90)
      angleY = -90;

    changed = true;
  }
  else if (holdright) // Pan
  {
  }
}

void Camera::OnMouseButton(const SDL_MouseButtonEvent & event)
{
  if (event.button == SDL_BUTTON_LEFT)
  {
    if ((holdleft)&&(event.type == SDL_MOUSEBUTTONUP))
    {
      holdleft = false;
      SDL_SetCursor(_hand1);
    }
    else if ((!holdleft)&&(event.type == SDL_MOUSEBUTTONDOWN))
    {
      holdleft = true;
      SDL_SetCursor(_hand2);
    }
  }
  else if (event.button == SDL_BUTTON_RIGHT)
  {
    if ((holdright)&&(event.type == SDL_MOUSEBUTTONUP))
    {
      holdright = false;
      SDL_SetCursor(_hand1);
    }
    else if ((!holdright)&&(event.type == SDL_MOUSEBUTTONDOWN))
    {
      holdright = true;
      SDL_SetCursor(_hand2);
    }
  }
  else if ((event.button == SDL_BUTTON_WHEELUP) && (event.type == SDL_MOUSEBUTTONDOWN))
  {
    distance -= zoomSensivity;
    changed = true;
  }
  else if ((event.button == SDL_BUTTON_WHEELDOWN) && (event.type == SDL_MOUSEBUTTONDOWN))
  {
    distance += zoomSensivity;
    changed = true;
  }
}

void Camera::OnKeyboard(const SDL_KeyboardEvent & event)
{
  if ((event.type == SDL_KEYDOWN)&&(event.keysym.sym == SDLK_HOME))
  {
    angleY = 0;
    angleZ = 0;
    changed = true;
  }
}

void Camera::setPanSensivity(double sensivity)
{
  panSensivity = sensivity;
}

void Camera::setRotateSensivity(double sensivity)
{
  rotateSensivity = sensivity;
}

void Camera::setZoomSensivity(double sensivity)
{
  zoomSensivity = sensivity;
}

void Camera::setDeltaXYZ(double dx, double dy, double dz)
{
  deltaX = dx;
  deltaY = dy;
  deltaZ = dz;
}

void Camera::setDistance(double dist)
{
  if (dist > 0)
    distance = dist;
}

Camera::~Camera()
{
  SDL_FreeCursor(_hand1);
  SDL_FreeCursor(_hand2);
  SDL_SetCursor(NULL);
}

void Camera::look()
{
  gluLookAt(distance,0,0,0,0,0,0,0,1);
  glRotated(angleY,0,1,0);
  glRotated(angleZ,0,0,1);
  glRotated(90, 0, 0, 1);
}

