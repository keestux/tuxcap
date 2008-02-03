INCLUDE_DIRECTORIES(${TUXCAP_SOURCE_DIR}/include ${TUXCAP_SOURCE_DIR}/chipmunk ${TUXCAP_SOURCE_DIR}/hgeparticle )

ADD_LIBRARY(pycap STATIC 
PycapApp.cpp
PycapBoard.cpp
PycapResources.cpp
)
