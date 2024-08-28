#ifndef OCTREE_H
#define OCTREE_H

#include <cstddef>
#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#define MAX(a, b, c) ((a) <= (b)? (b) <= (c)? (c) : (b) : (a) <= (c)? (c) : (a))
#define INFD std::numeric_limits<double>::infinity();

struct Key
{
  Key();
  Key(int32_t d, int32_t x, int32_t y, int32_t z);
  static Key root() { return Key(0, 0, 0, 0); }
  bool is_valid() const { return d >= 0 && x >= 0 && y >= 0 && z >= 0; }
  std::array<Key, 8> get_children() const;
  Key get_parent() const;

  int32_t d;
  int32_t x;
  int32_t y;
  int32_t z;
};

struct KeyHasher
{
  // PDAL hash method copied
  size_t operator()(const Key &k) const
  {
    std::hash<size_t> h;
    size_t k1 = ((size_t)k.d << 32) | k.x;
    size_t k2 = ((size_t)k.y << 32) | k.z;
    return h(k1) ^ (h(k2) << 1);
  }
};

inline bool operator==(const Key& a, const Key& b) { return a.d == b.d && a.x == b.x && a.y == b.y && a.z == b.z; }
inline bool operator!=(const Key& a, const Key& b) { return !(a == b); }
inline bool operator<(const Key& a, const Key& b)
{
  if (a.x < b.x) return true;
  if (a.x > b.x) return false;
  if (a.y < b.y) return true;
  if (a.y > b.y) return false;
  if (a.z < b.z) return true;
  if (a.z > b.z) return false;
  if (a.d < b.d) return true;
  if (a.d > b.d) return false;
  return false;
}

struct Node : public Key
{
  Node();
  void insert(size_t idx, uint32_t cell);
  size_t npoints() const {return point_idx.size(); };

  // Bounding box of the entry
  double bbox[4];
  float screen_size;

  std::vector<uint32_t> point_idx;
  std::unordered_set<uint32_t> occupancy;
};

class Octree
{
public:
  Octree() = default;
  Octree(double* x, double* y, double* z, size_t n);
  Key get_key(double x, double y, double z, int depth) const;
  int get_cell(double x, double y, double z, const Key& key) const;
  inline int get_max_depth() const { return max_depth; };
  inline double get_center_x() const { return (xmin+xmax)/2; };
  inline double get_center_y() const { return (ymin+ymax)/2; };
  inline double get_center_z() const { return (zmin+zmax)/2; };
  inline double get_halfsize() const { return (xmax-xmin)/2; };
  inline double get_size() const { return xmax-xmin; };
  inline double get_xmin() const { return xmin; };
  inline double get_ymin() const { return ymin; };
  inline double get_zmin() const { return zmin; };
  inline double get_xmax() const { return xmax; };
  inline double get_ymax() const { return ymax; };
  inline double get_zmax() const { return zmax; };
  inline uint32_t get_npoints() const { return npoint; };
  inline int get_gridsize() const { return grid_size; };
  void set_bbox(const Key& key, double* bb);
  inline void set_gridsize(int32_t size) { if (size > 2) grid_size = size; };
  void write(const std::string& filename);
  bool read(const std::string& filename);

  bool insert(uint32_t i);
  std::unordered_map<Key, Node, KeyHasher> registry;

private:
  void compute_max_depth(size_t npts, size_t max_points_per_octant);

private:
  double* x;
  double* y;
  double* z;
  uint32_t npoint;

  double xmin;
  double ymin;
  double zmin;
  double xmax;
  double ymax;
  double zmax;

  int32_t max_depth;
  int32_t grid_size;
};

#endif
