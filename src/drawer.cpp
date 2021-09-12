#include "drawer.h"

Drawer::Drawer(NumericVector x, NumericVector y, NumericVector z, IntegerVector r, IntegerVector g, IntegerVector b, IntegerVector id)
{
  this->x = x;
  this->y = y;
  this->z = z;
  this->r = r;
  this->g = g;
  this->b = b;
  this->id = id;

  this->npoints = x.length();

  this->minx = min(x);
  this->miny = min(y);
  this->minz = min(z);
  this->maxx = max(x);
  this->maxy = max(y);
  this->maxz = max(z);

  xcenter = (maxx+minx)/2;
  ycenter = (maxy+miny)/2;
  zcenter = (maxz+minz)/2;
  double distance = sqrt((maxx-minx)*(maxx-minx)+(maxy-miny)*(maxy-miny));

  this->maxpass = ceil((double)npoints/(double)2000000);
  this->pass = 1;
  this->size = 2;

  this->camera = new Camera();
  this->camera->setDistance(distance);
  this->camera->setPanSensivity(distance*0.001);
  this->camera->setZoomSensivity(distance*0.05);
}

Drawer::~Drawer()
{
  delete camera;
}

void Drawer::draw()
{
  if (camera->changed)
    pass = 1;
  if (pass > maxpass)
    return;

  if (pass == 1)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  camera->look();

  glPointSize(size);
  glBegin(GL_POINTS);

  for (int i = pass-1 ; i < x.length()-maxpass ; i += maxpass)
  {
    if (id(0) == 0)
      glColor3ub(r(i), g(i), b(i));
    else
      glColor3ub(r(id(i)-1), g(id(i)-1), b(id(i)-1));

    glVertex3d(x(i)-xcenter, y(i)-ycenter, z(i)-zcenter);
  }

  glEnd();
  glFlush();

  camera->changed = false;

  pass++;
  return;
}

void Drawer::setPointSize(float size)
{
  if (size > 0)
    this->size = size;
}

