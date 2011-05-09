PROJECT(libtuxcap)

# Fink changed the default to "Last". Basterds.
# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)

cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

# To build with debug info
#  (cd build-debug/ && cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG .. && make)

# To build for older than Snow Leopard:
#  (cd build-debug/ && CC=gcc-4.2 CXX=g++-4.2 cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG .. && make)

# To build using SDL mixer
#  (cd build-sdl/ && CC=gcc-4.2 CXX=g++-4.2 cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG -DSOUND_SYSTEM:STRING=SDL .. && make)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RELEASE)
ENDIF(NOT CMAKE_BUILD_TYPE)

ADD_CUSTOM_TARGET(bin ALL
    COMMAND mkdir -p bin
)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(resources)
