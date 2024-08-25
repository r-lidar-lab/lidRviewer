#include "drawer.h"

const float zNear = 1;
const float zFar = 100000;
const Uint32 time_per_frame = 1000 / 30;

// [[Rcpp::export]]
void viewer(DataFrame df)
{
  bool run = true;

  SDL_Event event;

  unsigned int width = 600;
  unsigned int height = 600;

  Uint32 last_time, current_time, elapsed_time;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Window *window = SDL_CreateWindow("lidRviewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1); // Enable VSync

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(70, (double)width / height, zNear, zFar);

  glEnable(GL_DEPTH_TEST);              // Enable depth testing for z-culling
  glDepthFunc(GL_LEQUAL);               // Set the type of depth-test
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective corrections


  glEnable(GL_POINT_SMOOTH);   // Round point

  glEnable(GL_LINE_SMOOTH);   // Enable line anti-aliasing
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  //glEnable(GL_MULTISAMPLE);
  //glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

  //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); // Enable changing the point size

  Drawer *drawer = new Drawer(window, df);
  drawer->camera.setRotateSensivity(0.1);
  drawer->camera.setZoomSensivity(10);
  drawer->camera.setPanSensivity(1);
  drawer->setPointSize(4);


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
        case SDLK_z:
          drawer->setAttribute(Attribute::Z);
          break;
        case SDLK_i:
          drawer->setAttribute(Attribute::I);
          break;
        case SDLK_c:
          drawer->setAttribute(Attribute::CLASS);
          break;
        case SDLK_r:
        case SDLK_g:
        case SDLK_b:
          drawer->setAttribute(Attribute::RGB);
          break;
        case SDLK_q:
          drawer->display_hide_spatial_index();
          break;
        case SDLK_l:
          drawer->display_hide_edl();
          break;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
          drawer->point_size_plus();
          break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
          drawer->point_size_minus();
          break;
        default:
          drawer->camera.OnKeyboard(event.key);
        break;
        }
        break;

      case SDL_MOUSEMOTION:
        drawer->camera.OnMouseMotion(event.motion);
        break;

      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
        drawer->camera.OnMouseEvent(event.button, SDL_MouseWheelEvent{}); // Pass an empty SDL_MouseWheelEvent
        break;

      case SDL_MOUSEWHEEL:
        drawer->camera.OnMouseEvent(SDL_MouseButtonEvent{}, event.wheel); // Pass an empty SDL_MouseButtonEvent
        break;

      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          width = event.window.data1;
          height = event.window.data2;
          glViewport(0, 0, width, height);
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(70, (double)width / height, zNear, zFar);
          drawer->camera.changed = true;
        }
        break;
      }
    }

    current_time = SDL_GetTicks();
    elapsed_time = current_time - last_time;

    if (elapsed_time > time_per_frame)
    {
      drawer->draw();
      last_time = current_time;
    }
    else
    {
      SDL_Delay(time_per_frame - elapsed_time);
    }
  }

  delete drawer;
  SDL_GL_DeleteContext(glContext); // Correctly delete OpenGL context
  SDL_DestroyWindow(window); // Correctly destroy window
  SDL_Quit();
  return;
}
