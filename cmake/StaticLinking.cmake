# http://stackoverflow.com/questions/15521958/cmake-static-and-dynamic-linking-based-on-build-type
# cmake looks for libraries ending with .a. This is for linux only!
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")


# set -static, and set LINK_SEARCH_END_STATIC to remove the additional -bdynamic from the linker line.

SET(CMAKE_EXE_LINKER_FLAGS "-static")
SET_TARGET_PROPERTIES(surface PROPERTIES LINK_SEARCH_END_STATIC 1)
