#ifndef CAMERA_H
#define CAMERA_H

#include "Frustum.h"
#include <SDL2/SDL.h>

class Camera
{
  public:
    Camera();
    ~Camera();

    void OnMouseMotion(const SDL_MouseMotionEvent & event);
    void OnMouseEvent(const SDL_MouseButtonEvent & event, const SDL_MouseWheelEvent & event_wheel);
    void OnKeyboard(const SDL_KeyboardEvent & event);

    void look();
    void setRotateSensivity(double sensivity);
    void setPanSensivity(double sensivity);
    void setZoomSensivity(double sensivity);
    void setDeltaXYZ(double dx, double dy, double dz);
    void setDistance(double);

    bool see(float x, float y, float z);

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
    bool holdleft;
    bool holdright;
    SDL_Cursor * _hand1;
    SDL_Cursor * _hand2;
    SDL_Cursor * _move;

private:
    CFrustum frustum;
};

#endif //CAMERA_H
