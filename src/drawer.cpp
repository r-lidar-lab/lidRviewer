#include "drawer.h"

Drawer::Drawer(NumericVector x, NumericVector y, NumericVector z, IntegerMatrix col)
{
  this->x = x;
  this->y = y;
  this->z = z;
  this->rgb = col;

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

  this->maxpass = ceil((double)npoints/(double)1000000);
  this->pass = 1;

  Rcout << "max pass = " << maxpass << std::endl;

  this->size = 2;

  this->camera = new Camera();
  this->camera->setDeltaXYZ((maxx+minx)/2, (maxy+miny)/2, minz);
}

Drawer::~Drawer()
{
  delete camera;
}

void Drawer::draw()
{
  if (camera->changed)
    pass = 1;
  else if (pass == maxpass)
    return;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  camera->look();

  glPointSize(size);
  glBegin(GL_POINTS);

  for (int j = maxpass-pass; j < maxpass ; j++)
  {
    for (int i = j ; i < x.length()-maxpass ; i+=maxpass)
    {
      glColor3ub(rgb(i,0),rgb(i,1),rgb(i,2));
      glVertex3d(x(i)-xcenter, y(i)-ycenter, z(i)-zcenter);
    }
  }

  glEnd();
  glFlush();
  SDL_GL_SwapBuffers();

  camera->changed = false;

  if (pass < maxpass)
    pass++;

  Rcout << "draw pass = " << pass << std::endl;

  return;
}

void Drawer::setPointSize(float size)
{
  this->size = size;
}
