PROJECT(libtuxcap)

cmake_minimum_required(VERSION 2.4)
    if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 OLD)
    endif(COMMAND cmake_policy)

SET(CMAKE_BUILD_TYPE DEBUG)

SUBDIRS(src resources)
