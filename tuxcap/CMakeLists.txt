PROJECT(TUXCAP)

#include some macros from another file...
INCLUDE(${CMAKE_SOURCE_DIR}/CMakeMacros/IJMacros.txt)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )

INCLUDE(FindImageMagick)
INCLUDE(FindSDL)
INCLUDE(FindOpenGL)
INCLUDE(FindSDL_mixer)
#INCLUDE(FindPythonInterp)
INCLUDE(FindPythonLibs)

Find_Package ( SDL REQUIRED )
Find_Package ( SDL_mixer REQUIRED )
Find_Package ( ImageMagick REQUIRED )
Find_Package ( OpenGL REQUIRED )

FIND_PACKAGE(ImageMagickLib)

FIND_PACKAGE(AudiereLib)
IF(AUDIERELIB_FOUND)
           MESSAGE("DBG lib Audiere found. ${AUDIERELIB_INCLUDE_DIR} ${AUDIERELIB_LINK_DIRECTORIES} ${AUDIERELIB_LIBRARIES}")
           LINK_DIRECTORIES(${AUDIERELIB_LINK_DIRECTORIES})
ENDIF(AUDIERELIB_FOUND)

SET(MY_LINK_LIBS    
   ${SDL_LIBRARY}
   ${SDLMIXER_LIBRARY}
   ${OPENGL_LIBRARIES}
   ${IMAGEMAGICKLIB_LIBRARIES} 
   ${IMAGEMAGICKLIB_CXX_LIBRARIES} 
   SDLmain                         
   chipmunk
)

SET (MY_DIRS
    chipmunk
    demo1
    demo2 
    demo3 
    demo4 
    demo5 
    physicsdemo 
    physicsdemo2 
    physicsdemo3 
    physicsdemo4
    physicsdemo5 
    physicsdemo6 
    physicsdemo7 
    hungarr 
    lib
)

#IF(PYTHONINTERP_FOUND)
IF (PYTHON_LIBRARIES)
     MESSAGE("Python development libraries found, building TuxCap Python bindings and examples")   
#     MESSAGE("Python interpreter ${PYTHON_EXECUTABLE}")   
     MESSAGE("Python libraries ${PYTHON_LIBRARIES} include path ${PYTHON_INCLUDE_PATH}")   
     SET(MY_LINK_LIBS    ${MY_LINK_LIBS} ${PYTHON_LIBRARIES})       
     SET(MY_DIRS    ${MY_DIRS} python-bindings/PetalHero python-bindings/Sample python-bindings/Template)                
ELSE (PYTHON_LIBRARIES)
     MESSAGE("No Python development libraries found, skipping building of TuxCap Python bindings")   
ENDIF (PYTHON_LIBRARIES)
#ELSE (PYTHONINTERP_FOUND)
#     MESSAGE("No Python interpreter found, skipping building of TuxCap Python bindings")   
#ENDIF(PYTHONINTERP_FOUND)

IF(AUDIERELIB_FOUND)
        SET(MY_LINK_LIBS    ${MY_LINK_LIBS} audiere)
ENDIF(AUDIERELIB_FOUND)

link_libraries (
               ${MY_LINK_LIBS}
)

IF(SDL_FOUND)
MESSAGE("libSDL found. ${SDL_INCLUDE_DIR} ${SDL_LIBRARY}")
ELSE(SDL_FOUND)
MESSAGE(FATAL_ERROR "libSDL requested but not found.")
ENDIF(SDL_FOUND)

IF(IMAGEMAGICKLIB_FOUND)
MESSAGE("lib ImageMagick found. ${IMAGEMAGICKLIB_INCLUDE_DIR} ${IMAGEMAGICKLIB_LIBRARIES} ${IMAGEMAGICKLIB_CXX_LIBRARIES}")
ELSE(IMAGEMAGICKLIB_FOUND)
MESSAGE(FATAL_ERROR "lib ImageMagick requested but not found.")
ENDIF(IMAGEMAGICKLIB_FOUND)

IF(SDLMIXER_FOUND)
MESSAGE("libSDL_mixer found. ${SDLMIXER_INCLUDE_DIR} ${SDLMIXER_LIBRARY}")
ELSE(SDLMIXER_FOUND)
MESSAGE(FATAL_ERROR "libSDL_mixer requested but not found.")
ENDIF(SDLMIXER_FOUND)

IF(OPENGL_FOUND)
MESSAGE("OpenGL found. ${OPENGL_INCLUDE_DIR} ${OPENGL_LIBRARIES}")
ELSE(OPENGL_FOUND)
MESSAGE(FATAL_ERROR "OpenGL requested but not found.")
ENDIF(OPENGL_FOUND)

SUBDIRS( ${MY_DIRS} )

SET (CMAKE_BUILD_TYPE RELEASE)
