
getDObj <- function(fname, begin=0, end=0, samples=FALSE) {
    fname <- path.expand(fname)
    .External("getDObj2", fname, begin=begin, end=end, samples=samples, PACKAGE="wrassp")
}


