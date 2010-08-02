#include <cstdio>
#include "GameApp.h"

using namespace Sexy;

int main(int argc, char** argv)
{
    int exit_code = 0;
    try {
        GameApp app;

        app.Init();
        app.Start();
        app.Shutdown();
    } catch (Exception * e) {
        fprintf(stderr, "%s\n", e->getMessage().c_str());
    } catch (...) {
        fprintf(stderr, "Oops. Unknown exception occured\n");
        exit_code = 1;
    }
    return exit_code;
}
