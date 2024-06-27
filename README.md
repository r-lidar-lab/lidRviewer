![Github](https://img.shields.io/badge/Github-0.1.0-green.svg) ![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)

This is a point cloud viewer for R. The first goal of this package is to be an alternative backend to display point clouds in the [lidR](https://github.com/Jean-Romain/lidR) package in replacement of `rgl`. It is fully supported but one may consider that this package is under development (it works good enough to me but still have bugs).

`rgl` is an awesome package but has some difficulties displaying large point clouds. The `lidRviewer` package is able to display large point clouds consisting of several million points. So far I have tried it with over 30 million points and the display window remained fluid (smoothly pan, zoom and rotate).

Advantage of `lidRviewer`:

* Can easily handle more than 20 million points while `rgl` can struggle displaying a tenth of that.
* Is much more memory efficient. It allocates only a small amount of additional memory while `rgl` may require gigabytes of memory.

Drawbacks

* Blocks the R session. If the viewer windows is open, you can't use R for anything else. You must close the viewer windows first.
* Can only display point clouds. No other feature. This is only a point cloud displayer so not intended as a replacement for `rgl`

## Installation

### GNU/Linux

```
sudo apt-get install libsdl2-dev freeglut3-dev
```

```r
devtools::install_github("Jean-Romain/lidRviewer")
```

### Windows

```r
devtools::install_github("Jean-Romain/lidRviewer")
```

### MacOS

```
brew install sdl2 mesa mesa-glu
```

```r
devtools::install_github("Jean-Romain/lidRviewer")
```

## Usage

```r
library(lidR)
LASfile <- system.file("extdata", "Megaplot.laz", package="lidR")
las <- readLAS(LASfile)
plot(las, backend = "lidRviewer")
```

- Rotate with left mouse button
- Zoom with mouse wheel
- Pan with right mouse button
