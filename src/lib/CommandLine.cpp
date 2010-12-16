//
// CommandLine.cpp
// A class to parse the command line options.
//

#include "Logging.h"
#include "anyoption.h"
#include "CommandLine.h"

namespace Sexy
{

CmdLine *   CmdLine::_cmdline;

bool CmdLine::ParseCommandLine(int argc, char** argv)
{
    if (_cmdline == NULL) {
        _cmdline = new CmdLine();
    }
    _cmdline->_argv0 = argv[0];

    AnyOption *opt = new AnyOption();
    _cmdline->_opt = opt;

    //opt->setVerbose(); /* print warnings about unknown options */
    opt->setVerbose();

    /* 3. SET THE USAGE/HELP   */
    opt->addUsage("");
    opt->addUsage("Usage:");
    opt->addUsage("");

    opt->addUsage(" -h  --help            Prints this help");
    /* by default all  options  will be checked on the command line and from option/resource file */
    opt->setFlag("help", 'h');   /* a flag (takes no argument), supporting long and short form */

    opt->addUsage("     --debug           enable debug");
    opt->setFlag("debug");

    opt->addUsage(" -v  --verbose         be verbose");
    opt->setFlag("verbose", 'v');

    opt->addUsage(" -w  --windowed        start in windowed mode");
    opt->setFlag("windowed", 'w');
    opt->addUsage(" -f  --fullscreen      start in fullscreen mode");
    opt->setFlag("fullscreen", 'f');

    opt->addUsage(" -p  --fps             ?");
    opt->setFlag("fps", 'p');

    opt->addUsage(" -o  --opengl          use OpenGL(EX) renderer");
    opt->setFlag("opengl", 'o');
    opt->addUsage(" -s  --software        use software renderer");
    opt->setFlag("software", 's');

    opt->addUsage("     --resource-dir DIR set the resource directory");
    opt->setOption("resource-dir");

    opt->addUsage("     --log FNAME       set the file name for logging (\"-\" is stdout)");
    opt->setOption("log");

    opt->addUsage("     --log-level LEVEL set the logging level (see below)");
    opt->addUsage(" LEVEL is a string with one or more of <name>:<severity>:<level>, separated by commas");
    opt->addUsage("   <name>     is the component name to enable logging/diagnostic for (Can be anything, you have to know what to use)");
    opt->addUsage("   <severity> is one of EMERGENCY, ALERT, CRITICAL, ERROR, WARNING, NOTICE, INFO, or DEBUG)");
    opt->addUsage("   <level>    is an integer that determines the amount of output");
    opt->setOption("log-level");

    opt->addUsage("");

    /* go through the command line and get the options  */
    opt->processCommandArgs(argc, argv);

    if (!opt->hasOptions()) {
        delete opt;
        return true;
    }

    if (opt->hasErrors() || opt->getFlag("help") || opt->getFlag('h')) {
        opt->printUsage();
        delete opt;
        return false;
    }

    if (opt->getValue("log") != NULL) {
        std::string log_fname = opt->getValue("log");
        if (log_fname == "-") {
            StdoutLogger::create_logger();
        }
        else {
            FileLogger::create_logger(log_fname);
        }
    }

    if (opt->getValue("log-level") != NULL) {
        if (!Logger::set_log_level(opt->getValue("log-level"))) {
            opt->printUsage();
            delete opt;
            return -1;
        }
    }

    // The rest of inquiries is done in SexyAppBase::ParseCommandLine
    return true;
}

}
