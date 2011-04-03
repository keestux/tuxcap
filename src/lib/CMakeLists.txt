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

# Note. The "." is needed because pycap and hgeparticle wants include files from here
INCLUDE_DIRECTORIES(. ../chipmunk ../hgeparticle ../pycap)

SET (MY_SOURCES
	Ratio.cpp 
	Buffer.cpp 
	SexyAppBase.cpp 
	MTRand.cpp 
	Common.cpp 
	Color.cpp 
	Insets.cpp 
	SexyMatrix.cpp 
	Font.cpp 
	Image.cpp 
	ImageLib.cpp 
	Graphics.cpp 
	Quantize.cpp 
	NativeDisplay.cpp 
	MemoryImage.cpp 
	SWTri.cpp 
	D3DInterface.cpp
        PVRTexture.cpp
        TextureData.cpp
        VertexList.cpp
	WidgetContainer.cpp 
	WidgetManager.cpp 
	Widget.cpp 
	DescParser.cpp 
	SharedImage.cpp 
	ImageFont.cpp 
	MusicInterface.cpp  
	ButtonWidget.cpp 
	DDImage.cpp 
	DDInterface.cpp 
	ResourceManager.cpp 
	KeyCodes.cpp
	XMLParser.cpp 
	ListWidget.cpp 
	EditWidget.cpp 
	HyperlinkWidget.cpp 
	ScrollbarWidget.cpp 
	ScrollbuttonWidget.cpp 
	Checkbox.cpp 
	Slider.cpp 
	Dialog.cpp  
	CursorWidget.cpp 
	TextWidget.cpp 
	DialogButton.cpp 
	XMLWriter.cpp 
	PropertiesParser.cpp 
	NaturalCubicSpline.cpp 
	Physics.cpp
	ParticlePhysicsSystem.cpp
	#FlashWidget.cpp

	PakInterface.cpp

	CommandLine.cpp
	Logging.cpp
	Timer.cpp

	anyoption.cpp

        GLExtensions.cpp
)
# Linux may need this for unordered_map
#SET_SOURCE_FILES_PROPERTIES(${MY_SOURCES}
#    PROPERTIES COMPILE_FLAGS "-std=c++0x"
#)

SET (MY_HEADERS
	Buffer.h
	ButtonListener.h
	ButtonWidget.h
	Checkbox.h
	CheckboxListener.h
	Color.h
	Common.h
	CursorWidget.h
	D3DInterface.h
        PVRTexture.h
        TextureData.h
        VertexList.h
	DDImage.h
	DDInterface.h
	DescParser.h
	DialogButton.h
	Dialog.h
	DialogListener.h
	EditListener.h
	EditWidget.h
	Flags.h
	Font.h
	Graphics.h
	HyperlinkWidget.h
	ImageFont.h
	Image.h
	ImageLib.h
	Insets.h
	KeyCodes.h
	ListListener.h
	ListWidget.h
	MemoryImage.h
	MTRand.h
	MusicInterface.h
	NativeDisplay.h
	NaturalCubicSpline.h
	Point.h
	PropertiesParser.h
	Quantize.h
	Ratio.h
	Rect.h
	ResourceManager.h
	ScrollbarWidget.h
	ScrollbuttonWidget.h
	ScrollListener.h
	SexyAppBase.h
	SexyMatrix.h
	Physics.h
	PhysicsListener.h
	SexyVector.h
	SharedImage.h
	Slider.h
	SliderListener.h
	SoundInstance.h
	SoundManager.h
	SWTri.h
	TextWidget.h
	TriVertex.h
	WidgetContainer.h
	Widget.h
	WidgetManager.h
	XMLParser.h
	XMLWriter.h
	ParticlePhysicsSystem.h

	PakInterface.h

	CommandLine.h
	Logging.h
	Timer.h

	anyoption.h
)

SET (CHIPMUNK_SOURCES 
	../chipmunk/chipmunk.c
	../chipmunk/cpArbiter.c
	../chipmunk/cpArray.c
	../chipmunk/cpBB.c
	../chipmunk/cpBody.c
	../chipmunk/cpCollision.c
	../chipmunk/cpHashSet.c
	../chipmunk/cpJoint.c
	../chipmunk/cpPolyShape.c
	../chipmunk/cpShape.c
	../chipmunk/cpSpace.c
	../chipmunk/cpSpaceHash.c
	../chipmunk/cpVect.c
)
SET_SOURCE_FILES_PROPERTIES(${CHIPMUNK_SOURCES}
    PROPERTIES COMPILE_FLAGS "-ffast-math -std=gnu99"
)

SET (CHIPMUNK_HEADERS
	../chipmunk/chipmunk.h
	../chipmunk/cpArbiter.h
	../chipmunk/cpArray.h
	../chipmunk/cpBB.h
	../chipmunk/cpBody.h
	../chipmunk/cpCollision.h
	../chipmunk/cpHashSet.h
	../chipmunk/cpJoint.h
	../chipmunk/cpPolyShape.h
	../chipmunk/cpShape.h
	../chipmunk/cpSpace.h
	../chipmunk/cpSpaceHash.h
	../chipmunk/cpVect.h
	../chipmunk/prime.h
)

SET (HGE_SOURCES 
	../hgeparticle/hgeparticle.cpp
	../hgeparticle/hgepmanager.cpp
	../hgeparticle/hgeRandom.cpp
	../hgeparticle/hgerect.cpp
	../hgeparticle/hgevector.cpp
)
SET_SOURCE_FILES_PROPERTIES(${HGE_SOURCES}
    PROPERTIES COMPILE_FLAGS "-ffast-math"
)

SET (HGE_HEADERS
	../hgeparticle/hgecolor.h
	../hgeparticle/hgeparticle.h
	../hgeparticle/hgeRandom.h
	../hgeparticle/hgerect.h
	../hgeparticle/hgevector.h
)

SET (PYCAP_SOURCES
	../pycap/PycapApp.cpp
	../pycap/PycapBoard.cpp
	../pycap/PycapResources.cpp
)

SET (PYCAP_HEADERS
	../pycap/PycapApp.h
	../pycap/PycapBoard.h
	../pycap/PycapResources.h
)

IF(SDLMIXER_FOUND)
	SET(MY_SOURCES ${MY_SOURCES}
            SDLMixerMusicInterface.cpp
            SDLMixerSoundManager.cpp
            SDLMixerSoundInstance.cpp
        )
	SET(MY_HEADERS ${MY_HEADERS}
            SDLCommon.h
            SDLMixerMusicInterface.h
            SDLMixerSoundInstance.h
            SDLMixerSoundManager.h
        )
ENDIF(SDLMIXER_FOUND)

IF(AUDIERELIB_FOUND)
	SET(MY_SOURCES ${MY_SOURCES}
            AudiereSoundManager.cpp
            AudiereSoundInstance.cpp
            AudiereLoader.cpp
            AudiereMusicInterface.cpp
        )
	SET(MY_HEADERS ${MY_HEADERS}
            AudiereLoader.h
            AudiereMusicInterface.h
            AudiereSoundInstance.h
            AudiereSoundManager.h
        )
ENDIF(AUDIERELIB_FOUND)

ADD_LIBRARY(tuxcap SHARED
    ${MY_SOURCES} ${MY_HEADERS}
    ${CHIPMUNK_SOURCES} ${CHIPMUNK_HEADERS}
    ${HGE_SOURCES} ${HGE_HEADERS}
    ${PYCAP_SOURCES} ${PYCAP_HEADERS}
    ${TINYXML_SOURCES} ${TINYXML_HEADERS}
)
SET_TARGET_PROPERTIES(tuxcap 
    PROPERTIES
    VERSION 4.0
    SOVERSION 4.0
    FRAMEWORK true
    PUBLIC_HEADER "${MY_HEADERS};${CHIPMUNK_HEADERS};${HGE_HEADERS};${PYCAP_HEADERS}"
)
IF(APPLE)
    # The following command sets a relative path for the tuxcap framework.
    # This is more convenient when creating an application on MacOS (.App)
    # structure where all required frameworks are included in the .App tree.
    #  install_name_tool -id @executable_path/../Frameworks/tuxcap.framework/Versions/4.0/tuxcap build/src/lib/tuxcap.framework/tuxcap
    ADD_CUSTOM_TARGET(set-relpath ALL
        COMMAND echo install_name_tool -id @executable_path/../Frameworks/tuxcap.framework/Versions/4.0/tuxcap ${CMAKE_CURRENT_BINARY_DIR}/tuxcap.framework/tuxcap
        COMMAND install_name_tool -id @executable_path/../Frameworks/tuxcap.framework/Versions/4.0/tuxcap ${CMAKE_CURRENT_BINARY_DIR}/tuxcap.framework/tuxcap
        DEPENDS tuxcap
    )
ENDIF(APPLE)
INSTALL(TARGETS tuxcap
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    FRAMEWORK DESTINATION "$ENV{HOME}/Library/Frameworks"
    PUBLIC_HEADER DESTINATION "include/tuxcap"
)

IF(NOT APPLE)
    ADD_LIBRARY(tuxcap_static STATIC
        ${MY_SOURCES} ${MY_HEADERS}
        ${CHIPMUNK_SOURCES} ${CHIPMUNK_HEADERS}
        ${HGE_SOURCES} ${HGE_HEADERS}
        ${PYCAP_SOURCES} ${PYCAP_HEADERS}
    )
    SET_TARGET_PROPERTIES(tuxcap_static
        PROPERTIES
        PREFIX "lib"
        SUFFIX ".a"
        OUTPUT_NAME "tuxcap"
        PUBLIC_HEADER "${MY_HEADERS};${CHIPMUNK_HEADERS};${HGE_HEADERS};${PYCAP_HEADERS}"
    )
    INSTALL(TARGETS tuxcap_static
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        PUBLIC_HEADER DESTINATION "include/tuxcap"
    )
ENDIF(NOT APPLE)
