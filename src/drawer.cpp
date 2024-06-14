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
  //printf("Pass %d/%d\n", pass, maxpass);

  if (!camera->changed)
  {
    return false;
  }

  //printf("Do draw %d/%d\n", pass, maxpass);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  camera->look();

  glPointSize(size);
  glBegin(GL_POINTS);

  int n_points_to_display = 0;
  int max_points_to_display = 2000000;

  std::vector<int> cell_npoints;
  std::vector<int> cells_to_display;
  std::vector<float>cell_distance;
  std::vector<char> cell_factor;

  cell_npoints.reserve(2000);
  cells_to_display.reserve(2000);
  cell_distance.reserve(2000);
  cell_factor.reserve(2000);

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

  // Check the cells
  for (auto cell = 0 ; cell < index.ncells ; cell++)
  {
    float cx, cy, cz;
    index.xyz_from_cell(cell, cx, cy, cz);

    if (camera->see(cx-xcenter,cy-ycenter,cz-zcenter))
    {
      n_points_to_display += index.heap[cell].size();
      double distance = sqrt((cx-xcenter-camera->x)*(cx-xcenter-camera->x) + (cy-ycenter-camera->y)*(cy-ycenter-camera->y) + (cz-zcenter-camera->z)*(cz-zcenter-camera->z));

      cell_npoints.push_back(index.heap[cell].size());
      cells_to_display.push_back(cell);
      cell_distance.push_back(distance);
    }
  }

  double dmin = *min_element(cell_distance.begin(), cell_distance.end());
  double dmax = *max_element(cell_distance.begin(), cell_distance.end());

  for (auto d : cell_distance)
  {
    if (d < dmin + (dmax-dmin)/3)
      cell_factor.push_back(1);
    else if (d < dmin + 2*(dmax-dmin)/3)
      cell_factor.push_back(2);
    else
      cell_factor.push_back(8);
  }

  int ntot = 0;
  for (auto j = 0 ; j < cells_to_display.size() ; j++)
  {
    ntot += cell_npoints[j]/cell_factor[j];
  }

  int ratio = std::ceil((double)(ntot)/(double)(max_points_to_display));

  printf("Ratio = %d, d_min = %.1lf, d_max = %.1lf\n", ratio, dmin, dmax);

  int k = 0;
  for (auto j = 0 ; j < cells_to_display.size() ; j++)
  {
    auto cell = cells_to_display[j];
    double dist = cell_distance[j];
    int n = cell_npoints[j];
    int factor = cell_factor[j];
    double d = n/(index.xres*index.yres);

    int local_ratio = ratio*factor;

    for (auto i : index.heap[cell])
    {
      if (local_ratio > 1 && i % local_ratio != 0) continue;

      float px = x(i)-xcenter;
      float py = y(i)-ycenter;
      float pz = z(i)-zcenter;

      //glColor3ub((dist-dmax)/(dmax-dmin)*255, 0, 255-(dist-dmax)/(dmax-dmin)*255);

      if (id(0) == 0)
        glColor3ub(r(i), g(i), b(i));
      else
        glColor3ub(r(id(i)-1), g(id(i)-1), b(id(i)-1));

      glVertex3d(px, py, pz);
      k++;
    }
  }

  glEnd();
  glFlush();

  camera->changed = false;

  printf("Displayed %d/%lu points (%.2f)\n", k, x.size(), (double)k/(double)x.size());

  return true;
}

void Drawer::setPointSize(float size)
{
  if (size > 0)
    this->size = size;
}

