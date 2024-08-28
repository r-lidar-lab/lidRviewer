#ifndef DRAWER_H
#define DRAWER_H

#include <Rcpp.h>

#include "Octree.h"
#include "camera.h"

using namespace Rcpp;

enum Attribute{Z, I, RGB, CLASS};

class Drawer
{
public:
  Drawer(SDL_Window*, DataFrame, std::string hnof);
  bool draw();
  void resize();
  void setPointSize(float);
  void setAttribute(Attribute x);
  void display_hide_spatial_index() { draw_index = !draw_index; camera.changed = true; };
  void display_hide_edl() { lightning = !lightning; camera.changed = true; };
  void point_size_plus() { point_size++; camera.changed = true; };
  void point_size_minus() { point_size--; camera.changed = true; };
  void budget_plus() { point_budget += 500000; camera.changed = true; };
  void budget_minus() { if (point_budget > 500000) point_budget -= 500000; camera.changed = true; };
  Camera camera;
  Octree index;

  float point_size;
  bool lightning;

private:
  void edl();
  bool is_visible(const Node& octant);
  void compute_cell_visibility();
  void query_rendered_point();
  void traverse_and_collect(const Key& key, std::vector<Node*>& visible_octants);

  bool draw_index;
  uint32_t npoints;
  int point_budget;
  int rgb_norm;

  double minx;
  double miny;
  double minz;
  double maxx;
  double maxy;
  double maxz;
  double xcenter;
  double ycenter;
  double zcenter;
  double xrange;
  double yrange;
  double zrange;
  double range;
  double zqmin;
  double zqmax;
  double minattr;
  double maxattr;
  double attrrange;

  DataFrame df;
  NumericVector x;
  NumericVector y;
  NumericVector z;
  IntegerVector r;
  IntegerVector g;
  IntegerVector b;

  IntegerVector attri;
  NumericVector attrd;

  Attribute attr;
  std::vector<int> pp;
  std::vector<Node*> visible_octants;

  SDL_Window *window;
  float zNear;
  float zFar;
  float fov;
  int width;
  int height;
};

#endif //DRAWER_H
