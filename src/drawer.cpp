#include "drawer.h"
#include <chrono>

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
  this->maxz = mean(z)*2-minz;

  auto start = std::chrono::high_resolution_clock::now();

  index = GridPartition(x,y,z);

  auto end = std::chrono::high_resolution_clock::now();

  // Calculate the duration
  std::chrono::duration<double> duration = end - start;

  // Output the duration in seconds
  std::cout << "Time taken for indexation: " << duration.count() << " seconds" << std::endl;

  attr = Attribute::Z;
  draw_index = false;

  xcenter = (maxx+minx)/2;
  ycenter = (maxy+miny)/2;
  zcenter = (maxz+minz)/2;
  double distance = sqrt((maxx-minx)*(maxx-minx)+(maxy-miny)*(maxy-miny));

  this->size = 5.0;

  this->camera.setDistance(distance);
  this->camera.setPanSensivity(distance*0.001);
  this->camera.setZoomSensivity(distance*0.05);
}

bool Drawer::draw()
{
  if (!camera.changed)  return false;

  // Immediate mode. Should be modernized.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  camera.look();

  int n_points_to_display = 0;
  int max_points_to_display = 2000000;
  float zrange = maxz-minz;

  glLineWidth(2.0f);
  glPointSize(this->size);

  // ==========================
  // Check the cells visibility
  // ==========================

  for (auto i = 0 ; i < index.ncells ; i++)
  {
    Cell& cell = index.heap[i];
    Cell& pcell = index.preview_points[i];

    // compute the distance of the cell to the camera
    // ----------------------------------------------

    float cx, cy, cz;

    index.xyz_from_cell(i, cx, cy, cz);

    cx -= xcenter;
    cy -= ycenter;
    cz = cell.min;
    cz -= zcenter;

    double dx = (cx-camera.x);
    double dy = (cy-camera.y);
    double dz = (cz-camera.z);

    cell.distance = sqrt(dx*dx+dy*dy+dz*dz);
    pcell.distance = cell.distance;

    // Check the cells visibility
    // --------------------------

    // Define the half-size of the cell, assuming square or rectangular cells
    float half_width = index.xres / 2.0f;
    float half_height = index.yres / 2.0f;

    // Calculate the coordinates of the four edge points
    float edge1x = cx - half_width, edge1y = cy - half_height;
    float edge2x = cx + half_width, edge2y = cy - half_height;
    float edge3x = cx - half_width, edge3y = cy + half_height;
    float edge4x = cx + half_width, edge4y = cy + half_height;

    // Check if any of the four edge points are visible
    bool visible = camera.see(edge1x, edge1y, cz) || camera.see(edge2x, edge2y, cz) || camera.see(edge3x, edge3y, cz) || camera.see(edge4x, edge4y, cz);

    cell.visible = visible;
    pcell.visible = visible;

    if (visible)
    {
      n_points_to_display += cell.idx.size();
      n_points_to_display += pcell.idx.size();
    }

    if (draw_index && cell.idx.size() > 0)
    {
      // Updated vertices based on center and size
      GLfloat vertices[] = {
        edge1x, edge1y, cz,
        edge2x, edge2y, cz,
        edge4x, edge4y, cz,
        edge3x, edge3y, cz
      };

      glBegin(GL_LINE_LOOP);

      if (visible)
        glColor3f(1.0f, 0.0f, 0.0f);
      else
        glColor3f(0.25f, 0.0f, 0.0f);

      for (int i = 0; i < 12; i += 3)
      {
        glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
      }
      glEnd();
    }
  }

  double dmin = INFD;
  double dmax = -INFD
  for (auto& cell : index.heap)
  {
    if (!cell.visible) continue;
    if (cell.distance < dmin) dmin = cell.distance;
    if (cell.distance > dmax) dmax = cell.distance;
  }

  printf("dmin %.1lf, dmax %.1lf\n", (maxx-minx)/2, (maxx-minx)*3);

  // Decimation parameters
  float fmin = 0.01f;
  float fmax = 1.0f;
  int nsampled = 0;
  for (auto i = 0 ; i < index.ncells ; i++)
  {
    Cell& cell = index.heap[i];
    Cell& pcell = index.preview_points[i];

    if (cell.visible == false) continue;

    float factor = 0;
    double dmin = (maxx-minx)/2;
    double dmax = (maxx-minx);
    if (cell.distance <= dmin)
      factor = 1.0f;
    else if (cell.distance >= dmax)
      factor = 0.0f;
    else
      factor = 1.0f - (cell.distance - dmin) / (dmax - dmin);

    cell.factor = factor;
    pcell.factor = factor;

    nsampled += cell.idx.size() * cell.factor;

    printf(" distance = %.1lf factor %.3f, nsampled %d\n", cell.distance, cell.factor, nsampled);
  }

  if (nsampled > max_points_to_display)
  {
    for (auto i = 0 ; i < index.ncells ; i++)
    {
      Cell& cell = index.heap[i];
      Cell& pcell = index.preview_points[i];

      if (cell.visible == false) continue;

      cell.factor = cell.factor * (double)max_points_to_display/(double)nsampled;
      pcell.factor = cell.factor;

      printf(" distance = %.1lf factor %.3f\n", cell.distance, cell.factor);
    }
  }

  //printf("dmin = %.1lf, dmax = %.1lf\n", dmin, dmax);

  // ======================
  // Display preview points
  // ======================

  glBegin(GL_POINTS);

  int k = 0;
  int nc = 0;
  for (const auto& cell : index.preview_points)
  {
    if (!cell.visible) continue;

    nc++;

    double dist = cell.distance;
    int n = cell.idx.size();
    float factor = cell.factor;

    //printf("    camera to cell %d: %.1f, factor %d\n", j, dist, factor);

    for (auto i : cell.idx)
    {
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
        glColor3ub((1.0f-factor)*255, 0, factor*255);
        break;
      }

      glVertex3d(px, py, pz);
      k++;
    }
  }

  // ======================
  // Display sampled points
  // ======================

  std::vector<Cell*> cells;
  for (auto& cell : index.heap) { cells.push_back(&cell); }
  std::sort(cells.begin(), cells.end(), [](const Cell* a, const Cell* b) { return a->distance < b->distance; });

  for (const auto cell : cells)
  {
    if (!cell->visible) continue;
    if (k > max_points_to_display) break;

    double dist = cell->distance;
    float factor = cell->factor;
    int npoints = cell->idx.size();
    int ndisplay = (double)npoints*factor;

    for (int j = 0 ; j < ndisplay ; j++)
    {
      int i = cell->idx[j];

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
        glColor3ub((1.0f-factor)*255, 0, factor*255);
        break;
      }

      glVertex3d(px, py, pz);
      k++;
    }
  }

  glEnd();

  // Draw the X axis (red)
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_LINES);
  glVertex3f(minx-xcenter, miny-ycenter, minz+10); // Start point of X axis
  glVertex3f(minx-xcenter+20, miny-ycenter, minz+10);  // End point of X axis
  glEnd();

  // Draw the Y axis (green)
  glColor3f(0.0f, 1.0f, 0.0f);
  glBegin(GL_LINES);
  glVertex3f(minx-xcenter, miny-ycenter, minz+10); // Start point of Y axis
  glVertex3f(minx-xcenter, miny-ycenter+20, minz+10);  // End point of Y axis
  glEnd();

  // Draw the Z axis (blue)
  glColor3f(0.0f, 0.0f, 1.0f);
  glBegin(GL_LINES);
  glVertex3f(minx-xcenter, miny-ycenter, minz+10); // Start point of Y axis
  glVertex3f(minx-xcenter, miny-ycenter, minz+20+10);  // End point of Y axis
  glEnd();

  camera.changed = false;

  printf("Displayed %d/%u cells %d/%lu points (%.1f\%)\n", nc, index.ncells, k, x.size(), (double)k/(double)x.size()*100);

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

 if (camera.see(px, py, pz))
 glColor3ub(0, 255, 0);
 else
 glColor3ub(255, 0, 0);

 glVertex3d(px, py, pz);
 }


 glEnd();
 glFlush();

 camera.changed = false;
 return true;*/

