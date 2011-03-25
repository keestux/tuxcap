SET(MY_SOURCES  main.cpp TitleScreen.cpp OptionsDialog.cpp GameOverEffect.cpp LevelupEffect.cpp 
    Board.cpp GameApp.h GameApp.cpp Res.cpp)
# The "../../bin" puts in in BUILD/bin and that will help the program
# to find the resource directory in BUILD/bin/../Resources
SET(CurrentExe "../../bin/Hungarr")

INCLUDE(../CMakeDemo.txt)
