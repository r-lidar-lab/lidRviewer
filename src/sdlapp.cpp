#include "drawer.h"

#define FPS 30
#define WW 1000
#define WH 1000

//' @export
// [[Rcpp::export]]
void plotxyz(NumericVector x, NumericVector y, NumericVector z, IntegerMatrix col, float size = 2)
{
  bool run = true;

  SDL_Event event;
  const Uint32 time_per_frame = 1000/FPS;
  unsigned int width = WW;
  unsigned int height = WH;

  Uint32 last_time,current_time,elapsed_time;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_WM_SetCaption("Point Cloud Viewer", NULL);
  SDL_SetVideoMode(WW, WH, 32, SDL_OPENGL | SDL_RESIZABLE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(70,(double)width/height,0.001,10000);

  glEnable(GL_DEPTH_TEST);

  Drawer* drawer = new Drawer(x,y,z, col);
  drawer->camera->setRotateSensivity(0.3);
  drawer->camera->setZoomSensivity(10);
  drawer->camera->setPanSensivity(1);
  drawer->setPointSize(size);

  last_time = SDL_GetTicks()-2*time_per_frame;

  while (run)
  {
    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
      case SDL_QUIT:
        run = false;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          run = false;
          break;
        default :
          drawer->camera->OnKeyboard(event.key);
        break;
        }
        break;
      case SDL_MOUSEMOTION:
        drawer->camera->OnMouseMotion(event.motion);
        break;
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
        drawer->camera->OnMouseButton(event.button);
        break;
      case SDL_VIDEORESIZE:
        SDL_SetVideoMode(event.resize.w,event.resize.h, 32, SDL_OPENGL| SDL_RESIZABLE);
        break;
      }
    }

    current_time = SDL_GetTicks();
    elapsed_time = current_time - last_time;
    last_time = current_time;

    drawer->draw();

    if (elapsed_time < time_per_frame)
    {
      //Rcout << "sleep for " << time_per_frame - (time_per_frame - elapsed_time) << "ms" << std::endl;
      SDL_Delay(time_per_frame - (time_per_frame - elapsed_time));
    }
  }

  delete drawer;
  SDL_Quit();
  return;
}

