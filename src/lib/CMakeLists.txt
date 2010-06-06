INCLUDE_DIRECTORIES(../include ../chipmunk ../hgeparticle ../pycap)

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
	Flags.cpp 
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
	../include/Buffer.h
	../include/ButtonListener.h
	../include/ButtonWidget.h
	../include/Checkbox.h
	../include/CheckboxListener.h
	../include/Color.h
	../include/Common.h
	../include/CursorWidget.h
	../include/D3DInterface.h
	../include/DDImage.h
	../include/DDInterface.h
	../include/DescParser.h
	../include/DialogButton.h
	../include/Dialog.h
	../include/DialogListener.h
	../include/EditListener.h
	../include/EditWidget.h
	../include/Flags.h
	../include/Font.h
	../include/Graphics.h
	../include/HyperlinkWidget.h
	../include/ImageFont.h
	../include/Image.h
	../include/ImageLib.h
	../include/Insets.h
	../include/KeyCodes.h
	../include/ListListener.h
	../include/ListWidget.h
	../include/MemoryImage.h
	../include/MTRand.h
	../include/MusicInterface.h
	../include/NativeDisplay.h
	../include/NaturalCubicSpline.h
	../include/Point.h
	../include/PropertiesParser.h
	../include/Quantize.h
	../include/Ratio.h
	../include/Rect.h
	../include/ResourceManager.h
	../include/ScrollbarWidget.h
	../include/ScrollbuttonWidget.h
	../include/ScrollListener.h
	../include/SDLMixerMusicInterface.h
	../include/SDLMixerSoundInstance.h
	../include/SDLMixerSoundManager.h
	../include/SexyAppBase.h
	../include/SexyMatrix.h
	../include/Physics.h
	../include/PhysicsListener.h
	../include/SexyVector.h
	../include/SharedImage.h
	../include/Slider.h
	../include/SliderListener.h
	../include/SoundInstance.h
	../include/SoundManager.h
	../include/SWTri.h
	../include/TextWidget.h
	../include/TriVertex.h
	../include/WidgetContainer.h
	../include/Widget.h
	../include/WidgetManager.h
	../include/XMLParser.h
	../include/XMLWriter.h
	../include/ParticlePhysicsSystem.h

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
	SET(MY_HEADERS ${MY_HEADERS} ../include/AudiereLoader.h
			       ../include/AudiereMusicInterface.h
			       ../include/AudiereSoundInstance.h
			       ../include/AudiereSoundManager.h
)
ENDIF(AUDIERELIB_FOUND)

ADD_LIBRARY(tuxcap_static STATIC  ${MY_SOURCES} ${CHIPMUNK_SOURCES} ${HGE_SOURCES} ${PYCAP_SOURCES})
ADD_LIBRARY(tuxcap SHARED ${MY_SOURCES} ${CHIPMUNK_SOURCES} ${HGE_SOURCES} ${PYCAP_SOURCES})


SET_TARGET_PROPERTIES(tuxcap 
				     PROPERTIES 
				     VERSION 4.0
)

SET_TARGET_PROPERTIES(tuxcap_static 
				     PROPERTIES
				     PREFIX "lib"
				     SUFFIX ".a"
				     OUTPUT_NAME "tuxcap"
)

INSTALL(TARGETS tuxcap tuxcap_static
	LIBRARY DESTINATION lib${LIB_SUFFIX}
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
)

INSTALL(FILES 
	${MY_HEADERS}
	DESTINATION include/tuxcap
)
