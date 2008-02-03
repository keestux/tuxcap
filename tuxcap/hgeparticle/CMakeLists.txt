INCLUDE_DIRECTORIES(${TUXCAP_SOURCE_DIR}/include ${TUXCAP_SOURCE_DIR}/chipmunk ${TUXCAP_SOURCE_DIR}/hgeparticle )
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

ADD_LIBRARY(hgeparticle STATIC 
hgeparticle.cpp
hgepmanager.cpp
hgeRandom.cpp
hgerect.cpp
hgevector.cpp
)
