#ifndef DRAWER_H
#define DRAWER_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <Rcpp.h>
#include "camera.h"

using namespace Rcpp;

class Drawer
{
  public:
    Drawer(NumericVector, NumericVector, NumericVector, IntegerMatrix);
    ~Drawer();
    Camera * camera;
     void draw();
     void setPointSize(float);

  private:
    int npoints;
    int pass;
    int maxpass;
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
    IntegerMatrix rgb;
};

#endif //DRAWER_H
