PROJECT(libtuxcap)

# Fink changed the default to "Last". Basterds.
# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)

cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

# To build with debug info
#  (cd build/&& cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG .. && make)
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RELEASE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SUBDIRS(src resources)
