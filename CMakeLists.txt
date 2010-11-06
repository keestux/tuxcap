PROJECT(libtuxcap)

# Fink changed the default to "Last". Basterds.
# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)

cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

SET(CMAKE_BUILD_TYPE RELEASE)

SUBDIRS(src resources)
