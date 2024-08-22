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

  this->dmin = INFD;
  this->dmax = -INFD

  this->index = GridPartition(x,y,z);

  this->attr = Attribute::Z;
  this->draw_index = false;
  this->max_points_to_display = 1000000;

  this->pp.reserve(this->max_points_to_display*1.1);

  this->xcenter = (maxx+minx)/2;
  this->ycenter = (maxy+miny)/2;
  this->zcenter = (maxz+minz)/2;
  this->xrange = maxx-minx;
  this->yrange = maxy-miny;
  this->zrange = maxz-minz;
  this->range = std::max(xrange, yrange);

  this->size = 5.0;

  double distance = sqrt(xrange*xrange+yrange*yrange);
  this->camera.setDistance(distance);
  this->camera.setPanSensivity(distance*0.001);
  this->camera.setZoomSensivity(distance*0.05);
}

bool Drawer::draw()
{
  if (!camera.changed)  return false;

  auto start = std::chrono::high_resolution_clock::now();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // Immediate mode. Should be modernized.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glLineWidth(2.0f);
  glPointSize(this->size);

  camera.look(); // Reposition the camera after rotation and translation of the scene;

  auto start_query = std::chrono::high_resolution_clock::now();

  compute_cell_visibility();
  query_rendered_point();

  auto end_query = std::chrono::high_resolution_clock::now();
  auto start_rendering = std::chrono::high_resolution_clock::now();

  if (draw_index)
  {
    for (auto i = 0 ; i < index.ncells ; i++)
    {
      Cell& cell = index.heap[i];
      if (!cell.visible || cell.preview.size() == 0) continue;

      float cx, cy, cz;

      index.xyz_from_cell(i, cx, cy, cz);

      cx -= xcenter;
      cy -= ycenter;
      cz = cell.min;
      cz -= zcenter;

      float half_width = index.xres / 2.0f;
      float half_height = index.yres / 2.0f;

      float edge1x = cx - half_width, edge1y = cy - half_height;
      float edge2x = cx + half_width, edge2y = cy - half_height;
      float edge3x = cx - half_width, edge3y = cy + half_height;
      float edge4x = cx + half_width, edge4y = cy + half_height;
      // Updated vertices based on center and size
      GLfloat vertices[] = {
        edge1x, edge1y, cz,
        edge2x, edge2y, cz,
        edge4x, edge4y, cz,
        edge3x, edge3y, cz
      };

      glBegin(GL_LINE_LOOP);
      glColor3f(1.0f, 0.0f, 0.0f);
      for (int i = 0; i < 12; i += 3)
        glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
      glEnd();
    }
  }

  glBegin(GL_POINTS);

  for (auto p : pp)
  {
    int i = p.pindex;
    float px = x(i)-xcenter;
    float py = y(i)-ycenter;
    float pz = z(i)-zcenter;
    glColor3ub(p.r, p.g, p.b);
    glVertex3d(px, py, pz);
  }

  glEnd();
  auto end_rendering = std::chrono::high_resolution_clock::now();

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

  auto end = std::chrono::high_resolution_clock::now();

  // Calculate the duration
  std::chrono::duration<double> total_duration = end - start;
  std::chrono::duration<double> query_duration = end_query - start_query;
  std::chrono::duration<double> rendering_duration = end_rendering - start_rendering;

  printf("Displayed %dk/%ldk points (%.1f\%)\n", (int)pp.size()/1000, x.size()/1000, (double)pp.size()/(double)x.size()*100);
  printf("Full Rendering: %.3f seconds (%.1f fps)\n", total_duration.count(), 1.0f/total_duration.count());
  printf("Cloud rendering: %.3f seconds (%.1f fps, %.1f\%)\n", rendering_duration.count(), 1.0f/rendering_duration.count(), rendering_duration.count()/total_duration.count()*100);
  printf("Spatial query: %.3f seconds (%.1f fps %.1f\%)\n", query_duration.count(), 1.0f/query_duration.count(), query_duration.count()/total_duration.count()*100);
  printf("\n");

  return true;
}

void Drawer::compute_cell_visibility()
{
  int nvisible = 0;

  for (auto i = 0 ; i < index.ncells ; i++)
  {
    Cell& cell = index.heap[i];

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

    if (visible) nvisible++;
  }

  printf("Visibility: %d/%lu visible cells\n", nvisible, index.heap.size());

  // Compute the range of distance from camera to cell

  dmin = INFD;
  dmax = -INFD
  for (const auto& cell : index.heap)
  {
    if (!cell.visible) continue;
    if (cell.distance < dmin) dmin = cell.distance;
    if (cell.distance > dmax) dmax = cell.distance;
  }

  return;
}

void Drawer::query_rendered_point()
{
  pp.clear();

  // Estimate the number of point to display if we plot all the visible points

  int n_points_to_display = 0;
  for (const auto& cell : index.heap)
  {
    if (cell.visible)
    {
      n_points_to_display += cell.preview.size();
      n_points_to_display += cell.points.size();
    }
  }

  // Estimate the cell decimation parameters

  printf("dmin %.1lf, dmax %.1lf\n\n", dmin, dmax);

  float tmin = 2;
  float tmax = 6;
  int nsampled = 0;
  for (auto& cell : index.heap)
  {
    if (!cell.visible) continue;

    double distance = cell.distance/dmin;

    float factor = 0;
    if (distance <= tmin)
      factor = 1.0f;
    else if (distance >= tmin)
      factor = 0.0f;
    else
      factor = 1.0f - (distance - tmin) / (tmax - tmin);

    cell.factor = factor;

    nsampled += cell.points.size() * cell.factor;

    printf("d = %.1lf, dr = %.1lf factor %.3f, nsampled %d\n", cell.distance, distance, cell.factor, nsampled);
  }

  printf("\n");

  // If we sample more point that we are allowing we decrease the factor

  if (nsampled > max_points_to_display)
  {
    float factor2 = (double)max_points_to_display/(double)nsampled;
    for (auto& cell : index.heap)
    {
      if (!cell.visible) continue;
      cell.factor *= factor2;
      printf(" distance = %.1lf factor %.3f\n", cell.distance, cell.factor);
    }
  }

  // Query the preview points

  for (const auto& cell : index.heap)
  {
    if (!cell.visible) continue;

    double dist = cell.distance;
    int n = cell.preview.size();
    float factor = cell.factor;

    //printf("    camera to cell %d: %.1f, factor %d\n", j, dist, factor);

    for (auto i : cell.preview)
    {
      RenderedPoint p;
      p.pindex = i;
      switch (attr)
      {
        case Attribute::Z:
          if (id(0) == 0)
            p.set_color(r(i), g(i), b(i));
          else
            p.set_color(r(id(i)-1), g(id(i)-1), b(id(i)-1));
          break;
        case Attribute::Distance:
          p.set_color((dmax-dist)/(dmax-dmin)*255, 0, 255-(dmax-dist)/(dmax-dmin)*255);
          break;
        case Attribute::Ratio:
          p.set_color((1.0f-factor)*255, 0, factor*255);
          break;
      }

      pp.emplace_back(p);
    }
  }

  // Query the sampled points

  for (const auto& cell : index.heap)
  {
    if (!cell.visible) continue;
    //if (k > max_points_to_display) break;

    double dist = cell.distance;
    float factor = cell.factor;
    int npoints = cell.points.size();
    int ndisplay = (double)npoints*factor;

    for (int j = 0 ; j < ndisplay ; j++)
    {
      int i = cell.points[j];

      RenderedPoint p;
      p.pindex = i;

      switch (attr)
      {
        case Attribute::Z:
          if (id(0) == 0)
            p.set_color(r(i), g(i), b(i));
          else
            p.set_color(r(id(i)-1), g(id(i)-1), b(id(i)-1));
          break;
        case Attribute::Distance:
          p.set_color((dmax-dist)/(dmax-dmin)*255, 0, 255-(dmax-dist)/(dmax-dmin)*255);
          break;
        case Attribute::Ratio:
          p.set_color((1.0f-factor)*255, 0, factor*255);
          break;
      }

      pp.emplace_back(p);
    }
  }
}

void Drawer::setPointSize(float size)
{
  if (size > 0) this->size = size;
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

