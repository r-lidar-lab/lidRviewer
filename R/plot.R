#' Display big 3D point clouds
#'
#' Display aritrary large in memory 3D point clouds. This function does no lag and can display
#' almost instantly millions of points. It is optimized to used the lowest possible memory
#' and is capable of displaying gigabytes of points allocating only few megabyte of memory
#'
#' @param x A \code{LAS*} object from the lidR package
#' @param y Unused (inherited from R base)
#' @export
#' @method plot LAS
#' @importClassesFrom lidR LAS
#' @useDynLib lidRviewer, .registration = TRUE
#' @importFrom Rcpp evalCpp
setGeneric("plot", function(x, y, ...) standardGeneric("plot"))

#' @rdname plot
setMethod("plot", signature(x = "LAS", y = "missing"), function(x, y, ...)
{
  plot.LAS(x, y, ...)
})

plot.LAS <- function(x, y, ...)
{
  viewer(x@data)
}

#' @aliases plot
#' @rdname plot
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
