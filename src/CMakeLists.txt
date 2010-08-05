SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

SET(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3")
SET(CMAKE_C_FLAGS_DEBUG "-Wall -g -O0 -DDEBUG")
SET(CMAKE_C_FLAGS_PROFILE "-Wall -g -O0 -pg -fprofile-arcs")
SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -DDEBUG")
SET(CMAKE_CXX_FLAGS_PROFILE "-Wall -g -O0 -pg -fprofile-arcs")

# These are absolutely necessary
INCLUDE(FindSDL)
INCLUDE(FindSDL_mixer)
INCLUDE(FindSDL_image)
INCLUDE(FindOpenGL)

Find_Package ( SDL REQUIRED )
Find_Package ( SDL_mixer REQUIRED )
Find_Package ( SDL_image REQUIRED )
Find_Package ( OpenGL REQUIRED )

SET(MY_LINK_LIBS    
    ${SDL_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${OPENGL_LIBRARIES}
)

INCLUDE(FindPythonLibs)

IF (PYTHON_LIBRARIES)
    MESSAGE("Python development libraries found, building TuxCap Python bindings and examples")
    MESSAGE("Python libraries ${PYTHON_LIBRARIES} include path ${PYTHON_INCLUDE_PATH}")
    SET(MY_LINK_LIBS    ${MY_LINK_LIBS} ${PYTHON_LIBRARIES})
    SET(MY_DIRS         ${MY_DIRS} pythondemo1 pythondemo2 pythondemo_template)
    INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
ELSE (PYTHON_LIBRARIES)
    MESSAGE("No Python development libraries found, skipping building of TuxCap Python bindings")
ENDIF (PYTHON_LIBRARIES)

IF(SDL_FOUND)
    MESSAGE("libSDL found. ${SDL_INCLUDE_DIR} ${SDL_LIBRARY}")
    INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
ELSE(SDL_FOUND)
    MESSAGE(FATAL_ERROR "libSDL requested but not found.")
ENDIF(SDL_FOUND)

IF(SDLMIXER_FOUND)
    MESSAGE("libSDL_mixer found. ${SDLMIXER_INCLUDE_DIR} ${SDLMIXER_LIBRARY}")
    INCLUDE_DIRECTORIES(${SDLMIXER_INCLUDE_DIR})
ELSE(SDLMIXER_FOUND)
    MESSAGE(FATAL_ERROR "libSDL_mixer requested but not found.")
ENDIF(SDLMIXER_FOUND)

IF(SDLIMAGE_FOUND)
    MESSAGE("libSDL_image found. ${SDLIMAGE_INCLUDE_DIR} ${SDLIMAGE_LIBRARY}")
    INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
ELSE(SDLIMAGE_FOUND)
    MESSAGE(FATAL_ERROR "libSDL_image requested but not found.")
ENDIF(SDLIMAGE_FOUND)

IF(OPENGL_FOUND)
    MESSAGE("OpenGL found. ${OPENGL_INCLUDE_DIR} ${OPENGL_LIBRARIES}")
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
ELSE(OPENGL_FOUND)
    MESSAGE(FATAL_ERROR "OpenGL requested but not found.")
ENDIF(OPENGL_FOUND)

# Optionally use Audiere library
SET(AUDIERELIB_FOUND FALSE)
FIND_PACKAGE(AudiereLib QUIET)
IF(AUDIERELIB_FOUND)
    MESSAGE("INFO: lib Audiere found. ${AUDIERELIB_INCLUDE_DIR} ${AUDIERELIB_LINK_DIRECTORIES} ${AUDIERELIB_LIBRARIES}")
    INCLUDE_DIRECTORIES(${AUDIERELIB_INCLUDE_DIR})
    LINK_DIRECTORIES(${AUDIERELIB_LINK_DIRECTORIES})
    SET(MY_LINK_LIBS ${MY_LINK_LIBS} audiere)
ENDIF(AUDIERELIB_FOUND)

SET(MY_LINK_LIBS ${MY_LINK_LIBS} util)

link_libraries(${MY_LINK_LIBS})

SUBDIRS(
    lib
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
    particledemo
    hungarr
    pythondemo1
    pythondemo2
    tuxpak
)
