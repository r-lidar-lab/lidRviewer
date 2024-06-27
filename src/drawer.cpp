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
  index = GridPartition(x,y,z);

  attr = Attribute::Z;

  xcenter = (maxx+minx)/2;
  ycenter = (maxy+miny)/2;
  zcenter = (maxz+minz)/2;
  double distance = sqrt((maxx-minx)*(maxx-minx)+(maxy-miny)*(maxy-miny));

  this->size = 1;

  this->camera = new Camera();
  this->camera->setDistance(distance);
  this->camera->setPanSensivity(distance*0.001);
  this->camera->setZoomSensivity(distance*0.05);
}

Drawer::~Drawer()
{
  delete camera;
}

bool Drawer::draw()
{
  if (!camera->changed)  return false;

  // Immediate mode. Should be modernized.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  camera->look();

  glPointSize(2);
  glBegin(GL_POINTS);

  int n_points_to_display = 0;
  int max_points_to_display = 2000000;

  std::vector<int> cell_npoints;
  std::vector<int> cells_to_display;
  std::vector<float>cell_distance;
  std::vector<char> cell_factor;

  cell_npoints.reserve(index.ncells);
  cells_to_display.reserve(index.ncells);
  cell_distance.reserve(index.ncells);
  cell_factor.reserve(index.ncells);

  // Check the cells
  for (auto cell = 0 ; cell < index.ncells ; cell++)
  {
    float cx, cy, cz;
    index.xyz_from_cell(cell, cx, cy, cz);

    //printf("cell %d: (%.1f, %.1f, %.1f)\n", cell, cx, cy, cz);

    if (camera->see(cx-xcenter,cy-ycenter,cz-zcenter))
    {
      n_points_to_display += index.heap[cell].size();
      double dx = (cx-xcenter+camera->deltaX-camera->x);
      double dy = (cy-ycenter+camera->deltaY-camera->y);
      double dz = (cz-zcenter+camera->deltaZ-camera->z);
      double distance = sqrt(dx*dx+dy*dy+dz*dz);

      cell_npoints.push_back(index.heap[cell].size());
      cells_to_display.push_back(cell);
      cell_distance.push_back(distance);
    }
  }

  double dmin = *min_element(cell_distance.begin(), cell_distance.end());
  double dmax = *max_element(cell_distance.begin(), cell_distance.end());
  float zrange = index.zmax-index.zmin;

  printf("dmin %.lf, dmax %.1lf, zrange %.1f\n", dmin, dmax, zrange);

  int k = 0;

  for (auto j = 0 ; j < cells_to_display.size() ; j++)
  {
    auto cell = cells_to_display[j];
    double dist = cell_distance[j];
    int n = cell_npoints[j];

    int factor = std::floor(dist/(zrange*4))+1;

    //printf("camera to cell %d: %.1f, factor %d\n", j, dist, factor);

    for (auto i : index.heap[cell])
    {
      if (factor > 1 && i % factor != 0) continue;

      float px = x(i)-xcenter;
      float py = y(i)-ycenter;
      float pz = z(i)-zcenter;

      // Coloring
      switch (attr)
      {
      case Attribute::Z:
        if (id(0) == 0)
          glColor3ub(r(i), g(i), b(i));
        else
          glColor3ub(r(id(i)-1), g(id(i)-1), b(id(i)-1));

        break;
      case Attribute::Distance:
        glColor3ub((dmax-dist)/(dmax-dmin)*255, 0, 255-(dmax-dist)/(dmax-dmin)*255);
        break;
      case Attribute::Ratio:
        if (factor == 1) glColor3ub(0, 138, 255);
        else if (factor == 2) glColor3ub(0, 255, 0);
        else if (factor == 3) glColor3ub(255, 138, 0);
        else if (factor == 4) glColor3ub(255, 0, 0);
        else if (factor == 5) glColor3ub(138, 0, 0);
        else if (factor == 6) glColor3ub(70, 0, 0);
        else glColor3ub(45, 0, 0);
        break;
      }

      glVertex3d(px, py, pz);
      k++;
    }
  }

  glEnd();
  glFlush();

  camera->changed = false;

  printf("Displayed %lu/%u cells %d/%lu points (%.2f)\n", cells_to_display.size(), index.ncells, k, x.size(), (double)k/(double)x.size());

  return true;
}

void Drawer::setPointSize(float size)
{
  if (size > 0)
    this->size = size;
}


/*for (auto i = 0 ; i < x.size() ; i++)
 {
 float px = x(i)-xcenter;
 float py = y(i)-ycenter;
 float pz = z(i)-zcenter;

 if (camera->see(px, py, pz))
 glColor3ub(0, 255, 0);
 else
 glColor3ub(255, 0, 0);

 glVertex3d(px, py, pz);
 }


 glEnd();
 glFlush();

 camera->changed = false;
 return true;*/

