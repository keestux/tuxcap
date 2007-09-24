PROJECT(TUXCAP)

#include some macros from another file...
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeMacros/IJMacros.txt)

#SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )

INCLUDE(FindImageMagick)
INCLUDE(FindSDL)
INCLUDE(FindOpenGL)
INCLUDE(FindSDL_mixer)

Find_Package ( SDL REQUIRED )
Find_Package ( SDL_mixer REQUIRED )
Find_Package ( ImageMagick REQUIRED )
Find_Package ( OpenGL REQUIRED )

FIND_PACKAGE(ImageMagickLib)

link_libraries (
   ${SDL_LIBRARY}
   ${SDLMIXER_LIBRARY}
   ${OPENGL_LIBRARIES}
   ${IMAGEMAGICKLIB_LIBRARIES} 
   ${IMAGEMAGICKLIB_CXX_LIBRARIES} 
   SDLmain                         
   chipmunk
   #${AUDIERELIB_LIBRARIES}
)

IF(SDL_FOUND)
MESSAGE("libSDL found. ${SDL_INCLUDE_DIR} ${SDL_LIBRARY}")
ELSE(SDL_FOUND)
MESSAGE("libSDL requested but not found.")
ENDIF(SDL_FOUND)

IF(IMAGEMAGICKLIB_FOUND)
MESSAGE("lib ImageMagick found. ${IMAGEMAGICKLIB_INCLUDE_DIR} ${IMAGEMAGICKLIB_LIBRARIES} ${IMAGEMAGICKLIB_CXX_LIBRARIES}")
ELSE(IMAGEMAGICKLIB_FOUND)
MESSAGE("lib ImageMagick requested but not found.")
ENDIF(IMAGEMAGICKLIB_FOUND)

IF(SDLMIXER_FOUND)
MESSAGE("libSDL_mixer found. ${SDLMIXER_INCLUDE_DIR} ${SDLMIXER_LIBRARY}")
ELSE(SDLMIXER_FOUND)
MESSAGE("libSDL_mixer requested but not found.")
ENDIF(SDLMIXER_FOUND)

IF(OPENGL_FOUND)
MESSAGE("OpenGL found. ${OPENGL_INCLUDE_DIR} ${OPENGL_LIBRARIES}")
ELSE(OPENGL_FOUND)
MESSAGE("OpenGL requested but not found.")
ENDIF(OPENGL_FOUND)

#FIND_PACKAGE(AudiereLib)
#IF(AUDIERELIB_FOUND)
#   MESSAGE("DBG lib Audiere found. ${AUDIERELIB_INCLUDE_DIR} ${AUDIERELIB_LINK_DIRECTORIES} ${AUDIERELIB_LIBRARIES}")
#  LINK_DIRECTORIES(${AUDIERELIB_LINK_DIRECTORIES})
#ELSE(AUDIERELIB_FOUND)
#   MESSAGE("DBG lib Audiere requested but not found.")
#ENDIF(AUDIERELIB_FOUND)

SUBDIRS(chipmunk demo1 demo2 demo3 demo4 demo5 physicsdemo physicsdemo2 physicsdemo3 physicsdemo4
physicsdemo5 physicsdemo6 physicsdemo7 
#physicsdemo8 blob 
hungarr lib)

SET (CMAKE_BUILD_TYPE RELEASE)


