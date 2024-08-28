#' Display big 3D point clouds
#'
#' Display arbitrary large in memory 3D point clouds from the lidR package. Keyboard can be use
#' to control the rendering
#' - Rotate with left mouse button
#' - Zoom with mouse wheel
#' - Pan with right mouse button
#' - Keyboard <kbd>r</kbd> or <kbd>g</kbd> or <kbd>b</kbd> to color with RGB
#' - Keyboard <kbd>z</kbd> to color with Z
#' - Keyboard <kbd>i</kbd> to color with Intensity
#' - Keyboard <kbd>c</kbd> to color with Classification
#' - Keyboard <kbd>+</kbd> or <kbd>-</kbd> to change the point size
#' - Keyboard <kbd>l</kbd> to enable/disable eyes-dome lightning
#'
#' @param x a point cloud with minimally 3 columns named X,Y,Z
#' @param y Unused (inherited from R base)
#' @param ... unused
#' @export
#' @method plot LAS
#' @importClassesFrom lidR LAS
#' @useDynLib lidRviewer, .registration = TRUE
#' @importFrom Rcpp evalCpp
#' @md
setMethod("plot", signature(x = "LAS", y = "missing"), function(x, y, ...)
{
  viewer(x@data, "")
})

render = function(f)
{
  x = normalizePath(x)
  las = lidR::readLAS(x)
  hnof = paste0(substr(x, 1, nchar(x) - 3), "hno")
  f = if (file.exists(hnof)) hnof else x
  print(f)
  viewer(las@data, f)
}


#' Deprecated backward compatible function
#'
#' @param x,y,z numeric vector
#' @param r,g,b integer vector
#' @param id index
#' @param size not used
#' @export
plot_xyzrgb <- function(x, y, z, r, g, b, id = NULL, size = 4)
{
  xtxt = deparse(substitute(x))
  ytxt = deparse(substitute(y))
  ztxt = deparse(substitute(z))
  rtxt = deparse(substitute(r))
  gtxt = deparse(substitute(g))
  btxt = deparse(substitute(b))
  itxt = deparse(substitute(id))

  if (!is.vector(x)) {stop(paste(xtxt, "is not a vector"))}
  if (!is.vector(y)) {stop(paste(ytxt, "is not a vector"))}
  if (!is.vector(z)) {stop(paste(ztxt, "is not a vector"))}
  if (!is.vector(r)) {stop(paste(rtxt, "is not a vector"))}
  if (!is.vector(g)) {stop(paste(gtxt, "is not a vector"))}
  if (!is.vector(b)) {stop(paste(btxt, "is not a vector"))}

  if (!is.integer(r)) {stop(paste(rtxt, "must contain integers"))}
  if (!is.integer(g)) {stop(paste(gtxt, "must contain integers"))}
  if (!is.integer(b)) {stop(paste(btxt, "must contain integers"))}

  if (length(x) != length(y)) {stop(paste(xtxt, "is not same length as", ytxt))}
  if (length(x) != length(z)) {stop(paste(xtxt, "is not same length as", ztxt))}
  if (length(r) != length(g)) {stop(paste(rtxt, "is not same length as", gtxt))}
  if (length(r) != length(b)) {stop(paste(rtxt, "is not same length as", btxt))}

  if (length(x) != length(r) & is.null(id)) {stop(paste(xtxt, "is not same length as", rtxt))}
  if (length(x) != length(g) & is.null(id)) {stop(paste(xtxt, "is not same length as", gtxt))}
  if (length(x) != length(b) & is.null(id)) {stop(paste(xtxt, "is not same length as", btxt))}

  if (!is.null(id))
  {
    if (!is.integer(id))
      stop("'id' must contain integers")

    if (max(id) > length(x))
      stop("Index out of bound. 'id' contains wrong ids.")

    if (min(id) <= 0)
      stop("Index out of bound. 'id' contains negative or 0 values.")

    if (length(x) != length(id))
        stop(paste(xtxt, "is not same length as", itxt))
  }
  else
  {
    id = 0
  }

  message("Point cloud viewer must be closed before to run other R code")

  df = data.frame(X = x, Y = y, Z = z, R = r, G = g, B = b)
  viewer(df)
}
