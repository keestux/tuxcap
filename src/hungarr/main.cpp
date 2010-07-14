#include "GameApp.h"

using namespace Sexy;

int main(int argc, char** argv) {
    GameApp app;

    app.Init();

    app.ParseCommandLine(argc, argv);

    app.Start();
    app.Shutdown();
    return 0;
}
