#include "index.h"

#include <chrono>
#include <random>

#define ROUNDANY(x,m) round((x) / m) * m

static void shuffle(std::vector<int>& x, int shuffle_size)
{
  std::mt19937 rng(0);

  if (shuffle_size > 0)
  {
    int s = x.size();
    for (int i = 0; i < s; i += shuffle_size/2)
    {
      int start = std::max(0, i - shuffle_size / 2);
      int end = std::min(s, i + shuffle_size / 2 + 1);
      std::shuffle(x.begin() + start, x.begin() + end, rng);
    }
  }
};

GridPartition::GridPartition()
{
}

GridPartition::GridPartition(const Rcpp::NumericVector x, const Rcpp::NumericVector y, const Rcpp::NumericVector z)
{
  auto start = std::chrono::high_resolution_clock::now();

  if (x.size() != y.size())
    Rcpp::stop("Internal error in spatial index: x and y have different sizes."); // # nocov

    if (x.size() != z.size())
      Rcpp::stop("Internal error in spatial index: x and z have different sizes."); // # nocov

    // Number of points
  npoints = x.size();

  build(x,y,z);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  std::cout << "Time taken for indexation: " << duration.count() << " seconds" << std::endl;
}

void GridPartition::build(const Rcpp::NumericVector x, const Rcpp::NumericVector y,  const Rcpp::NumericVector z)
{
  // Compute the bounding box
  // ========================
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

  // Compute some indicator of shape
  double xrange = xmax - xmin;
  double yrange = ymax - ymin;
  double zrange = zmax - zmin;
  double xyratio = xrange/yrange;
  double xzratio = xrange/zrange;
  //double yzratio = yrange/zrange;

  area = xrange * yrange;
  volume = xrange * yrange * zrange;

  // Create a registry for the voxel sampling preview
  // ================================================

  size_t nvoxels = 250000;
  double vres = std::cbrt(volume/nvoxels);

  double rxmin = ROUNDANY(xmin - 0.5 * vres, vres);
  double rymin = ROUNDANY(ymin - 0.5 * vres, vres);
  double rzmin = ROUNDANY(zmin - 0.5 * vres, vres);
  double rxmax = ROUNDANY(xmax + 0.5 * vres, vres);
  double rymax = ROUNDANY(ymax + 0.5 * vres, vres);
  double rzmax = ROUNDANY(zmax + 0.5 * vres, vres);

  int length = (rxmax - rxmin)/vres;
  int width  = (rymax - rymin)/vres;
  int height = (rzmax - rzmin)/vres;

  nvoxels = (size_t)length*(size_t)width*(size_t)height;

  std::vector<bool> bitregistry;
  bitregistry.resize(nvoxels);

  // Create a registry for the grid indexation
  // =========================================

  ncells = 1000;
  double cres = std::sqrt(area/ncells);

  xmin = ROUNDANY(xmin - 0.5 * vres, vres);
  ymin = ROUNDANY(ymin - 0.5 * vres, vres);
  zmin = ROUNDANY(zmin - 0.5 * vres, vres);
  xmax = ROUNDANY(xmax + 0.5 * vres, vres);
  ymax = ROUNDANY(ymax + 0.5 * vres, vres);
  zmax = ROUNDANY(zmax + 0.5 * vres, vres);

  xrange = xmax - xmin;
  yrange = ymax - ymin;
  zrange = zmax - zmin;
  xyratio = xrange/yrange;
  xzratio = xrange/zrange;

  ncols = std::round(xrange/cres);
  nrows = std::round(yrange/cres);
  nlayers = 1;

  ncells = ncols*nrows*nlayers;
  heap.resize(ncells);

  xres = xrange / (double)ncols;
  yres = yrange / (double)nrows;
  zres = zrange / (double)nlayers;

  printf("ncells = %d vres = %.1lf, %.1lf, %.1lf\n", ncells, xres, yres, zres);

  area = xrange * yrange;
  volume = area * zrange;


  // Index the points either in preview or regular
  // =========================================

  std::vector<int> index(x.size());
  std::iota(index.begin(), index.end(), 0);
  //shuffle(index, 5000000);
  std::mt19937 rng(0);
  std::shuffle(index.begin(), index.end(), rng);


  for (auto i : index)
  {
    // Voxel of this point
    int nx = std::floor((x[i] - rxmin) / vres);
    int ny = std::floor((y[i] - rymin) / vres);
    int nz = std::floor((z[i] - rzmin) / vres);
    int key = nx + ny*length + nz*length*width;
    int idx = get_cell(x[i], y[i], z[i]);
    bool vox_occupied = bitregistry[key];

    Cell& cell = heap[idx];;

    if (!vox_occupied)
    {
      bitregistry[key] = true;
      cell.preview.push_back(i);
    }
    else
    {
      cell.points.push_back(i);
    }

    if (cell.max < z[i]) cell.max = z[i];
    if (cell.min > z[i]) cell.min = z[i];
  }
}

int GridPartition::get_cell(double x, double y, double z)
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

void GridPartition::xyz_from_cell(int cell, float& x, float& y, float& z)
{
  int col = cell%ncols;
  int row = std::floor(cell/ncols);

  x = xmin + ((col+0.5) * xres);
  y = ymax - ((row+0.5) * yres);
  z = (zmax+zmin)/2;
}
