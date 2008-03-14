#include "PycapApp.h"

using namespace Sexy;

int main(int argc, char** argv) {
  PycapApp app;

  app.Init(argc,argv);
	app.Start();
	app.Shutdown();
	return 0;
}
