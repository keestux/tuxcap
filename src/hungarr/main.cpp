#include <cstdio>
#include <exception>
#include <CommandLine.h>
#include "GameApp.h"

using namespace Sexy;
using namespace std;

int main(int argc, char** argv)
{
    int exit_code = 0;

    // Do the initial command line parsing.
    if (!CmdLine::ParseCommandLine(argc, argv)) {
        return exit_code;
    }

    try {
        GameApp app;

        if (app.ParseCommandLine(argc, argv) != 0) {
            return exit_code;
        }

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
