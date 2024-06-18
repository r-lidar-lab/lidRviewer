# Process this file with autoconf to produce a configure script.

AC_INIT([lidRviewer], [m4_esyscmd([sed -n 's/^Version: //p' DESCRIPTION | tr -d '\n'])])

# Pick R specific compiler and flags
CXX=`"${R_HOME}/bin/R" CMD config CXX`
CPPFLAGS=`"${R_HOME}/bin/R" CMD config CPPFLAGS`
CXXFLAGS=`"${R_HOME}/bin/R" CMD config CXXFLAGS`

# Checks for libraries.

have_sdl2=no
PKG_CHECK_MODULES([SDL2], [sdl2 >= 2.0.0], [have_sdl2=yes], [have_sdl2=no])
AS_IF([test "x$have_sdl2" = "xno"], [
  AC_PATH_PROG([SDL2_CONFIG], [sdl2-config], [no])

  AS_IF([test "$SDL2_CONFIG" = "no"], [
  ], [
    SDL2_CFLAGS=$($SDL2_CONFIG --cflags)
    SDL2_LIBS=$($SDL2_CONFIG --libs)  
  ])
])

AC_SUBST([SDL2_CFLAGS])
AC_SUBST([SDL2_LIBS])  

AC_MSG_NOTICE([sdl was successfully found!])
AC_MSG_NOTICE([SDL2_CFLAGS: $SDL2_CFLAGS])
AC_MSG_NOTICE([SDL2_LIBS: $SDL2_LIBS])

have_glu=no
PKG_CHECK_MODULES([GLU], [glu >= 1.3.1], [have_glu=yes], [have_glu=no])

have_gl=no
PKG_CHECK_MODULES([GL], [gl >= 1.2], [have_gl=yes], [have_gl=no])

# Set CFLAGS and LIBS
CFLAGS="$CFLAGS $SDL2_CFLAGS $GLU_CFLAGS $GL_CFLAGS"
LIBS="$LIBS $SDL2_LIBS $GLU_LIBS $GL_LIBS"

# Checks for header files.
AC_CHECK_HEADERS([SDL2/SDL.h GL/glu.h GL/gl.h])

# Checks for library functions.
AC_CHECK_LIB([SDL2], [SDL_Init], [have_sdl2=yes], [have_sdl2=no])
AC_CHECK_LIB([GLU], [gluLookAt], [have_glu=yes], [have_glu=no])
AC_CHECK_LIB([GL], [glLoadIdentity], [have_gl=yes], [have_gl=no])

AS_IF([test "x$have_sdl2" = "xno"], [
  AC_MSG_NOTICE([error: 
    SDL2 library not found!
  
    Please install SDL2 development package:
      - Debian: apt-get install libsdl2-dev
      - Dnf based: dnf install SDL2-devel
      - Arch: pacman -S sdl2
      - MacOS: brew install sdl2
  ])
])

AS_IF([test "x$have_glu" = "xno"], [
  AC_MSG_NOTICE([error: 
    GLU library not found!
  
    Please install GLU development package:
      - Debian: apt-get install libglu1-mesa-dev
      - Dnf based: dnf install mesa-libGLU-devel
      - Arch: pacman -S glu
      - MacOS: brew install mesa-glu
  ])
])

AS_IF([test "x$have_gl" = "xno"], [
  AC_MSG_NOTICE([error: 
    GL library not found!
  
    Please install GL development package:
      - Debian: apt-get install libgl1-mesa-dev
      - Dnf based: dnf install mesa-libGL-devel
      - Arch: pacman -S mesa
      - MacOS: brew install mesa
  ])
])

AS_IF([test "x$have_sdl2" = "xno" -o "x$have_glu" = "xno" -o "x$have_gl" = "xno"], [
  AC_MSG_ERROR([Some libraries were not found!])
])


AC_SUBST([CFLAGS])
AC_SUBST([LIBS])
AC_CONFIG_FILES([src/Makevars])
AC_OUTPUT

