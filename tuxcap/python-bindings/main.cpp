#include "PycapApp.h"

using namespace Sexy;

int main(int argc, char** argv) {
	PycapApp app;

	app.Init();
	app.Start();
	app.Shutdown();
	return 0;
}
