INCLUDE_DIRECTORIES(${TUXCAP_SOURCE_DIR}/include ${TUXCAP_SOURCE_DIR}/chipmunk)

ADD_LIBRARY(TuxCap SHARED Ratio.cpp Buffer.cpp SexyAppBase.cpp MTRand.cpp Common.cpp Color.cpp Insets.cpp SexyMatrix.cpp 
Font.cpp Image.cpp ImageLib.cpp Graphics.cpp Flags.cpp Quantize.cpp NativeDisplay.cpp MemoryImage.cpp SWTri.cpp D3DInterface.cpp 
WidgetContainer.cpp WidgetManager.cpp Widget.cpp DescParser.cpp SharedImage.cpp ImageFont.cpp MusicInterface.cpp  
ButtonWidget.cpp DDImage.cpp DDInterface.cpp ResourceManager.cpp KeyCodes.cpp
XMLParser.cpp ListWidget.cpp EditWidget.cpp HyperlinkWidget.cpp ScrollbarWidget.cpp ScrollbuttonWidget.cpp Checkbox.cpp Slider.cpp Dialog.cpp  CursorWidget.cpp TextWidget.cpp DialogButton.cpp XMLWriter.cpp PropertiesParser.cpp NaturalCubicSpline.cpp 
Physics.cpp
#AudiereSoundManager.cpp AudiereSoundInstance.cpp AudiereLoader.cpp AudiereMusicInterface.cpp
SDLMixerMusicInterface.cpp SDLMixerSoundManager.cpp SDLMixerSoundInstance.cpp 
)

INSTALL(TARGETS TuxCap 
LIBRARY DESTINATION lib
)

INSTALL(FILES
${TUXCAP_SOURCE_DIR}/include/AudiereLoader.h
${TUXCAP_SOURCE_DIR}/include/AudiereMusicInterface.h
${TUXCAP_SOURCE_DIR}/include/AudiereSoundInstance.h
${TUXCAP_SOURCE_DIR}/include/AudiereSoundManager.h
${TUXCAP_SOURCE_DIR}/include/Buffer.h
${TUXCAP_SOURCE_DIR}/include/ButtonListener.h
${TUXCAP_SOURCE_DIR}/include/ButtonWidget.h
${TUXCAP_SOURCE_DIR}/include/Checkbox.h
${TUXCAP_SOURCE_DIR}/include/CheckboxListener.h
${TUXCAP_SOURCE_DIR}/include/Color.h
${TUXCAP_SOURCE_DIR}/include/Common.h
${TUXCAP_SOURCE_DIR}/include/CursorWidget.h
${TUXCAP_SOURCE_DIR}/include/D3DInterface.h
${TUXCAP_SOURCE_DIR}/include/DDImage.h
${TUXCAP_SOURCE_DIR}/include/DDInterface.h
${TUXCAP_SOURCE_DIR}/include/DescParser.h
${TUXCAP_SOURCE_DIR}/include/DialogButton.h
${TUXCAP_SOURCE_DIR}/include/Dialog.h
${TUXCAP_SOURCE_DIR}/include/DialogListener.h
${TUXCAP_SOURCE_DIR}/include/EditListener.h
${TUXCAP_SOURCE_DIR}/include/EditWidget.h
${TUXCAP_SOURCE_DIR}/include/Flags.h
${TUXCAP_SOURCE_DIR}/include/Font.h
${TUXCAP_SOURCE_DIR}/include/Graphics.h
${TUXCAP_SOURCE_DIR}/include/HyperlinkWidget.h
${TUXCAP_SOURCE_DIR}/include/ImageFont.h
${TUXCAP_SOURCE_DIR}/include/Image.h
${TUXCAP_SOURCE_DIR}/include/ImageLib.h
${TUXCAP_SOURCE_DIR}/include/Insets.h
${TUXCAP_SOURCE_DIR}/include/KeyCodes.h
${TUXCAP_SOURCE_DIR}/include/ListListener.h
${TUXCAP_SOURCE_DIR}/include/ListWidget.h
${TUXCAP_SOURCE_DIR}/include/MemoryImage.h
${TUXCAP_SOURCE_DIR}/include/MTRand.h
${TUXCAP_SOURCE_DIR}/include/MusicInterface.h
${TUXCAP_SOURCE_DIR}/include/NativeDisplay.h
${TUXCAP_SOURCE_DIR}/include/NaturalCubicSpline.h
${TUXCAP_SOURCE_DIR}/include/Point.h
${TUXCAP_SOURCE_DIR}/include/PropertiesParser.h
${TUXCAP_SOURCE_DIR}/include/Quantize.h
${TUXCAP_SOURCE_DIR}/include/Ratio.h
${TUXCAP_SOURCE_DIR}/include/Rect.h
${TUXCAP_SOURCE_DIR}/include/ResourceManager.h
${TUXCAP_SOURCE_DIR}/include/ScrollbarWidget.h
${TUXCAP_SOURCE_DIR}/include/ScrollbuttonWidget.h
${TUXCAP_SOURCE_DIR}/include/ScrollListener.h
${TUXCAP_SOURCE_DIR}/include/SDLMixerMusicInterface.h
${TUXCAP_SOURCE_DIR}/include/SDLMixerSoundInstance.h
${TUXCAP_SOURCE_DIR}/include/SDLMixerSoundManager.h
${TUXCAP_SOURCE_DIR}/include/SexyAppBase.h
${TUXCAP_SOURCE_DIR}/include/SexyMatrix.h
${TUXCAP_SOURCE_DIR}/include/Physics.h
${TUXCAP_SOURCE_DIR}/include/PhysicsListener.h
${TUXCAP_SOURCE_DIR}/include/SexyVector.h
${TUXCAP_SOURCE_DIR}/include/SharedImage.h
${TUXCAP_SOURCE_DIR}/include/Slider.h
${TUXCAP_SOURCE_DIR}/include/SliderListener.h
${TUXCAP_SOURCE_DIR}/include/SoundInstance.h
${TUXCAP_SOURCE_DIR}/include/SoundManager.h
${TUXCAP_SOURCE_DIR}/include/SWTri.h
${TUXCAP_SOURCE_DIR}/include/TextWidget.h
${TUXCAP_SOURCE_DIR}/include/TriVertex.h
${TUXCAP_SOURCE_DIR}/include/WidgetContainer.h
${TUXCAP_SOURCE_DIR}/include/Widget.h
${TUXCAP_SOURCE_DIR}/include/WidgetManager.h
${TUXCAP_SOURCE_DIR}/include/XMLParser.h
${TUXCAP_SOURCE_DIR}/include/XMLWriter.h
${TUXCAP_SOURCE_DIR}/chipmunk/chipmunk.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpArbiter.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpArray.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpBB.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpBody.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpCollision.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpHashSet.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpJoint.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpPolyShape.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpShape.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpSpace.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpSpaceHash.h
${TUXCAP_SOURCE_DIR}/chipmunk/cpVect.h
${TUXCAP_SOURCE_DIR}/chipmunk/prime.h

DESTINATION include/TuxCap
)

SET_TARGET_PROPERTIES(TuxCap PROPERTIES VERSION 1)

