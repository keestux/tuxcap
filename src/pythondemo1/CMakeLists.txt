SET(MY_SOURCES  main.cpp)
# The "../../bin" puts in in BUILD/bin and that will help the program
# to find the resource directory in BUILD/bin/../Resources
SET(CurrentExe "../../bin/pythondemo1")

INCLUDE(../CMakePycapDemo.txt)

#the following rule is to copy game.py into the build directory
ADD_CUSTOM_COMMAND(
    OUTPUT game.py
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/game.py .
    DEPENDS game.py
)
ADD_CUSTOM_TARGET(gamepy1 ALL DEPENDS game.py)
