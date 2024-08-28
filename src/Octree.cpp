#include "Octree.h"

#include <cstdio>
#include <cmath>
#include <limits>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <stdexcept>

Key::Key(int32_t d, int32_t x, int32_t y, int32_t z) : d(d), x(x), y(y), z(z) {}
Key::Key() : Key(-1, -1, -1, -1) {}

std::array<Key, 8> Key::get_children() const
{
  std::array<Key, 8> children;
  for (unsigned char direction = 0 ; direction < 8 ; direction++)
  {
    Key key(*this);

    key.d++;
    key.x *= 2;
    key.y *= 2;
    key.z *= 2;

    if (direction & (((unsigned char)1) << 0)) key.x++;
    if (direction & (((unsigned char)1) << 1)) key.y++;
    if (direction & (((unsigned char)1) << 2)) key.z++;

    children[direction] = key;
  }

  return children;
}

Key Key::get_parent() const
{
  if (!is_valid()) return Key();
  if (d == 0) return Key();
  return Key(d - 1, x >> 1, y >> 1, z >> 1);
}

Node::Node()
{
  bbox[0] = 0;
  bbox[1] = 0;
  bbox[2] = 0;
  bbox[3] = 0;
  screen_size = 0;
}

void Node::insert(size_t idx, uint32_t cell)
{
  point_idx.push_back(idx);
  if (cell >= 0) occupancy.insert(cell); // cell = -1 means that recording the location of the point is useless (save memory)
};

Octree::Octree(double* x, double* y, double* z, size_t n)
{
  if (n > UINT32_MAX)
    throw std::runtime_error("Spatial indexation is bound to 4,294 billion points");

  this->npoint = n;
  this->x = x;
  this->y = y;
  this->z = z;

  this->max_depth = 0;
  this->grid_size = 128;

  // Compute the bounding box
  xmin =  INFD;
  xmax = -INFD;
  ymin =  INFD;
  ymax = -INFD;
  zmin =  INFD;
  zmax = -INFD;
  for (size_t i = 0 ; i < n ; i++)
  {
    if (x[i] < xmin) xmin = x[i];
    if (x[i] > xmax) xmax = x[i];
    if (y[i] < ymin) ymin = y[i];
    if (y[i] > ymax) ymax = y[i];
    if (z[i] < zmin) zmin = z[i];
    if (z[i] > zmax) zmax = z[i];
  }

  double center_x = (xmin+xmax)/2;
  double center_y = (ymin+ymax)/2;
  double center_z = (zmin+zmax)/2;
  double halfsize = MAX(xmax-xmin, ymax-ymin, zmax-zmin)/2;

  xmin = center_x - halfsize;
  ymin = center_y - halfsize;
  zmin = center_z - halfsize;
  xmax = center_x + halfsize;
  ymax = center_y + halfsize;
  zmax = center_z + halfsize;

  compute_max_depth(n, 10000);
}

void Octree::compute_max_depth(uint64_t npts, size_t max_points_per_octant)
{
  // strategy to regulate the maximum depth of the octree
  double xsize = xmax-xmin;
  double ysize = ymax-ymin;
  double zsize = zmax-zmin;
  double size  = MAX(xsize, ysize, zsize);

  max_depth = 0;

  while (npts > max_points_per_octant)
  {
    if (xsize >= size) { npts /= 2; }
    if (ysize >= size) { npts /= 2; }
    if (zsize >= size) { npts /= 2; }
    size /= 2;
    max_depth++;
  }

  //printf("Max depth = %d\n", max_depth);
}

bool Octree::insert(uint32_t i)
{
  std::unordered_map<Key, Node, KeyHasher>::iterator it;

  // Search a place to insert the point
  int lvl = 0;
  int cell = 0;
  bool accepted = false;
  while (!accepted)
  {
    Key key = get_key(x[i], y[i], z[i], lvl);

    if (lvl == max_depth)
      cell = -1; // Do not build an occupancy grid for last level. Point must be inserted anyway.
    else
      cell = get_cell(x[i], y[i], z[i], key);

    //printf("Suggested key %d-%d-%d-%d in cell %d\n", key.x, key.y, key.z, key.d, cell);

    it = registry.find(key);
    if (it == registry.end())
    {
      //printf("Registry add key %d-%d-%d-%d\n", key.x, key.y, key.z, key.d);
      Node node;
      set_bbox(key, node.bbox);
      it = registry.emplace(key, node).first;
    }

    auto it2 = it->second.occupancy.find(cell);
    accepted = (it2 == it->second.occupancy.end()) || (lvl == max_depth);

    lvl++;
  }

  it->second.insert(i, cell);

  //if (it->first.d == 0)
  //printf("Insert point i = %lu (%.1lf, %.1lf, %.1lf) in %d-%d-%d-%d in cell %d n = %lu\n", i,  x[i], y[i], z[i], it->first.x, it->first.y, it->first.z, it->first.d, cell, it->second.point_idx.size());

  return true;
}

Key Octree::get_key(double x, double y, double z, int depth) const
{
  int grid_size = 1 << depth;  // 2^depth
  double grid_resolution = (xmax - xmin) / grid_size;

  int xi = static_cast<int>((x - xmin) / grid_resolution);
  int yi = static_cast<int>((y - ymin) / grid_resolution);
  int zi = static_cast<int>((z - zmin) / grid_resolution);

  xi = std::clamp(xi, 0, grid_size - 1);
  yi = std::clamp(yi, 0, grid_size - 1);
  zi = std::clamp(zi, 0, grid_size - 1);

  return Key(depth, xi, yi, zi);
}

void Octree::set_bbox(const Key& key, double* bb)
{
  double size = get_halfsize()*2;
  double res  = size / (1 << key.d);

  double minx = res * key.x + (get_center_x() - get_halfsize());
  double miny = res * key.y + (get_center_y() - get_halfsize());
  double minz = res * key.z + (get_center_z() - get_halfsize());
  double maxx = minx + res;
  double maxy = miny + res;
  double maxz = minz + res;

  bb[0] = (minx+maxx)/2;
  bb[1] = (miny+maxy)/2;
  bb[2] = (minz+maxz)/2;
  bb[3] = res/2;

  return;
}

int Octree::get_cell(double x, double y, double z, const Key& key) const
{
  double size = get_halfsize()*2;
  double res  = size / (1 << key.d);

  double minx = res * key.x + (get_center_x() - get_halfsize());
  double miny = res * key.y + (get_center_y() - get_halfsize());
  double minz = res * key.z + (get_center_z() - get_halfsize());
  double maxx = minx + res;

  // Get cell id in this octant
  double grid_resolution = (maxx - minx) / grid_size;
  int xi = (int)std::floor((x - minx) / grid_resolution);
  int yi = (int)std::floor((y - miny) / grid_resolution);
  int zi = (int)std::floor((z - minz) / grid_resolution);
  xi = std::clamp(xi, 0, grid_size - 1);
  yi = std::clamp(yi, 0, grid_size - 1);
  zi = std::clamp(zi, 0, grid_size - 1);

  return zi * grid_size * grid_size + yi * grid_size + xi;
}

const std::string FILE_SIGNATURE = "HNOF";
const int FILE_VERSION_MAJOR = 1;
const int FILE_VERSION_MINOR = 0;

// Write function
void Octree::write(const std::string& filename)
{
  printf("write\n");
  std::ofstream outFile(filename, std::ios::binary);

  if (!outFile)
    throw std::runtime_error("Failed to open file for writing: " + filename);

  // Write file signature
  outFile.write(FILE_SIGNATURE.c_str(), FILE_SIGNATURE.size());

  // Write file version
  outFile.write(reinterpret_cast<const char*>(&FILE_VERSION_MAJOR), 4);
  outFile.write(reinterpret_cast<const char*>(&FILE_VERSION_MINOR), 4);

  // Write the bounding box
  outFile.write(reinterpret_cast<const char*>(&xmin), 8);
  outFile.write(reinterpret_cast<const char*>(&ymin), 8);
  outFile.write(reinterpret_cast<const char*>(&zmin), 8);
  outFile.write(reinterpret_cast<const char*>(&xmax), 8);
  outFile.write(reinterpret_cast<const char*>(&ymax), 8);
  outFile.write(reinterpret_cast<const char*>(&zmax), 8);

  // Write the grid spacing
  outFile.write(reinterpret_cast<const char*>(&zmax), 8);

  // Write the size of the unordered_map
  std::uint64_t mapSize = registry.size(); // Use uint64_t for large sizes
  outFile.write(reinterpret_cast<const char*>(&mapSize), 8);

  // Iterate through each key-value pair
  for (const auto& pair : registry)
  {
    // Write each int of Key
    outFile.write(reinterpret_cast<const char*>(&pair.first.d), 4);
    outFile.write(reinterpret_cast<const char*>(&pair.first.x), 4);
    outFile.write(reinterpret_cast<const char*>(&pair.first.y), 4);
    outFile.write(reinterpret_cast<const char*>(&pair.first.z), 4);

    // Write the size of the vector<int> (octant)
    size_t vectorSize = pair.second.point_idx.size();
    outFile.write(reinterpret_cast<const char*>(&vectorSize), sizeof(size_t));

    // Write the vector<int> data
    outFile.write(reinterpret_cast<const char*>(pair.second.point_idx.data()), vectorSize * 4);
  }

  outFile.close();
}

// Read function
bool Octree::read(const std::string& filename)
{
  printf("read\n");

  std::ifstream inFile(filename, std::ios::binary);

  if (!inFile)
    throw std::runtime_error("Failed to open file for reading: " + filename);

  // Read and validate file signature
  char signature[FILE_SIGNATURE.size()];
  inFile.read(signature, FILE_SIGNATURE.size());
  if (std::string(signature, FILE_SIGNATURE.size()) != FILE_SIGNATURE)
    throw std::runtime_error("Invalid file signature.");

  // Read and validate file version
  int fileVersionMajor;
  inFile.read(reinterpret_cast<char*>(&fileVersionMajor), 4);
  int fileVersionMinor;
  inFile.read(reinterpret_cast<char*>(&fileVersionMinor), 4);
  if (fileVersionMajor != 1 || fileVersionMinor != 0)
    throw std::runtime_error(std::string("Unsupported file version: ") + std::to_string(fileVersionMajor) + "." + std::to_string(fileVersionMinor));

  // Read the size of the unordered_map
  uint64_t mapSize;
  inFile.read(reinterpret_cast<char*>(&mapSize), 8);

  // Read the bbox
  inFile.read(reinterpret_cast<char*>(&xmin), 8);
  inFile.read(reinterpret_cast<char*>(&ymin), 8);
  inFile.read(reinterpret_cast<char*>(&zmin), 8);
  inFile.read(reinterpret_cast<char*>(&xmax), 8);
  inFile.read(reinterpret_cast<char*>(&ymax), 8);
  inFile.read(reinterpret_cast<char*>(&zmax), 8);

  // Read the grid spacing
  inFile.read(reinterpret_cast<char*>(&grid_size), 4);

  // Clear the existing map
  registry.clear();

  // Read each key-value pair
  npoint = 0;
  max_depth = 0;
  for (size_t i = 0; i < mapSize; ++i)
  {
    // Read Key (4 integers)
    Key key;
    inFile.read(reinterpret_cast<char*>(&key), sizeof(Key));

    if (key.d > max_depth) max_depth = key.d;

    // Read the size of the vector<int>
    uint64_t vectorSize;
    inFile.read(reinterpret_cast<char*>(&vectorSize), 8);

    npoint += vectorSize;

    Node octant;
    set_bbox(key, octant.bbox);
    octant.point_idx.resize(vectorSize);

    // Read the vector<int> data
    inFile.read(reinterpret_cast<char*>(&(octant.point_idx[0])), vectorSize * 4);

    // Insert the pair into the unordered_map
    registry.emplace(key, std::move(octant));
  }

  inFile.close();
  return true;
}
