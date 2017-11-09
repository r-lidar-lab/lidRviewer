#' Display big 3D point clouds
#'
#' Display big 3D point cloud using rgl style but is memory efficent and does no lag
#' even for millions of points.
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
#' @examples
#' \dontrun{
#' x = runif(1000, 0, 100)
#' y = runif(1000, 0, 100)
#' z = runif(1000, 0, 100)
#' c = t(col2rgb(sample(rainbow(10), 1000,replace = T)))
#' plot_xyz(x,y,z, c)
#' }
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

  if (nrow(col) < length(x)) {
    stop("Wrong colo matrix size")
  }

  plotxyz(x,y,z, col, size)
}
