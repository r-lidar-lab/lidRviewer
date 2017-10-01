![Github](https://img.shields.io/badge/Github-0.1.0-green.svg) ![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)

This is a point cloud viewer for R. The first goal of this package is to be the backend to display point clouds in the `lidR` package in replacement of `rgl`.

`rgl` is an awesome package but have difficulties (to do not say it can't) displaying big point clouds. This package enable to display big points up to several millions of points. So far I tried up to 20 millions and the windows was fluid.

Advantage of `PointCloudViewer`:

* handle easily more than 20 million points while `rgl` get in trouble from a tenth of that.
* have pan zoom rotate while `rgl` only has zoom rotate.
* is much more memory efficient. It allocate only few extra memory.

Drawbacks

* blocks the R session. If the viewer windows is open, you can't make R stuff. You must close the viewer windows first (so no multiple plots).
* enables only to display a point cloud. No other feature. This is only a big point cloud displayer not a replacement of `rgl`

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

Start praying. Then install OpenGL (actually if `rgl` is installed it should be already good). Then install the [SDL library](https://www.google.com/search?q=install+SDL+windows&oq=install+SDL+windows&gs_l=psy-ab.3..0i71k1l4.2152.2152.0.2599.1.1.0.0.0.0.0.0..0.0.dummy_maps_web_fallback...0...1.1.64.psy-ab..1.0.0....0.pv8VzgF7f-Y). Then try:

```
devtools::install_github("Jean-Romain/PointCloudViewer")
```

But to be honnest I'm not expecting it to work. But who knows? In theory it could work if the SDL is properly installed.

## Benchmark

On a core i7 with 11 million points.

| Package            | Time (s)      | Alloc mem | Fluid |
| ------------------ |:-------------:| ---------:|:-----:|
| `rgl`              | 15-20         | 1.2 GB    | no    |
| `PointCloudViewer` | < 1           | 1.0 MB    | yes   |

Time is the time to open the windows. Alloc mem. is the extra memory allocated by the call. Fluid means you can rotate and zoom freely.

