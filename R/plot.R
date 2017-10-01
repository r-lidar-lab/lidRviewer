#' Display big 3D point clouds
#'
#' @param x vector of x coordinates
#' @param y vector of y coordinates
#' @param z vector of z coordinates
#' @param col matrix n x 3 of rgb values to color the points
#' @param size the size of the points
#'
#' @useDynLib PointCloudViewer, .registration = TRUE
#' @importFrom Rcpp evalCpp
#' @export
plot_xyz <- function(x, y, z, col, size = 2)
{
  xtxt = deparse(substitute(x))
  ytxt = deparse(substitute(y))
  ztxt = deparse(substitute(z))

  if (!is.vector(x)) {stop(paste(xtxt, "is not a vector"))}
  if (!is.vector(y)) {stop(paste(ytxt, "is not a vector"))}
  if (!is.vector(z)) {stop(paste(ztxt, "is not a vector"))}

  if (length(x) != length(y)) {
    stop(paste(xtxt, "is not same length as", ytxt))
  }

  if (length(x) != length(z)) {
    stop(paste(xtxt, "is not same length as", ztxt))
  }

  if (nrow(col) < lenght(x)) {
    stop("Wrong colo matrix size")
  }

  plotxyz(x,y,z, col, size)
}
