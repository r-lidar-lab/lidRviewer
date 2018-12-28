.R.home.bin <- function(arch) {
  bin <- R.home('bin')
  if (arch == "i386") bin <- normalizePath(paste0(bin, "/../i386/"))
  return(bin)
}

.install.dll <- function(arch = c("x64", "i386")) {
  arch <- match.arg(arch)
  temp <- tempdir()
  bin  <- .R.home.bin(arch)
  ddl  <- paste0(temp, '/sdldll.zip')
  dll  <- paste0(temp, '/SDL.dll')

  if (!dir.exists(bin)) {
    cat(paste0("  SDL dll installation skipped for arch", arch, "\n"))
    return(TRUE)
  }

  url <- if (arch == "x64") {
    'https://www.libsdl.org/release/SDL-1.2.15-win32-x64.zip'
  } else {
    'https://www.libsdl.org/release/SDL-1.2.15-win32.zip'
  }

  download.file(url, ddl, quiet = TRUE)

  cat(" - SDL dll downloaded\n")

  utils::unzip(ddl, exdir = temp)

  cat(" - SDL dll uncompressed\n")

  status <- file.copy(dll, bin, recursive = TRUE)

  if (!all(status)) {
    cat(" - Error: SDL dll was not copied correctly.\n")
    return(FALSE)
  } else {
    cat(" - SDL dll installed\n")
    return(TRUE)
  }
}

.install.headers <- function() {
  temp <- tempdir()
  ddl  <- paste0(temp, '/sdldevel.tar.gz')
  inc  <- paste0(R.home('include'), '/SDL')
  url  <- 'https://www.libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz'

  if (!dir.exists(inc)) dir.create(inc)

  download.file(url, ddl, quiet = TRUE)

  cat(" - SDL headers downloaded\n")

  utils::untar(ddl, exdir = temp)

  cat(" - SDL headers uncompressed\n")

  includes <- paste0(temp, '/SDL-1.2.15/include/')
  status   <- file.copy(includes, inc, recursive = TRUE)

  if (!all(status)) {
    cat(" - Error: headers where not copied correctly.\n")
    return(FALSE)
  } else {
    cat(" - SDL headers installed\n")
    return(TRUE)
  }
}

install.sdl <- function() {
  if (.Platform$OS.type != "windows")
    stop("This script is designed to install the SDL on Windows only.", call. = FALSE)

  cat("Downloading and installing SDL headers...\n")
  s1 <- .install.headers()
  cat("Downloading and installing SDL dll (x64)...\n")
  s2 <- .install.dll("x64")
  cat("Downloading and installing SDL dll (i386)...\n")
  s3 <- .install.dll("i386")

  if (s1 + s2 + s3 < 3)
    stop("Something when wrong during the installation.", call. = FALSE)
  else
    cat("Installation of the SDL library: done")
}

