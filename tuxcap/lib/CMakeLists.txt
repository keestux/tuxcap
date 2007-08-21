INCLUDE_DIRECTORIES(${TUXCAP_SOURCE_DIR}/include)

ADD_LIBRARY(TuxCap SHARED Ratio.cpp Buffer.cpp SexyAppBase.cpp MTRand.cpp Common.cpp Color.cpp Insets.cpp SexyMatrix.cpp 
Font.cpp Image.cpp ImageLib.cpp Graphics.cpp Flags.cpp Quantize.cpp NativeDisplay.cpp MemoryImage.cpp SWTri.cpp D3DInterface.cpp 
WidgetContainer.cpp WidgetManager.cpp Widget.cpp DescParser.cpp SharedImage.cpp ImageFont.cpp MusicInterface.cpp AudiereMusicInterface.cpp 
AudiereSoundManager.cpp AudiereSoundInstance.cpp AudiereLoader.cpp ButtonWidget.cpp DDImage.cpp DDInterface.cpp ResourceManager.cpp KeyCodes.cpp
XMLParser.cpp ListWidget.cpp EditWidget.cpp HyperlinkWidget.cpp ScrollbarWidget.cpp ScrollbuttonWidget.cpp Checkbox.cpp Slider.cpp Dialog.cpp  CursorWidget.cpp TextWidget.cpp
DialogButton.cpp XMLWriter.cpp PropertiesParser.cpp NaturalCubicSpline.cpp
)

INSTALL_TARGETS(/lib TuxCap)

