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

  xcenter = (maxx+minx)/2;
  ycenter = (maxy+miny)/2;
  zcenter = (maxz+minz)/2;
  double distance = sqrt((maxx-minx)*(maxx-minx)+(maxy-miny)*(maxy-miny));

  this->size = 5;

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

  std::vector<int> cell_npoints;
  std::vector<int> cells_to_display;
  std::vector<float>cell_distance;
  std::vector<char> cell_factor;

  cell_npoints.reserve(index.ncells);
  cells_to_display.reserve(index.ncells);
  cell_distance.reserve(index.ncells);
  cell_factor.reserve(index.ncells);

  glLineWidth(2.0f);
  glPointSize(size);

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

  // Check the cells
  for (auto i = 0 ; i < index.ncells ; i++)
  {
    const Cell& cell = index.heap[i];
    if (cell.idx.size() == 0) continue;

    float cx, cy, cz;
    index.xyz_from_cell(i, cx, cy, cz);
    cz = cell.min;

    cx -= xcenter;
    cy -= ycenter;
    cz -= zcenter;

    //bool visible = camera.see(cx, cy, cz);

    // Define the half-size of the cell, assuming square or rectangular cells
    float half_width = index.xres / 2.0f;
    float half_height = index.yres / 2.0f;

    // Calculate the coordinates of the four edge points
    float edge1x = cx - half_width, edge1y = cy - half_height;
    float edge2x = cx + half_width, edge2y = cy - half_height;
    float edge3x = cx - half_width, edge3y = cy + half_height;
    float edge4x = cx + half_width, edge4y = cy + half_height;

    // Check if any of the four edge points are visible
    bool visible = camera.see(edge1x, edge1y, cz) ||
      camera.see(edge2x, edge2y, cz) ||
      camera.see(edge3x, edge3y, cz) ||
      camera.see(edge4x, edge4y, cz);

    if (visible)
    {
      n_points_to_display += cell.idx.size();
      double dx = (cx+camera.deltaX-camera.x);
      double dy = (cy+camera.deltaY-camera.y);
      double dz = (cz+camera.deltaZ-camera.z);
      double distance = sqrt(dx*dx+dy*dy+dz*dz);

      cell_npoints.push_back(n_points_to_display);
      cells_to_display.push_back(i);
      cell_distance.push_back(distance);
    }

    // Draw this quad
    GLfloat size = index.xres;
    GLfloat halfSize = size / 2.0f;

    // Updated vertices based on center and size
    GLfloat vertices[] = {
      cx - halfSize, cy - halfSize, cz,  // Bottom-left corner
      cx + halfSize, cy - halfSize, cz,  // Bottom-right corner
      cx + halfSize, cy + halfSize, cz,  // Top-right corner
      cx - halfSize, cy + halfSize, cz   // Top-left corner
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


  double dmin = *min_element(cell_distance.begin(), cell_distance.end());
  double dmax = *max_element(cell_distance.begin(), cell_distance.end());
  float zrange = maxz-minz;

  printf("  dmin %.lf, dmax %.1lf, zrange %.1f\n", dmin, dmax, zrange);

  glBegin(GL_POINTS);

  int k = 0;

  for (auto j = 0 ; j < cells_to_display.size() ; j++)
  {
    auto cell = cells_to_display[j];
    double dist = cell_distance[j];
    int n = cell_npoints[j];

    int factor = std::floor(dist/(zrange*4))+1;
    //int factor = 1;

    //printf("    camera to cell %d: %.1f, factor %d\n", j, dist, factor);

    for (auto i : index.heap[cell].idx)
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

  camera.changed = false;

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

