LOCAL_PATH := $(call my-dir)

###########################
#
# tuxcap shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := tuxcap

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/lib \
			$(LOCAL_PATH)/../SDL/include \
			$(LOCAL_PATH)/../SDL_image \
			$(LOCAL_PATH)/../chipmunk \
			$(LOCAL_PATH)/../hgeparticle \
			$(LOCAL_PATH)/../png

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/src/lib/Ratio.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Buffer.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/SexyAppBase.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/MTRand.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Common.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Color.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Insets.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/SexyMatrix.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Font.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Image.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ImageLib.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Graphics.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Quantize.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/NativeDisplay.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/MemoryImage.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/SWTri.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/D3DInterface.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/PVRTexture.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/TextureData.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/VertexList.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/WidgetContainer.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/WidgetManager.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Widget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/DescParser.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ImageFont.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/MusicInterface.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ButtonWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/DDImage.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/DDInterface.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ResourceManager.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/KeyCodes.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/XMLParser.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ListWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/EditWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/HyperlinkWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ScrollbarWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ScrollbuttonWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Checkbox.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Slider.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Dialog.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/CursorWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/TextWidget.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/DialogButton.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/XMLWriter.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/PropertiesParser.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/NaturalCubicSpline.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Physics.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/ParticlePhysicsSystem.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/PakInterface.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/CommandLine.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Logging.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/Timer.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/anyoption.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/GLExtensions.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/GLState.cpp) \
	$(wildcard $(LOCAL_PATH)/src/lib/IMG_savepng.c) \
	$(wildcard $(LOCAL_PATH)/src/lib/DXTTexture.cpp) \
	)
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -lz
LOCAL_STATIC_LIBRARIES := chipmunk hgeparticle png
LOCAL_SHARED_LIBRARIES := SDL SDL_image
LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES -DDEBUG -D__ANDROID__
LOCAL_CPPFLAGS := -DDEBUG -D__ANDROID__
include $(BUILD_SHARED_LIBRARY)
