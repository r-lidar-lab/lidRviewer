![Github](https://img.shields.io/badge/Github-0.1.0-green.svg) ![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)

This is a point cloud viewer for R. The first goal of this package is to be an alternative backend to display point clouds in the [lidR](https://github.com/Jean-Romain/lidR) package in replacement of `rgl`. It is fully supported since `lidR v1.4.0` but one may consider that this package is under development (it works good enough to me but still have bugs).

`rgl` is an awesome package but has some difficulties displaying large point clouds. The 'PointCloudViewer' package is able to display large point clouds consisting of several million points. So far I have tried it with over 30 million points and the display window remained fluid (smoothly pan, zoom and rotate).

Advantage of `PointCloudViewer`:

* can easily handle more than 20 million points while `rgl` can struggle displaying a tenth of that.
* has pan zoom rotate while `rgl` only has zoom rotate.
* is much more memory efficient. It allocates only a small amount of additional memory.

Drawbacks

* blocks the R session. If the viewer windows is open, you can't use R for anything else. You must close the viewer windows first (so no multiple plots).
* can only display point clouds. No other feature. This is only a large point cloud displayer so not intended as a replacement for `rgl`

## Installation

`PointCloudViewer` is based on OpenGL and the SDL. You must install both first.

### GNU/Linux

```
sudo apt-get install libsdl-dev freeglut3-dev
```

```r
devtools::install_github("Jean-Romain/PointCloudViewer")
```

### Windows

*Section updated on December 27 2018*

We successfully installed the package on Windows machines. As always things are much harder on Windows :wink:. I wrote an automatic installation script that install the SDL and build the package.

1. Make sure you have a C++ compiler. On Windows install [Rtools.exe](https://cran.r-project.org/bin/windows/Rtools/). Install the `devtools` package.

```r
install.packages("devtools")
```

2. Run the following script by openning R (or Rstudio) **as administrator**. This script download the SDL library and install it along with R (R belongs in a folder where only an adminitrator can write). Once it is done it installs the `PointCloudViewer` package with `devtools`.

```r
source("https://raw.githubusercontent.com/Jean-Romain/PointCloudViewer/master/sdl.R")
install.sdl()
devtools::install_github("Jean-Romain/PointCloudViewer")
```

## Usage

```r
library(lidR)
LASfile <- system.file("extdata", "Megaplot.laz", package="lidR")
las <- readLAS(LASfile)
plot(las, backend = "pcv")
```

- Rotate with left mouse button
- Zoom with mouse wheel
- Pan with right mouse button

## How it works
 
Displaying huge point clouds in real time is a complex task. Advanced techniques often rely on Octrees to selectively display points within the camera field of view. These techniques are smart but not really appropriate for my low memory usage requirements. Considering how R stores data, constructing an Octree would require a full copy of the point cloud. This package uses some very naive optimization to render large point clouds in pseudo-real time:
 
* When users rotate/pan/zoom it only displays a maximum of 2 million points, which allows for smooth, fluid movement of the display.
* When the program finds a few milliseconds of free time between two events it progressively densifies the point cloud until the entire point cloud is displayed. This allows the user to rotate/pan/zoom again before the end of the rendering.
* It uses a pointer to the R memory and does not create any copy. The memory allocated for the rendering is therefore reduced to a few megabytes independently of the point cloud size.
 
So far it works well on my tests and the package can display as many points as the user wants (I stopped my tests after 35 million points [~1GB]).

## Benchmark

On a core i7 with 11 million points.

| Package            | Time (s)      | Alloc mem | Fluid |
| ------------------:|:-------------:| :--------:|:-----:|
| `rgl`              | 15-20         | 1.2 GB    | no    |
| `PointCloudViewer` | < 1           | 1.0 MB    | yes   |

'Time' is the time taken to open the Windows. 'Alloc mem' is the extra memory allocated by the call. 'Fluid' means you can rotate and zoom freely.
