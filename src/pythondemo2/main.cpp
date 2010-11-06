#include <cstdio>
#include "PycapApp.h"

using namespace Sexy;

int main(int argc, char** argv)
{
    int exit_code = 0;
    try {
        PycapApp app;

        app.Init(argc, argv, false);

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
