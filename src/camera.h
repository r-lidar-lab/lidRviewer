#ifndef CAMERA_H
#define CAMERA_H

#include <SDL/SDL.h>

class Camera
{
  public:
    Camera();
    ~Camera();

    void OnMouseMotion(const SDL_MouseMotionEvent & event);
    void OnMouseButton(const SDL_MouseButtonEvent & event);
    void OnKeyboard(const SDL_KeyboardEvent & event);

    void look();
    void setRotateSensivity(double sensivity);
    void setPanSensivity(double sensivity);
    void setZoomSensivity(double sensivity);
    void setDeltaXYZ(double dx, double dy, double dz);
    void setDistance(double);
    bool changed;

  private:
    double zoomSensivity;
    double rotateSensivity;
    double panSensivity;
    double distance;
    double angleY;
    double angleZ;
    double deltaX;
    double deltaY;
    double deltaZ;
    bool holdleft;
    bool holdright;
    SDL_Cursor * _hand1;
    SDL_Cursor * _hand2;
};

#endif //CAMERA_H
