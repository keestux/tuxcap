#
# Note for MacOS developers.
# If you want to build TuxCap on MacOS you will need SDL libraries.
# One possibility for that is to install Fink, and the following
# packages: cmake sdl sdl-image sdl-image-shlibs sdl-mixer sdl-mixer-shlibs
# This will automatically include ogg and vorbis libraries too.
# For MacOS we're chosing i386. If you have a older PPC system you'll
# have to change the ARCH into "-arch ppc".
# After installing Fink you need to add /sw/bin/init.sh in your ~/.profile
# This will be enough for cmake to find the SDL libraries.
#

#
# Note about sound system.
# You can select the sound system of your choice by
# defining SOUND_SYSTEM on the command line when running
# cmake. For example:
#  (cd build/&& cmake -DSOUND_SYSTEM:STRING=AUDIERE .. && make)
# TuxCap has the modules for SDLMixer and for Audiere.
# If you want something else you have to leave it undefined.
#

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

SET(ARCH "")
IF(APPLE)
    # We need all this to be able to create a library/executable that runs on Leopard,
    # and even perhaps on Tiger (with PPC)
    #SET(ARCH "-arch i386 -arch ppc  -mmacosx-version-min=10.4 -DMAC_OS_X_VERSION_MIN_REQUIRED=1040 -isysroot /Developer/SDKs/MacOSX10.4u.sdk")
    SET(ARCH "-arch i386 -mmacosx-version-min=10.5 -DMAC_OS_X_VERSION_MIN_REQUIRED=1050")
ENDIF(APPLE)
SET(CMAKE_C_FLAGS_RELEASE   "${ARCH} -DNDEBUG -O3")
SET(CMAKE_C_FLAGS_DEBUG     "${ARCH} -Wall -g -O0 -DDEBUG")
SET(CMAKE_C_FLAGS_PROFILE   "${ARCH} -Wall -g -O0 -pg -fprofile-arcs")
SET(CMAKE_CXX_FLAGS_RELEASE "${ARCH} -DNDEBUG -O3")
SET(CMAKE_CXX_FLAGS_DEBUG   "${ARCH} -Wall -g -O0 -DDEBUG")
SET(CMAKE_CXX_FLAGS_PROFILE "${ARCH} -Wall -g -O0 -pg -fprofile-arcs")

# These packages are absolutely necessary
Find_Package ( SDL REQUIRED )
Find_Package ( SDL_image REQUIRED )
Find_Package ( OpenGL REQUIRED )

SET(MY_LINK_LIBS
    ${SDL_LIBRARY}
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

IF(NOT SOUND_SYSTEM)
    SET(SOUND_SYSTEM "UNDEFINED_SOUND_SYSTEM")
ENDIF(NOT SOUND_SYSTEM)
IF(${SOUND_SYSTEM} STREQUAL "SDL")
    Find_Package ( SDL_mixer REQUIRED )
    IF(SDLMIXER_FOUND)
        MESSAGE("libSDL_mixer found. ${SDLMIXER_INCLUDE_DIR} ${SDLMIXER_LIBRARY}")
        INCLUDE_DIRECTORIES(${SDLMIXER_INCLUDE_DIR})
    ELSE(SDLMIXER_FOUND)
        MESSAGE(FATAL_ERROR "libSDL_mixer requested but not found.")
    ENDIF(SDLMIXER_FOUND)
    SET(MY_LINK_LIBS
        ${MY_LINK_LIBS}
        ${SDLMIXER_LIBRARY}
    )
    ADD_DEFINITIONS(-DUSE_SDLMIXER)
ENDIF()

IF(${SOUND_SYSTEM} STREQUAL "AUDIERE")
    SET(AUDIERELIB_FOUND FALSE)
    FIND_PACKAGE(AudiereLib QUIET)
    IF(AUDIERELIB_FOUND)
        MESSAGE("INFO: lib Audiere found. ${AUDIERELIB_INCLUDE_DIR} ${AUDIERELIB_LINK_DIRECTORIES} ${AUDIERELIB_LIBRARIES}")
        INCLUDE_DIRECTORIES(${AUDIERELIB_INCLUDE_DIR})
        LINK_DIRECTORIES(${AUDIERELIB_LINK_DIRECTORIES})
        SET(MY_LINK_LIBS ${MY_LINK_LIBS} audiere)
    ENDIF(AUDIERELIB_FOUND)
    ADD_DEFINITIONS(-DUSE_AUDIERE)
ENDIF()

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
