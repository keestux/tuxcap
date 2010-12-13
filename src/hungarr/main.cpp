#include <cstdio>
#include <exception>
#include <Logging.h>
#include "GameApp.h"

using namespace Sexy;
using namespace std;

int main(int argc, char** argv)
{
    int exit_code = 0;
    try {
        GameApp app;

        app.ParseCommandLine(argc, argv);

        app.Init();

        app.Start();
        app.Shutdown();
    }
    catch (exception * e) {
        fprintf(stderr, "%s\n", e->what());
    }
    catch (Exception * e) {
        fprintf(stderr, "%s\n", e->getMessage().c_str());
    }
    catch (...) {
        fprintf(stderr, "Oops. Unknown exception occured\n");
        exit_code = 1;
    }
    return exit_code;
}
