![Github](https://img.shields.io/badge/Github-0.1.0-green.svg) ![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)

This is a point cloud viewer for R. The first goal of this package is to be the backend to display point clouds in the `lidR` package in replacement of `rgl`.

`rgl` is an awesome package but has some difficulties displaying large point clouds. The 'PointCloudViewer' package is able to display large point clouds consisting of several million points. So far I have tried it with over 20 million points and the display window remained fluid (smoothly pan, zoom and rotate).

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

```
devtools::install_github("Jean-Romain/PointCloudViewer")
```

### Windows

<<<<<<< HEAD
Start praying... Then install OpenGL (actually if `rgl` is installed it should be already good). Then install the [SDL library](install sdl library windows). Then try:
=======
Start praying. Then install OpenGL (actually if `rgl` is installed it should be already good). Then install the [SDL library](https://www.google.com/search?q=install+SDL+windows&oq=install+SDL+windows&gs_l=psy-ab.3..0i71k1l4.2152.2152.0.2599.1.1.0.0.0.0.0.0..0.0.dummy_maps_web_fallback...0...1.1.64.psy-ab..1.0.0....0.pv8VzgF7f-Y). Then try:
>>>>>>> 5a744f39eac1d2ea5f606d0986cbbb90174475d5

```
devtools::install_github("Jean-Romain/PointCloudViewer")
```

<<<<<<< HEAD
To be honest I'm not expecting this to work, but who knows? In theory it could work if the SDL is properly installed.
=======
But to be honnest I'm not expecting it to work. But who knows? In theory it could work if the SDL is properly installed.
>>>>>>> 5a744f39eac1d2ea5f606d0986cbbb90174475d5

## Benchmark

On a core i7 with 11 million points.

| Package            | Time (s)      | Alloc mem | Fluid |
| ------------------ |:-------------:| ---------:|:-----:|
| `rgl`              | 15-20         | 1.2 GB    | no    |
| `PointCloudViewer` | < 1           | 1.0 MB    | yes   |

<<<<<<< HEAD
'Time' is the time taken to open the Windows. 'Alloc mem' Is the extra memory allocated by the call. 'Fluid' means you can rotate and zoom freely.
=======
Time is the time to open the windows. Alloc mem. is the extra memory allocated by the call. Fluid means you can rotate and zoom freely.

>>>>>>> 5a744f39eac1d2ea5f606d0986cbbb90174475d5
