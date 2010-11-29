SET(DIRS
    fonts
    images
    music
    properties
    sounds
)
SET(RESOURCES_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
ADD_CUSTOM_COMMAND(
    OUTPUT main.pak
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${CMAKE_BINARY_DIR}/src/tuxpak/tuxpak -c ${CMAKE_CURRENT_BINARY_DIR}/main.pak ${DIRS}
    DEPENDS tuxpak
)
ADD_CUSTOM_TARGET(mainpak ALL
    DEPENDS main.pak
)
