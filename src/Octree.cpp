#include "Octree.h"

#include <cstdio>
#include <cmath>
#include <chrono>
#include <algorithm>

EPTkey::EPTkey(int d, int x, int y, int z) : d(d), x(x), y(y), z(z) {}
EPTkey::EPTkey() : EPTkey(-1, -1, -1, -1) {}

std::array<EPTkey, 8> EPTkey::get_children() const
{
  std::array<EPTkey, 8> children;
  for (unsigned char direction = 0 ; direction < 8 ; direction++)
  {
    EPTkey key(*this);

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

EPTkey EPTkey::get_parent() const
{
  if (!is_valid()) return EPTkey();
  if (d == 0) return EPTkey();
  return EPTkey(d - 1, x >> 1, y >> 1, z >> 1);
}

EPToctant::EPToctant()
{
  bbox[0] = 0;
  bbox[1] = 0;
  bbox[2] = 0;
  bbox[3] = 0;
  screen_size = 0;
}

void EPToctant::insert(size_t idx, int cell)
{
  point_idx.push_back(idx);
  if (cell >= 0) occupancy.insert(cell); // cell = -1 means that recording the location of the point is useless (save memory)
};

EPToctree::EPToctree(double* x, double* y, double* z, size_t n)
{
  auto start = std::chrono::high_resolution_clock::now();

  this->npoint = n;
  this->x = x;
  this->y = y;
  this->z = z;

  this->point_spacing = 0;
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

  for (size_t i = 0 ; i < n ; i++)
  {
    insert(i);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  printf("Indexation: %.1lf seconds (%.1lfM pts/s)\n", duration.count(), n/duration.count()/1000000);
}

void EPToctree::compute_max_depth(size_t npts, size_t max_points_per_octant)
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

bool EPToctree::insert(size_t i)
{
  std::unordered_map<EPTkey, EPToctant, EPTKeyHasher>::iterator it;

  // Search a place to insert the point
  int lvl = 0;
  int cell = 0;
  bool accepted = false;
  while (!accepted)
  {
    EPTkey key = get_key(x[i], y[i], z[i], lvl);

    if (lvl == max_depth)
      cell = -1; // Do not build an occupancy grid for last level. Point must be inserted anyway.
    else
      cell = get_cell(x[i], y[i], z[i], key);

    //printf("Suggested key %d-%d-%d-%d in cell %d\n", key.x, key.y, key.z, key.d, cell);

    it = registry.find(key);
    if (it == registry.end())
    {
      //printf("Registry add key %d-%d-%d-%d\n", key.x, key.y, key.z, key.d);
      EPToctant node;
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

EPTkey EPToctree::get_key(double x, double y, double z, int depth) const
{
  int grid_size = 1 << depth;  // 2^depth
  double grid_resolution = (xmax - xmin) / grid_size;

  int xi = static_cast<int>((x - xmin) / grid_resolution);
  int yi = static_cast<int>((y - ymin) / grid_resolution);
  int zi = static_cast<int>((z - zmin) / grid_resolution);

  xi = std::clamp(xi, 0, grid_size - 1);
  yi = std::clamp(yi, 0, grid_size - 1);
  zi = std::clamp(zi, 0, grid_size - 1);

  return EPTkey(depth, xi, yi, zi);
}

void EPToctree::set_bbox(const EPTkey& key, double* bb)
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

int EPToctree::get_cell(double x, double y, double z, const EPTkey& key) const
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
  xi = (std::min)((std::max)(0, xi), grid_size - 1);
  yi = (std::min)((std::max)(0, yi), grid_size - 1);
  zi = (std::min)((std::max)(0, zi), grid_size - 1);

  return zi * grid_size * grid_size + yi * grid_size + xi;
}
