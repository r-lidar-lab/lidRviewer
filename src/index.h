#ifndef GP_H
#define GP_H

#include <Rcpp.h>
#include <limits>

#define INFD std::numeric_limits<double>::infinity();

struct Cell
{
  Cell()  { min = INFD ; max = -INFD; };
  double min;
  double max;

  std::vector<int> idx;
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

inline GridPartition::GridPartition()
{
}

inline GridPartition::GridPartition(const Rcpp::NumericVector x, const Rcpp::NumericVector y, const Rcpp::NumericVector z)
{
  if (x.size() != y.size())
    Rcpp::stop("Internal error in spatial index: x and y have different sizes."); // # nocov

  if (x.size() != z.size())
    Rcpp::stop("Internal error in spatial index: x and z have different sizes."); // # nocov

  // Number of points
  npoints = x.size();

  build(x,y,z);
}

inline void GridPartition::build(const Rcpp::NumericVector x, const Rcpp::NumericVector y,  const Rcpp::NumericVector z)
{
  //if (npoints == 0)
  //  Rcpp::stop("Internal error in spatial index: impossible to build an index with 0 points."); // # nocov

  // Compute the bounding box
  xmin =  INFD;
  xmax = -INFD;
  ymin =  INFD;
  ymax = -INFD;
  zmin =  INFD;
  zmax = -INFD;

  for (auto i = 0 ; i < x.size() ; i++)
  {
    if (x[i] < xmin) xmin = x[i];
    if (x[i] > xmax) xmax = x[i];
    if (y[i] < ymin) ymin = y[i];
    if (y[i] > ymax) ymax = y[i];
    if (z[i] < zmin) zmin = z[i];
    if (z[i] > zmax) zmax = z[i];
  }

  double buf = 1;
  xmin -= buf;
  xmax += buf;
  ymin -= buf;
  ymax += buf;
  zmin -= buf;
  zmax += buf;

  // Historically the spatial index was a quadtree defined by a depth
  // The depth is still used to compute the number of cells
  ncells = 1000;

  // Compute some indicator of shape
  double xrange = xmax - xmin;
  double yrange = ymax - ymin;
  double zrange = zmax - zmin;
  double xyratio = xrange/yrange;
  double xzratio = xrange/zrange;
  //double yzratio = yrange/zrange;

  // Compute the number of rows and columns in such a way that there is approximately
  // the number of wanted cells but the organization of the cell is well balanced
  // so the resolutions on x-y are close. We want:
  // ncols/nrows = xyratio
  // ncols*nrows = ncells
  ncols = std::round(std::sqrt(ncells*xyratio));
  if (ncols <= 0) ncols = 1;
  nrows = std::round(ncols/xyratio);
  if (nrows <= 0) nrows = 1;
  nlayers = 1;

  ncells = ncols*nrows*nlayers;

  xres = xrange / (double)ncols;
  yres = yrange / (double)nrows;
  zres = zrange / (double)nlayers;

  printf("ncells = %d res = %.1lf, %.1lf, %.1lf\n", ncells, xres, yres, zres);

  area = xrange * yrange;
  volume = area * zrange;

  heap.resize(ncells);
  for (auto i = 0 ; i < x.size() ; i++)
  {
    int idx = get_cell(x[i], y[i], z[i]);
    Cell& cell = heap[idx];
    if (cell.max < z[i]) cell.max = z[i];
    if (cell.min > z[i]) cell.min = z[i];
    cell.idx.push_back(i);
  }
}

inline int GridPartition::get_cell(double x, double y, double z)
{
  int col = std::floor((x - xmin) / xres);
  int row = std::floor((ymax - y) / yres);
  int lay = std::floor((z - zmin) / zres);
  if (row < 0 || row > (int)nrows-1 || col < 0 || col > (int)ncols-1 || lay < 0 || lay > (int)nlayers-1)
    Rcpp::stop("Internal error in spatial index: point out of the range."); // # nocov
  int cell = lay * nrows * ncols + row * ncols + col;
  if (cell < 0 || cell >= (int)ncells)
    Rcpp::stop("Internal error in spatial index: cell out of the range."); // # nocov
  return cell;
}

inline void GridPartition::xyz_from_cell(int cell, float& x, float& y, float& z)
{
  int col = cell%ncols;
  int row = std::floor(cell/ncols);

  x = xmin + ((col+0.5) * xres);
  y = ymax - ((row+0.5) * yres);
  z = (zmax+zmin)/2;
}

#endif
