#include <Rcpp.h>

#include <SDL2/SDL.h>

#include <thread>
#include <atomic>

#include "drawer.h"
#include "sdlglutils.h"

const char *hand1[] =
{
  /* width height num_colors chars_per_pixel */
  " 16 16 3 1 ",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "       XX       ",
  "   XX X..XXX    ",
  "  X..XX..X..X   ",
  "  X..XX..X..X X ",
  "   X..X..X..XX.X",
  "   X..X..X..X..X",
  " XX X.......X..X",
  "X..XX..........X",
  "X...X.........X ",
  " X............X ",
  "  X...........X ",
  "  X..........X  ",
  "   X.........X  ",
  "    X.......X   ",
  "     X......X   ",
  "     X......X   ",
  "0,0"
};

const char *hand2[] =
  {
  /* width height num_colors chars_per_pixel */
  " 16 16 3 1 ",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "                ",
  "                ",
  "                ",
  "                ",
  "    XX XX XX    ",
  "   X..X..X..XX  ",
  "   X........X.X ",
  "    X.........X ",
  "   XX.........X ",
  "  X...........X ",
  "  X...........X ",
  "  X..........X  ",
  "   X.........X  ",
  "    X.......X   ",
  "     X......X   ",
  "     X......X   ",
  "0,0"
  };

const char *move[] =
  {
  /* width height num_colors chars_per_pixel */
  " 16 16 1 ",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "       XX       ",
  "      X..X      ",
  "     X....X     ",
  "    X......X    ",
  "   X XX..XX X   ",
  "  X.X X..X X.X  ",
  " X..XXX..XXX..X ",
  "X..............X",
  "X..............X",
  " X..XXX..XXX..X ",
  "  X.X X..X X.X  ",
  "   X XX..XX X   ",
  "    X......X    ",
  "     X....X     ",
  "      X..X      ",
  "       XX       ",
  "0,0"
  };



const Uint32 time_per_frame = 1000 / 30;
bool running = false;
std::thread sdl_thread;

void sdl_loop(DataFrame df, std::string hnof)
{
  SDL_Event event;

  unsigned int width = 600;
  unsigned int height = 600;
  bool run = true;

  Uint32 last_time, current_time, elapsed_time;

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    Rcpp::stop("Unable to initialize SDL: %s", SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Window *window = SDL_CreateWindow("lidRviewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  if (window == nullptr)
  {
    SDL_Quit();
    Rcpp::stop("Unable to create SDL window: %s", SDL_GetError());
  }

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1); // Enable VSync

  SDL_Cursor* _hand1 = cursorFromXPM(hand1);
  SDL_Cursor* _hand2 = cursorFromXPM(hand2);
  SDL_Cursor* _move  = cursorFromXPM(move);
  SDL_SetCursor(_hand1);

  Drawer *drawer = new Drawer(window, df, hnof);
  drawer->camera.setRotateSensivity(0.1);
  drawer->camera.setZoomSensivity(10);
  drawer->camera.setPanSensivity(1);
  drawer->setPointSize(4);

  last_time = SDL_GetTicks();

  bool ctrlPressed = false;
  bool rotate = false;
  bool pan = false;

  while (run)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
        {
          run = false;
          break;
        }

        case SDL_KEYDOWN:
        {
          switch (event.key.keysym.sym)
          {
            case SDLK_LCTRL:
            case SDLK_RCTRL:
              ctrlPressed = true;
              break;
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
            break;
          }
          break;
        }

        case SDL_KEYUP:
        {
          switch (event.key.keysym.sym)
          {
            case SDLK_LCTRL:
            case SDLK_RCTRL:
              ctrlPressed = false;
              break;
          }
          break;
        }

        case SDL_MOUSEBUTTONUP:
        {
          switch(event.button.button)
          {
            case SDL_BUTTON_LEFT:
              rotate = false;
              SDL_SetCursor(_hand1);
              break;
            case SDL_BUTTON_RIGHT:
              pan = false;
              SDL_SetCursor(_hand1);
              break;
          }
          break;
        }

        case SDL_MOUSEBUTTONDOWN:
        {
          switch(event.button.button)
          {
            case SDL_BUTTON_LEFT:
              rotate = true;
              SDL_SetCursor(_hand2);
              break;
            case SDL_BUTTON_RIGHT:
              pan = true;
              SDL_SetCursor(_move);
              break;
          }
          break;
        }

        case SDL_MOUSEMOTION:
        {
          if (pan) drawer->camera.pan(event.motion.xrel, event.motion.yrel);
          if (rotate) drawer->camera.rotate(event.motion.xrel, event.motion.yrel);
          break;
        }

        case SDL_MOUSEWHEEL:
        {
          if (ctrlPressed && event.wheel.y > 0) drawer->budget_plus();
          else if (ctrlPressed && event.wheel.y < 0) drawer->budget_minus();
          else drawer->camera.zoom(event.wheel.y);
          break;
        }

        case SDL_WINDOWEVENT:
        {
          drawer->resize();
          break;
        }
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

  delete drawer;
  SDL_SetCursor(NULL);
  SDL_FreeCursor(_hand1);
  SDL_FreeCursor(_hand2);
  SDL_FreeCursor(_move);
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();
  running = false;
}

// [[Rcpp::export]]
void viewer(DataFrame df, bool detach, std::string hnof)
{
  if (detach)
  {
    if (running) Rcpp::stop("lidRviewer is limited to one rendering point cloud");
    sdl_thread = std::thread(sdl_loop, df, hnof);
    sdl_thread.detach();  // Detach the thread to allow it to run independently
    running = true;
  }
  else
  {
    sdl_loop(df, hnof);
  }
}
