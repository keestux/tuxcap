SET(MY_SOURCES main.cpp TitleScreen.cpp DemoDialog.cpp Board.cpp GameApp.cpp Res.cpp)
# The "../../bin" puts in in BUILD/bin and that will help the program
# to find the resource directory in BUILD/bin/../Resources
SET(CurrentExe "demo_5")

INCLUDE(../CMakeDemo.txt)
