#include "drawer.h"
#include "PSquare.h"

#include <chrono>
#include <random>

const std::vector<std::array<unsigned char, 3>> zgradient = {
  {0, 0, 255},
  {0, 29, 252},
  {0, 59, 250},
  {0, 89, 248},
  {0, 119, 246},
  {0, 148, 244},
  {0, 178, 242},
  {0, 208, 240},
  {0, 238, 238},
  {31, 240, 208},
  {63, 242, 178},
  {95, 244, 148},
  {127, 246, 118},
  {159, 248, 89},
  {191, 250, 59},
  {223, 252, 29},
  {255, 255, 0},
  {255, 223, 0},
  {255, 191, 0},
  {255, 159, 0},
  {255, 127, 0},
  {255, 95, 0},
  {255, 63, 0},
  {255, 31, 0},
  {255, 0, 0}
};

const std::vector<std::array<unsigned char, 3>> classcolor = {
  {211, 211, 211}, // [1]
  {211, 211, 211}, // [2]
  {0,   0,   255}, // [3]
  {50,  205, 50},  // [4]
  {34,  139, 34},  // [5]
  {0,   100, 0},   // [6]
  {255, 0,   0},   // [7]
  {255, 255, 0},   // [8]
  {255, 255, 0},   // [9]
  {100, 149, 237}, // [10]
  {255, 255, 0},   // [11]
  {51,  51,  51},  // [12]
  {255, 255, 0},   // [13]
  {255, 192, 203}, // [14]
  {255, 192, 203}, // [15]
  {160, 32,  240}, // [16]
  {255, 192, 203}, // [17]
  {255, 165, 0},   // [18]
  {255, 255, 0}    // [19]
};

const std::vector<std::array<unsigned char, 3>> igradient = {
  {255,   0,   0},  // [1]
  {255,  14,   0},  // [2]
  {255,  28,   0},  // [3]
  {255,  42,   0},  // [4]
  {255,  57,   0},  // [5]
  {255,  71,   0},  // [6]
  {255,  85,   0},  // [7]
  {255,  99,   0},  // [8]
  {255, 113,   0},  // [9]
  {255, 128,   0},  // [10]
  {255, 142,   0},  // [11]
  {255, 156,   0},  // [12]
  {255, 170,   0},  // [13]
  {255, 184,   0},  // [14]
  {255, 198,   0},  // [15]
  {255, 213,   0},  // [16]
  {255, 227,   0},  // [17]
  {255, 241,   0},  // [18]
  {255, 255,   0},  // [19]
  {255, 255,  21},  // [20]
  {255, 255,  64},  // [21]
  {255, 255, 106},  // [22]
  {255, 255, 149},  // [23]
  {255, 255, 191},  // [24]
  {255, 255, 234}   // [25]
};

Drawer::Drawer(SDL_Window *window, DataFrame df)
{
  this->window = window;

  this->df = df;
  this->x = df["X"];
  this->y = df["Y"];
  this->z = df["Z"];

  this->npoints = x.length();

  this->minx = min(x);
  this->miny = min(y);
  this->minz = min(z);
  this->maxx = max(x);
  this->maxy = max(y);
  this->maxz = max(z);
  this->xcenter = (maxx+minx)/2;
  this->ycenter = (maxy+miny)/2;
  this->zcenter = (maxz+minz)/2;
  this->xrange = maxx-minx;
  this->yrange = maxy-miny;
  this->zrange = maxz-minz;
  this->range = std::max(xrange, yrange);

  this->draw_index = false;
  this->point_budget = 300000;
  this->point_size = 5.0;
  this->lightning = true;

  this->pp.reserve(this->point_budget*1.1);

  double distance = sqrt(xrange*xrange+yrange*yrange);
  this->camera.setDistance(distance);
  this->camera.setPanSensivity(distance*0.001);
  this->camera.setZoomSensivity(distance*0.05);

  setAttribute(Attribute::Z);
  setAttribute(Attribute::RGB);

  auto start = std::chrono::high_resolution_clock::now();

  this->index = EPToctree(&x[0], &y[0], &z[0], x.size());

  for (size_t i = 0 ; i < x.size() ; i++)
  {
    index.insert(i);
    if (i % 1000000 == 0)
    {
      camera.changed = true;
      draw();
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  //printf("Indexation: %.1lf seconds (%.1lfM pts/s)\n", duration.count(), x.size()/duration.count()/1000000);

  this->point_budget *= 10;
  camera.changed = true;
  draw();
}

void Drawer::setAttribute(Attribute x)
{
  if (x == Attribute::RGB && df.containsElementNamed("R"))
  {
    this->attr = x;
    this->r = df["R"];
    this->g = df["G"];
    this->b = df["B"];
    camera.changed = true;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, r.size() - 1);

    rgb_norm = 1;
    for (int i = 0; i < std::min((int)r.size(), 100); ++i)
    {
      int index = dis(gen);
      if (r[index] > 255) rgb_norm = 255;
    }
  }
  else if (x == Attribute::CLASS && df.containsElementNamed("Classification"))
  {
    this->attr = x;
    this->attri = df["Classification"];
    camera.changed = true;
  }
  else if (x == Attribute::I && df.containsElementNamed("Intensity"))
  {
    this->attr = x;
    this->attri = df["Intensity"];
    PSquare p99(0.99);
    for (const auto& i : attri) p99.addDataPoint(i);
    this->minattr = minz;
    this->maxattr = p99.getQuantile();
    this->attrrange = maxattr - minattr;
    camera.changed = true;
  }
  else
  {
    PSquare p99(0.99);
    for (const auto& pz : z) p99.addDataPoint(pz);
    this->attr = Attribute::Z;
    this->minattr = minz;
    this->maxattr = p99.getQuantile();
    this->attrrange = maxattr - minattr;
    camera.changed = true;
  }
}

bool Drawer::draw()
{
  if (!camera.changed)  return false;

  auto start = std::chrono::high_resolution_clock::now();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // Immediate mode. Should be modernized.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glLineWidth(2.0f);
  glPointSize(this->point_size);

  camera.look(); // Reposition the camera after rotation and translation of the scene;

  auto start_query = std::chrono::high_resolution_clock::now();

  compute_cell_visibility();
  query_rendered_point();

  auto end_query = std::chrono::high_resolution_clock::now();
  auto start_rendering = std::chrono::high_resolution_clock::now();

  glBegin(GL_POINTS);

  for (auto i : pp)
  {
    float px = x[i]-xcenter;
    float py = y[i]-ycenter;
    float pz = z[i]-zcenter;

    switch (attr)
    {
      case Attribute::Z:
      {
        float nz = (std::clamp(z[i], minattr, maxattr) - minattr) / (attrrange);
        int bin = std::min(static_cast<int>(nz * (zgradient.size() - 1)), static_cast<int>(zgradient.size() - 1));
        auto& col = zgradient[bin];
        glColor3ub(col[0], col[1], col[2]);
        break;
      }
      case Attribute::RGB:
      {
        glColor3ub(r[i]/rgb_norm, g[i]/rgb_norm, b[i]/rgb_norm);
        break;
      }
      case Attribute::CLASS:
      {
        int classification = std::clamp(attri[i], 0, 19);
        auto& col = classcolor[classification];
        glColor3ub(col[0], col[1], col[2]);
        break;
      }
      case Attribute::I:
      {
        float ni = (std::clamp(attri[i], (int)minattr, (int)maxattr) - (int)minattr) / (attrrange);
        int bin = std::min(static_cast<int>(ni * (igradient.size() - 1)), static_cast<int>(igradient.size() - 1));
        auto& col = igradient[bin];
        glColor3ub(col[0], col[1], col[2]);
        break;
      }
    }

    glVertex3d(px, py, pz);
  }

  glEnd();

  if (lightning)
  {
    eyes_dome_lightning();
  }

  if (draw_index)
  {
    glColor3f(1.0f, 1.0f, 1.0f);

    for (const auto& octant : visible_octants)
    {
      float centerX = octant->bbox[0] - xcenter;
      float centerY = octant->bbox[1] - ycenter;
      float centerZ = octant->bbox[2] - zcenter;
      float halfSize = octant->bbox[3];

      float x0 = centerX - halfSize;
      float x1 = centerX + halfSize;
      float y0 = centerY - halfSize;
      float y1 = centerY + halfSize;
      float z0 = centerZ - halfSize;
      float z1 = centerZ + halfSize;

      glBegin(GL_LINES);

      // Bottom face
      glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0);
      glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1);
      glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
      glVertex3f(x0, y0, z1); glVertex3f(x0, y0, z0);

      // Top face
      glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
      glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1);
      glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
      glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0);

      // Vertical edges
      glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0);
      glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0);
      glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1);
      glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);

      glEnd();
    }
  }

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

  auto end_rendering = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();

  // Calculate the duration
  std::chrono::duration<double> total_duration = end - start;
  std::chrono::duration<double> query_duration = end_query - start_query;
  std::chrono::duration<double> rendering_duration = end_rendering - start_rendering;

  /*printf("Displayed %dk/%ldk points (%.1f\%)\n", (int)pp.size()/1000, x.size()/1000, (double)pp.size()/(double)x.size()*100);
  printf("Full Rendering: %.3f seconds (%.1f fps)\n", total_duration.count(), 1.0f/total_duration.count());
  printf("Cloud rendering: %.3f seconds (%.1f fps, %.1f\%)\n", rendering_duration.count(), 1.0f/rendering_duration.count(), rendering_duration.count()/total_duration.count()*100);
  printf("Spatial query: %.3f seconds (%.1f fps %.1f\%)\n", query_duration.count(), 1.0f/query_duration.count(), query_duration.count()/total_duration.count()*100);
  printf("\n");*/

  glFlush();
  SDL_GL_SwapWindow(window);

  return true;
}

void Drawer::eyes_dome_lightning()
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int w = viewport[2];
  int h = viewport[3];

  std::vector< GLfloat > depth( w * h, 0 );
  glReadPixels( 0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[0] );

  std::vector<GLubyte> colorBuffer(w * h * 3);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &colorBuffer[0]);

  const float zNear = 1;
  const float zFar = 10000;

  std::vector<GLfloat> worldLogDistances(w * h);
  for (int i = 0; i < w * h; ++i)
  {
    GLfloat z = depth[i];           // Depth value from the depth buffer
    GLfloat zNDC = 2.0f * z - 1.0f; // Convert depth value to Normalized Device Coordinate (NDC)
    GLfloat zCamera = (2.0f * zNear * zFar) / (zFar + zNear - zNDC * (zFar - zNear)); // Convert NDC to camera space Z (real-world distance)
    worldLogDistances[i] = std::log2(zCamera);  // Store the real-world log distance
  }

  // Define the 8 possible neighbor offsets in a 2D grid
  /*std::vector<std::pair<int, int>> neighbors = {
    {-1, -1}, {-1, 0}, {-1, 1},
    { 0, -1},          { 0, 1},
    { 1, -1}, { 1, 0}, { 1, 1}
  };*/

  // Define the 4 possible neighbor offsets in a 2D grid
  std::vector<std::pair<int, int>> neighbors = {
             {-1, 0},
    { 0, -1},         { 0, 1},
             { 1, 0},
  };

  // Iterate over each pixel to shade the rendering
  float edlStrength = 10;
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      int idx = y * w + x;

      // Find the maximum log depth among neighbors
      GLfloat maxLogDepth = std::max(0.0f, worldLogDistances[idx]);

      // Compute the response for the current pixel
      GLfloat sum = 0.0f;
      for (const auto& offset : neighbors)
      {
        int nx = x + offset.first;
        int ny = y + offset.second;
        if (nx >= 0 && nx < w && ny >= 0 && ny < h)
        {
          int nIdx = ny * w + nx;
          sum += maxLogDepth - worldLogDistances[nIdx];
        }
      }

      float response = sum/4;
      float shade = std::exp(-response * 300.0 * edlStrength);
      shade = 1-std::clamp(shade, 0.0f, 255.0f)/255.0f;

      colorBuffer[idx * 3] *= shade;
      colorBuffer[idx * 3 + 1] *= shade;
      colorBuffer[idx * 3 + 2] *= shade;
    }
  }

  glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, colorBuffer.data());
}

bool Drawer::is_visible(const EPToctant& octant)
{
  return camera.see(octant.bbox[0]-xcenter, octant.bbox[1]-ycenter, octant.bbox[2]-zcenter, octant.bbox[3]);
}

void Drawer::compute_cell_visibility()
{
  visible_octants.clear();

  EPTkey root = EPTkey::root();
  traverse_and_collect(root, visible_octants);

  std::sort(visible_octants.begin(), visible_octants.end(), [](const EPToctant* a, const EPToctant* b)
  {
    return a->screen_size > b->screen_size;  // Sort in descending order
  });
}

void Drawer::traverse_and_collect(const EPTkey& key, std::vector<EPToctant*>& visible_octants)
{
  auto it = index.registry.find(key);
  if (it == index.registry.end()) return;

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int screenWidth = viewport[2];
  int screenHeight = viewport[3];

  float fov = 45;
  float slope = std::tan(fov/2.0f);

  double cx = camera.x;
  double cy = camera.y;
  double cz = camera.z;

  EPToctant& octant = it->second;

  // Check if the current octant is visible
  if (is_visible(octant))
  {
    // Calculate the screen size or other criteria for visibility
    float x = octant.bbox[0] - xcenter;
    float y = octant.bbox[1] - ycenter;
    float z = octant.bbox[2] - zcenter;
    float radius = octant.bbox[3] * 2 * 1.414f;

    float distance = std::sqrt((cx - x) * (cx - x) + (cy - y) * (cy - y) + (cz - z) * (cz - z));
    octant.screen_size = (screenHeight / 2.0f) * (radius / (slope * distance));

    if (octant.screen_size > 200)
    {
      visible_octants.push_back(&octant);

      // Recurse into children
      std::array<EPTkey, 8> children_keys = key.get_children();
      for (const EPTkey& child_key  : children_keys)
      {
        traverse_and_collect(child_key, visible_octants);
      }
    }
  }
}


void Drawer::query_rendered_point()
{
  pp.clear();

  unsigned int n = 0;
  for (const auto octant : visible_octants)
  {
    n += octant->point_idx.size();
    pp.insert(pp.end(), octant->point_idx.begin(), octant->point_idx.end());
    if (n > point_budget) break;
  }
}

void Drawer::setPointSize(float size)
{
  if (size > 0) this->point_size = size;
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

