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
  this->max_points_to_display = 2000000;

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

  float tmin = 3;
  float tmax = 7;
  int nsampled = 0;
  for (auto& cell : index.heap)
  {
    if (!cell.visible) continue;

    double distance = cell.distance/dmin;

    float factor = 0;
    if (distance <= tmin)
      factor = 1.0f;
    else if (distance >= tmax)
      factor = 0.0f;
    else
      factor = 1.0f - (distance - tmin) / (tmax - tmin);

    cell.factor = std::pow(factor, 6);

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


/*if (postprod)
 {
 std::vector<unsigned char> pixelData(width * height * 4); // Assuming RGBA
 glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

 // Step 3: Identify and fill holes
 int maxIterations = 5; // Number of passes to make
 for (int iter = 0; iter < maxIterations; ++iter) {
 for (int y = WS; y < height - WS; ++y) {
 for (int x = WS; x < width - WS; ++x) {
 int index = (y * width + x) * 4;
 if (isBackgroundPixel(pixelData[index], pixelData[index + 1], pixelData[index + 2], pixelData[index + 3]))
 {
 //pixelData[index] = 0; pixelData[index + 1] = 255; pixelData[index + 2] = 0;
 if (hasSurroundingForegroundPixels(pixelData, x, y, width)) {
 //pixelData[index] = 255; pixelData[index + 1] = 255; pixelData[index + 2] = 255;
 fillHole(pixelData, x, y, width);
 }
 }
 }
 }
 }

 glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());
 }*/


// depth-buffer edge detection
/*std::vector< GLfloat > depth( width * height, 0 );
 glReadPixels( 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[0] );

 // Linearize depth and convert to unsigned byte
 std::vector<GLubyte> depthImage(width * height);
 for (size_t i = 0; i < depth.size(); ++i) {
 float linearDepth = (2.0f * zNear) / (zFar + zNear - depth[i] * (zFar - zNear));
 depthImage[i] = static_cast<GLubyte>(linearDepth * 255.0f);  // Map to [0, 255]
 }

 int Gx[3][3] = {
 {-1, 0, 1},
 {-2, 0, 2},
 {-1, 0, 1}
 };

 int Gy[3][3] = {
 {-1, -2, -1},
 { 0,  0,  0},
 { 1,  2,  1}
 };

 std::vector<GLubyte> edgeImage(width * height, 0);

 for (int y = 1; y < height - 1; ++y) {
 for (int x = 1; x < width - 1; ++x) {
 float gx = 0.0f;
 float gy = 0.0f;

 for (int i = -1; i <= 1; ++i) {
 for (int j = -1; j <= 1; ++j) {
 gx += Gx[i + 1][j + 1] * depthImage[(y + i) * width + (x + j)];
 gy += Gy[i + 1][j + 1] * depthImage[(y + i) * width + (x + j)];
 }
 }

 float gradient = sqrt(gx * gx + gy * gy);
 edgeImage[y * width + x] = static_cast<GLubyte>(std::min(gradient, 255.0f)); // Clamp to [0, 255]
 }
 }

 // Step 4: Replace values >= 255 with 0
 for (size_t i = 0; i < edgeImage.size(); ++i) {
 if (edgeImage[i] >= 220) {
 edgeImage[i] = 0;
 }
 }

 // Step 5: Find the new maximum value
 GLubyte newMaxValue = 0;
 for (size_t i = 0; i < edgeImage.size(); ++i) {
 if (edgeImage[i] > newMaxValue) {
 newMaxValue = edgeImage[i];
 }
 }

 // Step 6: Rescale the image to the range [0, 255]
 if (newMaxValue > 0) {  // Avoid division by zero
 for (size_t i = 0; i < edgeImage.size(); ++i) {
 edgeImage[i] = static_cast<GLubyte>((edgeImage[i] * 255.0f) / newMaxValue);
 }
 }

 // Step 7: Create the shadow image
 std::vector<GLubyte> shadowImage(width * height * 4, 0);  // Initialize as white with full alpha

 for (int y = 0; y < height; ++y) {
 for (int x = 0; x < width; ++x) {
 size_t index = y * width + x;
 if (edgeImage[index] == 0) {
 // Set to black with some alpha value (e.g., 128 for semi-transparency)
 shadowImage[index * 4 + 0] = 0;
 shadowImage[index * 4 + 1] = 0;
 shadowImage[index * 4 + 2] = 0;
 shadowImage[index * 4 + 3] = 128;  // Alpha value (0-255, where 0 is fully transparent and 255 is fully opaque)
 }
 }
 }

 // Step 8: Enable blending and set blend function
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 // Step 9: Render the shadow image
 glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, shadowImage.data());
 */

/*
 * #define WS 5

 bool isBackgroundPixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
 // Define what you consider as a background pixel.
 // Example: A black pixel with full transparency.
 return (r < 200 && g < 200 && b < 200);
 }

 bool hasSurroundingForegroundPixels(const std::vector<unsigned char>& pixelData, int x, int y, int width) {
 int offsets[] = {-WS, 0, WS};  // To iterate over neighboring pixels
 int n = 0;
 for (int dy : offsets) {
 for (int dx : offsets) {
 if (dx == 0 && dy == 0) continue; // Skip the center pixel itself

 int neighborIndex = ((y + dy) * width + (x + dx)) * 4; // Calculate neighbor's index in the array
 unsigned char r = pixelData[neighborIndex];
 unsigned char g = pixelData[neighborIndex + 1];
 unsigned char b = pixelData[neighborIndex + 2];
 unsigned char a = pixelData[neighborIndex + 3];

 if (!isBackgroundPixel(r, g, b, a)) {
 n++;
 }
 }
 }
 if (n >= 8) return true;
 return false; // No foreground pixels around
 }

 void fillHole(std::vector<unsigned char>& pixelData, int x, int y, int width) {
 int offsets[] = {-WS, 0, WS};  // To iterate over neighboring pixels
 int rSum = 0, gSum = 0, bSum = 0, aSum = 0;
 int count = 0;

 // Calculate the sum of the colors of the surrounding foreground pixels
 for (int dy : offsets) {
 for (int dx : offsets) {
 if (dx == 0 && dy == 0) continue; // Skip the center pixel itself

 int neighborIndex = ((y + dy) * width + (x + dx)) * 4;
 unsigned char r = pixelData[neighborIndex];
 unsigned char g = pixelData[neighborIndex + 1];
 unsigned char b = pixelData[neighborIndex + 2];
 unsigned char a = pixelData[neighborIndex + 3];

 if (!isBackgroundPixel(r, g, b, a)) {
 rSum += r;
 gSum += g;
 bSum += b;
 aSum += a;
 count++;
 }
 }
 }

 // Avoid division by zero
 if (count > 0) {
 int index = (y * width + x) * 4;
 pixelData[index]     = rSum / count;
 pixelData[index + 1] = gSum / count;
 pixelData[index + 2] = bSum / count;
 pixelData[index + 3] = aSum / count;
 }
 }*/

