INCLUDE_DIRECTORIES(${libtuxcap_SOURCE_DIR}/tuxcap/include ${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk 
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle ${libtuxcap_SOURCE_DIR}/tuxcap/pycap )

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
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/chipmunk.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpArbiter.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpArray.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpBB.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpBody.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpCollision.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpHashSet.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpJoint.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpPolyShape.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpShape.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpSpace.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpSpaceHash.c
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpVect.c
)

SET_SOURCE_FILES_PROPERTIES(${CHIPMUNK_SOURCES}
        PROPERTIES COMPILE_FLAGS "-ffast-math -std=gnu99"
)

SET (HGE_SOURCES 
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgeparticle.cpp
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgepmanager.cpp
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgeRandom.cpp
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgerect.cpp
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgevector.cpp
)

SET_SOURCE_FILES_PROPERTIES(${HGE_SOURCES}
        PROPERTIES COMPILE_FLAGS "-ffast-math"
)

SET (PYCAP_SOURCES 
${libtuxcap_SOURCE_DIR}/tuxcap/pycap/PycapApp.cpp
${libtuxcap_SOURCE_DIR}/tuxcap/pycap/PycapBoard.cpp
${libtuxcap_SOURCE_DIR}/tuxcap/pycap/PycapResources.cpp
)

SET (MY_HEADERS
${libtuxcap_SOURCE_DIR}/tuxcap/include/Buffer.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ButtonListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ButtonWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Checkbox.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/CheckboxListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Color.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Common.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/CursorWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/D3DInterface.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/DDImage.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/DDInterface.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/DescParser.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/DialogButton.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Dialog.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/DialogListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/EditListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/EditWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Flags.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Font.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Graphics.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/HyperlinkWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ImageFont.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Image.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ImageLib.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Insets.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/KeyCodes.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ListListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ListWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/MemoryImage.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/MTRand.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/MusicInterface.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/NativeDisplay.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/NaturalCubicSpline.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Point.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/PropertiesParser.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Quantize.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Ratio.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Rect.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ResourceManager.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ScrollbarWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ScrollbuttonWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ScrollListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SDLMixerMusicInterface.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SDLMixerSoundInstance.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SDLMixerSoundManager.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SexyAppBase.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SexyMatrix.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Physics.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/PhysicsListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SexyVector.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SharedImage.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Slider.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SliderListener.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SoundInstance.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SoundManager.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/SWTri.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/TextWidget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/TriVertex.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/WidgetContainer.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/Widget.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/WidgetManager.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/XMLParser.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/XMLWriter.h
${libtuxcap_SOURCE_DIR}/tuxcap/include/ParticlePhysicsSystem.h

${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/chipmunk.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpArbiter.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpArray.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpBB.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpBody.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpCollision.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpHashSet.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpJoint.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpPolyShape.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpShape.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpSpace.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpSpaceHash.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/cpVect.h
${libtuxcap_SOURCE_DIR}/tuxcap/chipmunk/prime.h

${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgecolor.h
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgeparticle.h
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgeRandom.h
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgerect.h
${libtuxcap_SOURCE_DIR}/tuxcap/hgeparticle/hgevector.h    

${libtuxcap_SOURCE_DIR}/tuxcap/pycap/PycapApp.h
${libtuxcap_SOURCE_DIR}/tuxcap/pycap/PycapBoard.h
${libtuxcap_SOURCE_DIR}/tuxcap/pycap/PycapResources.h
)

IF(AUDIERELIB_FOUND)
        SET(MY_SOURCES ${MY_SOURCES} AudiereSoundManager.cpp AudiereSoundInstance.cpp AudiereLoader.cpp AudiereMusicInterface.cpp)
        SET(MY_HEADERS ${MY_HEADERS} ${libtuxcap_SOURCE_DIR}/tuxcap/include/AudiereLoader.h
                       ${libtuxcap_SOURCE_DIR}/tuxcap/include/AudiereMusicInterface.h
                       ${libtuxcap_SOURCE_DIR}/tuxcap/include/AudiereSoundInstance.h
                       ${libtuxcap_SOURCE_DIR}/tuxcap/include/AudiereSoundManager.h
)
ENDIF(AUDIERELIB_FOUND)

ADD_LIBRARY(tuxcap_static STATIC  ${MY_SOURCES} ${CHIPMUNK_SOURCES} ${HGE_SOURCES} ${PYCAP_SOURCES})
ADD_LIBRARY(tuxcap SHARED ${MY_SOURCES} ${CHIPMUNK_SOURCES} ${HGE_SOURCES} ${PYCAP_SOURCES})

SET_TARGET_PROPERTIES(tuxcap 
                             PROPERTIES 
                             VERSION 2
)

SET_TARGET_PROPERTIES(tuxcap_static 
                             PROPERTIES
                             PREFIX "lib"
                             SUFFIX ".a"
                             OUTPUT_NAME "tuxcap"
)

INSTALL(FILES 
${MY_HEADERS}
DESTINATION include/tuxcap
)

INSTALL(TARGETS tuxcap tuxcap_static
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib
)
