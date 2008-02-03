INCLUDE_DIRECTORIES(${libtuxcap_SOURCE_DIR}/tuxcap/include ${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk ${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle )

ADD_LIBRARY(pycap STATIC 
PycapApp.cpp
PycapBoard.cpp
PycapResources.cpp
)
