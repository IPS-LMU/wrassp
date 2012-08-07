
getDObj <- function(fname) {
    fname <- path.expand(fname)
    .Call("getDObj", fname, PACKAGE="wrassp")
}


