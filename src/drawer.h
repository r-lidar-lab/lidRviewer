#ifndef DRAWER_H
#define DRAWER_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <Rcpp.h>
#include "Octree.h"
#include "camera.h"
//#include "index.h"

using namespace Rcpp;

enum Attribute{Z, I, RGB, CLASS};

struct RenderedPoint
{
  RenderedPoint() { memset(this, 0, sizeof(RenderedPoint)); };
  void set_color(unsigned char r, unsigned char g, unsigned char b) { this->r = r; this->g = g; this->b = b; };
  int pindex;
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

class Drawer
{
public:
  Drawer(DataFrame);
  bool draw();
  void setPointSize(float);
  void setAttribute(Attribute x);
  void display_hide_spataial_index() { draw_index = !draw_index; };
  Camera camera;
  EPToctree index;

  float point_size;
  bool lightning;

private:
  void eyes_dome_lightning();
  bool is_visible(const EPToctant& octant);
  void compute_cell_visibility();
  void query_rendered_point();
  void traverse_and_collect(const EPTkey& key, std::vector<EPToctant*>& visible_octants);

  bool draw_index;
  int npoints;
  int point_budget;

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
  std::vector<EPToctant*> visible_octants;
};

#endif //DRAWER_H
