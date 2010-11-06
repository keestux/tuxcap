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
	SDLMixerMusicInterface.cpp 
	SDLMixerSoundManager.cpp 
	SDLMixerSoundInstance.cpp 
	ParticlePhysicsSystem.cpp
	#FlashWidget.cpp

        PakInterface.cpp
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

SET (PYCAP_SOURCES 
	../pycap/PycapApp.cpp
	../pycap/PycapBoard.cpp
	../pycap/PycapResources.cpp
)

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
	SDLMixerMusicInterface.h
	SDLMixerSoundInstance.h
	SDLMixerSoundManager.h
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

	../hgeparticle/hgecolor.h
	../hgeparticle/hgeparticle.h
	../hgeparticle/hgeRandom.h
	../hgeparticle/hgerect.h
	../hgeparticle/hgevector.h    

	../pycap/PycapApp.h
	../pycap/PycapBoard.h
	../pycap/PycapResources.h
)

IF(AUDIERELIB_FOUND)
	SET(MY_SOURCES ${MY_SOURCES} AudiereSoundManager.cpp AudiereSoundInstance.cpp AudiereLoader.cpp AudiereMusicInterface.cpp)
	SET(MY_HEADERS ${MY_HEADERS}
            AudiereLoader.h
            AudiereMusicInterface.h
            AudiereSoundInstance.h
            AudiereSoundManager.h
)
ENDIF(AUDIERELIB_FOUND)

ADD_LIBRARY(tuxcap SHARED  ${MY_SOURCES} ${MY_HEADERS} ${CHIPMUNK_SOURCES} ${HGE_SOURCES} ${PYCAP_SOURCES})
SET_TARGET_PROPERTIES(tuxcap 
    PROPERTIES
    VERSION 4.0
    SOVERSION 4.0
    FRAMEWORK true
    PUBLIC_HEADER "${MY_HEADERS}"
)
INSTALL(TARGETS tuxcap
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
    FRAMEWORK DESTINATION "/Library/Frameworks"
    PUBLIC_HEADER DESTINATION "include/tuxcap"
)

IF(NOT APPLE)
    ADD_LIBRARY(tuxcap_static STATIC  ${MY_SOURCES} ${MY_HEADERS} ${CHIPMUNK_SOURCES} ${HGE_SOURCES} ${PYCAP_SOURCES})
    SET_TARGET_PROPERTIES(tuxcap_static
        PROPERTIES
        PREFIX "lib"
        SUFFIX ".a"
        OUTPUT_NAME "tuxcap"
        PUBLIC_HEADER "${MY_HEADERS}"
    )
    INSTALL(TARGETS tuxcap_static
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        PUBLIC_HEADER DESTINATION "include/tuxcap"
    )
ENDIF(NOT APPLE)
