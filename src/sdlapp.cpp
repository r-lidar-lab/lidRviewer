#include "drawer.h"

#define FPS 30
#define WW 600
#define WH 600

// [[Rcpp::export]]
void plotxyz(NumericVector x, NumericVector y, NumericVector z, IntegerVector r, IntegerVector g, IntegerVector b, IntegerVector id, float size = 2)
{
  bool run = true;

  SDL_Event event;
  const Uint32 time_per_frame = 1000 / FPS;
  unsigned int width = WW;
  unsigned int height = WH;

  Uint32 last_time, current_time, elapsed_time;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Window *window = SDL_CreateWindow("Point Cloud Viewer",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        width, height,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1); // Enable VSync

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(70, (double)width / height, 0.1, 100000);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

  Drawer *drawer = new Drawer(x, y, z, r, g, b, id);
  drawer->camera->setRotateSensivity(0.3);
  drawer->camera->setZoomSensivity(10);
  drawer->camera->setPanSensivity(1);
  drawer->setPointSize(size);

  last_time = SDL_GetTicks();

  while (run)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
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
        case SDLK_d:
          drawer->setAttribute(Attribute::Distance);
          drawer->camera->changed = true;
          break;
        case SDLK_z:
          drawer->setAttribute(Attribute::Z);
          drawer->camera->changed = true;
          break;
        case SDLK_f:
          drawer->setAttribute(Attribute::Z);
          drawer->camera->changed = true;
          break;
        default:
          drawer->camera->OnKeyboard(event.key);
        break;
        }
        break;

      case SDL_MOUSEMOTION:
        //printf("Mouse motion\n");
        drawer->camera->OnMouseMotion(event.motion);
        break;

      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
        drawer->camera->OnMouseEvent(event.button, SDL_MouseWheelEvent{}); // Pass an empty SDL_MouseWheelEvent
        break;

      case SDL_MOUSEWHEEL:
        drawer->camera->OnMouseEvent(SDL_MouseButtonEvent{}, event.wheel); // Pass an empty SDL_MouseButtonEvent
        break;

      case SDL_WINDOWEVENT:
        //printf("SDL windows event\n");
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          width = event.window.data1;
          height = event.window.data2;
          glViewport(0, 0, width, height);
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(70, (double)width / height, 0.1, 10000);
          drawer->camera->changed = true;
        }
        break;
      }
    }

    current_time = SDL_GetTicks();
    elapsed_time = current_time - last_time;

    if (elapsed_time > time_per_frame)
    {
      //printf("Try draw\n");
      if (drawer->draw())
      {
        //printf("Update windows\n");
        SDL_GL_SwapWindow(window);
      }

      last_time = current_time;

    }
    else
    {
      //printf("Delay\n");
      SDL_Delay(time_per_frame - elapsed_time);
    }
  }

  delete drawer;
  SDL_GL_DeleteContext(glContext); // Correctly delete OpenGL context
  SDL_DestroyWindow(window); // Correctly destroy window
  SDL_Quit();
  return;
}
