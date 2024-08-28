#ifndef CAMERA_H
#define CAMERA_H

#include "Frustum.h"

class Camera
{
  public:
    Camera();

    void rotate(int xrel, int yrel);
    void pan(int xrel, int yrel);
    void zoom(int zrel);
    void look();
    void setRotateSensivity(double sensivity);
    void setPanSensivity(double sensivity);
    void setZoomSensivity(double sensivity);
    void setDeltaXYZ(double dx, double dy, double dz);
    void setDistance(double);

    bool see(float x, float y, float z, float hsize);

    bool changed;
    double zoomSensivity;
    double rotateSensivity;
    double panSensivity;
    double distance;
    double x;
    double y;
    double z;
    double angleY;
    double angleZ;
    double deltaX;
    double deltaY;
    double deltaZ;

private:
    CFrustum frustum;
};

#endif //CAMERA_H
