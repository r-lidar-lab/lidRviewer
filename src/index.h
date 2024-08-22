#ifndef GP_H
#define GP_H

#include <Rcpp.h>
#include <limits>

#define INFD std::numeric_limits<double>::infinity();

struct Voxel
{
  int i, j, k;
  Voxel() : i(0), j(0), k(0) {}
  Voxel(int i, int j, int k) : i(i), j(j), k(k) {}
  size_t hash() const
  {
    // Create individual hashes for x and y
    size_t hashi = std::hash<int>{}(i);
    size_t hashj = std::hash<int>{}(j);
    size_t hashk = std::hash<int>{}(k);

    // Combine the individual hashes using a hash combiner
    size_t seed = 0;
    seed ^= hashi + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hashj + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hashk + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
  };
  bool operator==(const Voxel& other) const { return i == other.i && j == other.j && k == other.k; };
};

struct VoxelHash
{
  std::size_t operator()(const Voxel& voxel) const { return voxel.hash() ;}
};

struct Cell
{
  Cell()  { min = INFD ; max = -INFD; distance = 0; factor = 0; visible = false; };
  double min;
  double max;
  double distance;
  float factor;
  bool visible;
  std::vector<int> preview;
  std::vector<int> points;
};
/*
 * Spatial index using a grid-based indexation. The grid-based indexation
 * can be extended with multiple layers to become a voxel-based indexation.
 * public members are:
 * - Constructors
 * - Lookup (templated to search any arbitrary shape)
 * - knn (both in 2D or 3D)
 */
class GridPartition
{
public:
  GridPartition();
  GridPartition(const Rcpp::NumericVector, const Rcpp::NumericVector, const Rcpp::NumericVector);

  unsigned int  npoints;
  unsigned int ncols, nrows, nlayers, ncells;
  double xmin,ymin,xmax,ymax,zmin,zmax;
  double xres, yres, zres;
  double area, volume;
  std::vector<Cell> heap;

  int get_cell(double, double, double);
  void xyz_from_cell(int cell, float&, float&, float&);

private:

  void build(const Rcpp::NumericVector, const Rcpp::NumericVector, const Rcpp::NumericVector);
};

#endif
