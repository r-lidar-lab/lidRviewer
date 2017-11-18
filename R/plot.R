#' Display big 3D point clouds
#'
#' Display big 3D point clouds using rgl style. This function does no lag and can diplay
#' almost instantly millions of points. It is optimized to used the lowest possible memory
#' and is capable of diplaying gigabytes of points allocating only few megabyte of memory
#'
#' @param x numeric vector of x coordinates
#' @param y numeric vector of y coordinates
#' @param z numeric vector of z coordinates
#' @param r integer vector of red components
#' @param g integer vector of green components
#' @param b integer vector of blue components
#' @param col character vector or hexadecimal colors.
#' @param id integer vector. Memory optimization. Instead of storing 3 vectors of integer
#' and potentially storing duplicated entries it is possible to provide a short list of colors
#' and refer to these color using a single set of integer used as id to the color.
#' @param size the size of the points
#' @aliases plot
#' @rdname plot
#' @useDynLib PointCloudViewer, .registration = TRUE
#' @importFrom Rcpp evalCpp
#' @export
#' @examples
#' \dontrun{
#' x = runif(1000, 0, 100)
#' y = runif(1000, 0, 100)
#' z = runif(1000, 0, 100)
#' col = rainbow(10)
#' id = sample(1:10, 10000, replace = TRUE)
#' plot_xyzcol(x, y, z, col, id)
#' }
plot_xyzcol <- function(x, y, z, col, id = NULL, size = 2)
{
  col = col2rgb(col)
  plot_xyzrgb(x, y, z, col[1,], col[2,], col[3,], id, size)
}

#' @aliases plot
#' @rdname plot
#' @export
plot_xyzrgb <- function(x, y, z, r, g, b, id = NULL, size = 2)
{
  xtxt = deparse(substitute(x))
  ytxt = deparse(substitute(y))
  ztxt = deparse(substitute(z))
  rtxt = deparse(substitute(r))
  gtxt = deparse(substitute(g))
  btxt = deparse(substitute(b))

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
  }
  else
  {
    id = 0
  }

  plotxyz(x,y,z,r,g,b,id,size)
}
