#include "PycapApp.h"

using namespace Sexy;

int main(int argc, char** argv) {
  PycapApp app;

  SetAppResourceFolder("../images");

  app.Init(argc,argv, false);
  app.Start();
  app.Shutdown();
  return 0;
}
