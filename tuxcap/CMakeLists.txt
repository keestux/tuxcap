PROJECT(TUXCAP)

#include some macros from another file...
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeMacros/IJMacros.txt)

#SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

INCLUDE(FindImageMagick)
INCLUDE(FindSDL)
INCLUDE(FindOpenGL)
#INCLUDE(FindSDL_mixer)

Find_Package ( SDL REQUIRED )
#Find_Package ( SDL_mixer REQUIRED )
Find_Package ( ImageMagick REQUIRED )
Find_Package ( OpenGL REQUIRED )

link_libraries (
   ${SDL_LIBRARY}
 #  ${SDLMIXER_LIBRARY}
   SDLmain
   audiere
   chipmunk
)

#TODO show message if required libs not found

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )

FIND_PACKAGE(AudiereLib)
IF(AUDIERELIB_FOUND)
    MESSAGE("DBG lib Audiere found. ${AUDIERELIB_INCLUDE_DIR} ${AUDIERELIB_LINK_DIRECTORIES}")
    LINK_DIRECTORIES(${AUDIERELIB_LINK_DIRECTORIES})
ELSE(AUDIERELIB_FOUND)
    MESSAGE("DBG lib Audiere requested but not found.")
ENDIF(AUDIERELIB_FOUND)

FIND_PACKAGE(ChipmunkLib)
IF(CHIPMUNKLIB_FOUND)
    MESSAGE("DBG lib chipmunk found. ${CHIPMUNKLIB_INCLUDE_DIR} ${CHIPMUNKLIB_LINK_DIRECTORIES}")
    LINK_DIRECTORIES(${CHIPMUNKLIB_LINK_DIRECTORIES})
ELSE(CHIPMUNKLIB_FOUND)
    MESSAGE("DBG lib chipmunk requested but not found. The physicsdemo's won't be build")
ENDIF(CHIPMUNKLIB_FOUND)

IF(CHIPMUNKLIB_FOUND)
SUBDIRS(demo1 demo2 demo3 demo4 demo5 physicsdemo physicsdemo2 physicsdemo3 physicsdemo4
physicsdemo5 physicsdemo6 physicsdemo7 hungarr lib)
ELSE(CHIPMUNKLIB_FOUND)
SUBDIRS(demo1 demo2 demo3 demo4 demo5 hungarr lib)
ENDIF(CHIPMUNKLIB_FOUND)

SET (CMAKE_BUILD_TYPE Debug)


