INCLUDE_DIRECTORIES(${libtuxcap_SOURCE_DIR}/tuxcap/include ${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk ${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle )
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

ADD_LIBRARY(hgeparticle STATIC 
hgeparticle.cpp
hgepmanager.cpp
hgeRandom.cpp
hgerect.cpp
hgevector.cpp
)
