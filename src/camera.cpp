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

  const char *move[] =
  {
    /* width height num_colors chars_per_pixel */
    " 16 16 1 ",
    /* colors */
    "X c #000000",
    ". c #ffffff",
    "  c None",
    /* pixels */
    "       XX       ",
    "      X..X      ",
    "     X....X     ",
    "    X......X    ",
    "   X XX..XX X   ",
    "  X.X X..X X.X  ",
    " X..XXX..XXX..X ",
    "X..............X",
    "X..............X",
    " X..XXX..XXX..X ",
    "  X.X X..X X.X  ",
    "   X XX..XX X   ",
    "    X......X    ",
    "     X....X     ",
    "      X..X      ",
    "       XX       ",
    "0,0"
    };

  _hand1 = cursorFromXPM(hand1);
  _hand2 = cursorFromXPM(hand2);
  _move  = cursorFromXPM(move);

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
  panSensivity = 10;
  rotateSensivity = 0.3;
  zoomSensivity = 30;
}

void Camera::OnMouseMotion(const SDL_MouseMotionEvent & event)
{
  if (holdleft) // Rotate
  {
    //printf("Rotate\n");
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
    panSensivity = distance*0.01;
    //printf("Pan\n");
    deltaX += event.xrel*panSensivity;
    deltaY -= event.yrel*panSensivity;
    changed = true;
  }
}

void Camera::OnMouseEvent(const SDL_MouseButtonEvent &event, const SDL_MouseWheelEvent &event_wheel)
{
  //printf("Mouse event\n");

  if (event.button == SDL_BUTTON_LEFT)
  {
    if ((holdleft) && (event.type == SDL_MOUSEBUTTONUP))
    {
      holdleft = false;
      SDL_SetCursor(_hand1);
    }
    else if ((!holdleft) && (event.type == SDL_MOUSEBUTTONDOWN))
    {
      holdleft = true;
      SDL_SetCursor(_hand2);
    }
  }
  else if (event.button == SDL_BUTTON_RIGHT)
  {
    if ((holdright) && (event.type == SDL_MOUSEBUTTONUP))
    {
      holdright = false;
      SDL_SetCursor(_hand1);
    }
    else if ((!holdright) && (event.type == SDL_MOUSEBUTTONDOWN))
    {
      holdright = true;
      SDL_SetCursor(_move);
    }
  }

  // Handle mouse wheel event separately
  if (event_wheel.type == SDL_MOUSEWHEEL)
  {
    //printf("Wheel even\n");

    if (event_wheel.y > 0)
    {
      //printf("Wheel up\n");
      distance += zoomSensivity;
      panSensivity = distance * 0.001;
      zoomSensivity = distance * 0.05;
      changed = true;
    }
    else if (event_wheel.y < 0)
    {
      //printf("Wheel down\n");
      distance -= zoomSensivity;
      panSensivity = distance * 0.001;
      zoomSensivity = distance * 0.05;
      changed = true;
    }
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
  glTranslated(deltaX, deltaY, 0.0);
  gluLookAt(distance,0,0,0,0,0,0,0,1);
  glRotated(angleY,0,1,0);
  glRotated(angleZ,0,0,1);
  glRotated(90, 0, 0, 1);



  // Convert angles from degrees to radians
  double alphay_rad = angleY * M_PI / 180.0;
  double alphaz_rad = angleZ * M_PI / 180.0 * -1;


  double dx = deltaX*cos(alphaz_rad) + deltaY*sin(alphaz_rad);
  double dy = deltaY*cos(alphaz_rad) + deltaX*sin(alphaz_rad);
  /*if (angleZ < 90) { dx *= -1.0; dy *= -1.0; }
  else if (angleZ < 180) { dx *= 1.0; dy *= -1.0; }
  else if (angleZ < 270) { dx *= -1.0; dy *= 1.0; }
  else if (angleZ <= 360) { dx *= -1.0; dy *= -1.0; }*/

  // Calculate the coordinates of the camera
  x = distance*sin(M_PI/2-alphay_rad)*cos(alphaz_rad) + dx;
  y = distance*sin(M_PI/2-alphay_rad)*sin(alphaz_rad) + dy;
  z = distance*cos(M_PI/2-alphay_rad);

  printf("  Δ [%.2f %.2f], δ [%.2f %.2f], α [%.2f, %.2f]\n", deltaX, deltaY, dx, dy, angleY, angleZ);
  printf("  Camera distance = %2.f coordinates = (%.2f %.2f %.2f)\n", distance, x, y, z);
}

float Camera::angle(float px, float py, float pz)
{
  // Calculate vectors AB and BC
  float AB[3] = { (px+deltaX)-x, (py+deltaY)-y, (pz+deltaZ)-z };
  float BC[3] = { -x, -y, -z };

  // Calculate the dot product of AB and BC
  double dot = AB[0] * BC[0] + AB[1] * BC[1] + AB[2] * BC[2];

  // Calculate the magnitudes of AB and BC
  double magAB = sqrt(AB[0] * AB[0] + AB[1] * AB[1] + AB[2] * AB[2]);
  double magBC = sqrt(BC[0] * BC[0] + BC[1] * BC[1] + BC[2] * BC[2]);

  // Calculate the cosine of the angle between AB and BC
  double cosTheta = dot / (magAB * magBC);

  // Calculate the angle in radians
  double angleRadians = acos(cosTheta);

  // Convert the angle to degrees
  double angle = angleRadians * (180.0 / M_PI);

  if (angle < 0) angle *= -1;

  return angle;
}

bool Camera::see(float px, float py, float pz)
{
  return angle(px, py, pz) < 60;
}

