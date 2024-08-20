#ifndef DRAWER_H
#define DRAWER_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <Rcpp.h>
#include "camera.h"
#include "index.h"

using namespace Rcpp;

enum Attribute{Z, Distance, Ratio, Angle};

class Drawer
{
  public:
    Drawer(NumericVector, NumericVector, NumericVector, IntegerVector, IntegerVector, IntegerVector, IntegerVector);
    ~Drawer();
    Camera * camera;
     bool draw();
     void setPointSize(float);
     void setAttribute(Attribute x) { attr = x; };

  private:
    int npoints;
    float size;
    double minx;
    double miny;
    double minz;
    double maxx;
    double maxy;
    double maxz;
    double xcenter;
    double ycenter;
    double zcenter;
    NumericVector x;
    NumericVector y;
    NumericVector z;
    IntegerVector r;
    IntegerVector g;
    IntegerVector b;
    IntegerVector id;
    GridPartition index;
    Attribute attr;
};

#endif //DRAWER_H
