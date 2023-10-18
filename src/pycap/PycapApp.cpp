//--------------------------------------------------
// PycapApp
//
// Pycap application object
//
// Jarrad 'Farbs' Woods
// W.P. van Paassen
// Tony Oakden
//--------------------------------------------------

// includes
#include <stdlib.h>
#include "PycapApp.h"
#include "PycapBoard.h"

#include "WidgetManager.h"
#include "Graphics.h"
#include "Font.h"
#include "SoundManager.h"
#include "SoundInstance.h"
#include "MusicInterface.h"
#include "KeyCodes.h"
#include "Common.h"
#include "DDImage.h"
#include "SexyMatrix.h"
#include "Rect.h"
#include "Logging.h"

#ifndef INITGUID
#define INITGUID
#endif

#include <Python.h>

// namespace
using namespace Sexy;

// static data definition
PycapApp* PycapApp::sApp = NULL;

// functions

//--------------------------------------------------
// PycapApp
//--------------------------------------------------

PycapApp::PycapApp()
{
    // initialize own members
    sApp           = this;
    pModule        = NULL;
    pDict          = NULL;

    mBoard         = NULL;
    mResources     = NULL;
    mResFailed     = false;
    mPythonHome    = "";
    mPythonPath    = "";
    mPythonHomeSet = false;
    mPythonPathSet = false;
    mBundled       = false;
    //-------------------
}

//--------------------------------------------------
// ~PycapApp
//--------------------------------------------------

PycapApp::~PycapApp()
{
    // clean up board if necessary
    if (mBoard != NULL) {
        mWidgetManager->RemoveWidget(mBoard);
        delete mBoard;
    }

    // clean up resources object
    if (mResources != NULL) {
        delete mResources;
    }

    // clean up app pointer if it's pointing to this
    if (sApp == this) {
        sApp = NULL;
    }

    // clean up python
    if (pModule) {
        Py_DECREF(pModule); // drop the module
    }
    Py_Finalize(); // shut down the interpreter

    if (mBundled) {
        unsetenv("PYTHONPATH");
        unsetenv("PYTHONHOME");

        if (mPythonHomeSet)
            setenv("PYTHONHOME", mPythonHome.c_str(), 0);
        if (mPythonPathSet)
            setenv("PYTHONPATH", mPythonPath.c_str(), 0);
    }
}

//--------------------------------------------------
// Init
//--------------------------------------------------

void PycapApp::Init(int argc, char*argv[], bool bundled)
{
    // Set up python

    SexyAppBase::ParseCommandLine(argc, argv);

    mBundled = bundled;
    if (mBundled) {
        // We don't want users to override PYTHONPATH
        char* env = getenv("PYTHONPATH");
        if (env != NULL) {
            mPythonPathSet = true;
            mPythonPath = std::string(env);
            unsetenv("PYTHONPATH");
        }

        env = getenv("PYTHONHOME");
        if (env != NULL) {
            mPythonHomeSet = true;
            mPythonHome = std::string(env);
            unsetenv("PYTHONHOME");
        }

        std::string bindirname = GetFileDir(GetFullPath(argv[0]));
        // Make <bindir>/.. the root of the Python environment
        // This sets sys.exec_prefix (more or less)
        setenv("PYTHONHOME", GetFileDir(bindirname).c_str(), 0);
        // In case we don't have a full python environment
        setenv("PYTHONPATH", bindirname.c_str(), 0);
    }

    // This is also done at SexyAppBase::Init() but that won't be executed before
    // it is too late.
    if (GetAppResourceFolder() == "" && mArgv0 != "") {
        // ResourceFolder not set.
        // Use the directory of the program instead.
        std::string bindir = GetFileDir(std::string(mArgv0), true);
        std::string rscDir = determineResourceFolder(bindir);
        LOG(mLogFacil, 2, Logger::format("determineResourceFolder = '%s'", rscDir.c_str()));
        if (rscDir != "") {
            LOG(mLogFacil, 1, Logger::format("Setting AppResourceFolder to '%s'\n", rscDir.c_str()));
            SetAppResourceFolder(rscDir);
        }
    } else {
        LOG(mLogFacil, 1, Logger::format("AppResourceFolder = '%s'", GetAppResourceFolder().c_str()));
    }

    PyImport_AppendInittab("Pycap", NULL);
    PyImport_AppendInittab("PycapRes", NULL);
    Py_Initialize();

    PyRun_SimpleString("import sys");

    if (!mBundled) {
        PyRun_SimpleString("import os");
        PyRun_SimpleString((std::string("argv0 = '") + argv[0] + "'").c_str());
        PyRun_SimpleString(std::string("_mybindir = os.path.dirname(argv0)").c_str());
	// Add <bindir> to the PATH (sys.path) so that we can find game.py
        PyRun_SimpleString(std::string("sys.path.insert(0, os.path.abspath(_mybindir))").c_str());


	if (GetAppResourceFolder() != "") {
	    // Add AppResourceDir to the PATH (sys.path) so that we can find game.py there too
	    PyRun_SimpleString(std::string("_myresdir = '" + GetAppResourceFolder() + "'").c_str());
	    PyRun_SimpleString(std::string("sys.path.insert(0, os.path.abspath(_myresdir))").c_str());
	}
    }

    if (mDebug)
        PyRun_SimpleString("print 'sys.path', '\\n        '.join(sys.path)");

    // Set up Pycap module
    static PyMethodDef resMethods[] = {
        {"markDirty", pMarkDirty, METH_VARARGS, "markDirty()\nMark the screen dirty & call a refresh."},
        {"fillRect", pFillRect, METH_VARARGS, "fillRect( x, y, width, height )\nFill a specified rect with the current colour."},
        {"setColour", pSetColour, METH_VARARGS, "setColour( red, green, blue, alpha )\nSet the draw colour. Use a value between 0 and 255 for each component."},
        {"setFont", pSetFont, METH_VARARGS, "setFont( font )\nSet the active font."},
        {"setColourize", pSetColourize, METH_VARARGS, "setColourize( on )\nEnable/Disable colourized drawing."},
        {"drawLine", pDrawLine, METH_VARARGS, "draw a line using the start en end position"},
        {"drawTri", pDrawTri, METH_VARARGS, "Fills a triangle"},
        {"drawQuad", pDrawQuad, METH_VARARGS, "Fills a quad"},
        {"drawQuadTextured", pDrawQuadTextured, METH_VARARGS, "Fills a quad with a texture"},
        {"drawImage", pDrawImage, METH_VARARGS, "drawImage( image, x, y )\nDraw an image resource at pixel coords."},
        {"drawImageF", pDrawImageF, METH_VARARGS, "drawImageF( image, fx, fy )\nDraw an image resource at float coords."},
        {"drawImageRot", pDrawImageRot, METH_VARARGS, "drawImageRot( image, x, y, angle )\nDraw an image resource at pixel coords rotated by a given angle."},
        {"drawImageRotF", pDrawImageRotF, METH_VARARGS, "drawImageRot( image, x, y, angle )\nDraw an image resource at float coords rotated by a given angle."},
        {"drawImageRotScaled", pDrawImageRotScaled, METH_VARARGS, "Rotate, scale and draw an image resource at float coords."},
        {"drawImageScaled", pDrawImageScaled, METH_VARARGS, "drawImageScaled( image, x, y, width, height )\nScale and draw an image resource at int coords."},
        {"drawString", pDrawString, METH_VARARGS, "drawString( string, x, y )\nWrite a given string to the screen using the current font."},
        {"showMouse", pShowMouse, METH_VARARGS, "showMouse( show )\nShow or hide the mouse cursor."},
        {"drawmodeNormal", pDrawmodeNormal, METH_VARARGS, "drawmodeNormal()\nSet the drawing mode to normal."},
        {"drawmodeAdd", pDrawmodeAdd, METH_VARARGS, "drawmodeAdd()\nSet the drawing mode to additive."},
        {"playSound", pPlaySound, METH_VARARGS, "playSound( sound, volume, panning, pitch )\nPlay a sound, providing id, and optionally volume, panning, and pitch adjust."},
        {"readReg", pReadReg, METH_VARARGS, "readReg( key )\nRead an entry from the system registry."},
        {"writeReg", pWriteReg, METH_VARARGS, "writeReg( key, data )\nWrite an entry to the system registry."},
        {"playTune", pPlayTune, METH_VARARGS, "playTune( tune, loopCount )\nPlay a music tune, with optional loop parameter."},
        {"stopTune", pStopTune, METH_VARARGS, "stopTune( tune )\nStop playing a musictune. If not specified, all tunes are stopped."},
        {"setTuneVolume", pSetTuneVolume, METH_VARARGS, "setTuneVolume( index, volume )\nChange the volume for a song"},
        {"setVolume", pSetVolume, METH_VARARGS, "setVolume( volume )\nChange the global volume for all music"},
        {"setClipRect", pSetClipRect, METH_VARARGS, "setClipRect( x, y, width, height )\nSet the clipping rectangle."},
        {"clearClipRect", pClearClipRect, METH_VARARGS, "clearClipRect()\nClear the clipping rectangle."},
        {"setTranslation", pSetTranslation, METH_VARARGS, "setTranslation( x, y )\nSet the translation applied to all draw calls."},
        {"setFullscreen", pSetFullscreen, METH_VARARGS, "setFullscreen( fullscreen )\nSet whether the app should be in fullscreen or windowed mode."},
        {"getFullscreen", pGetFullscreen, METH_VARARGS, "getFullscreen()\nGet whether the app is in fullscreen or windowed mode."},
        {"getKeyCode", pGetKeyCode, METH_VARARGS, "getKeyCode\n."},
        {"allowAllAccess", pAllowAllAccess, METH_VARARGS, "allowAllAccess( fileName )\nTell the OS that all users can view and modify a file. Required for Vista."},
        {"getAppDataFolder", pGetAppDataFolder, METH_VARARGS, "getAppDataFolder()\nGet the folder that game data should be saved to. Required for Vista."},
        {"getAppResourceFolder", pGetAppResourceFolder, METH_VARARGS, "getAppResourceFolder()\nGet the folder where the game resources are stored. Required for GNU/Linux."},
        {"getIs3DAccelerated", pGetIs3DAccelerated, METH_VARARGS, "getIs3DAccelerated()\nReturns if the game has 3D acceleration enabled"},
        {"set3DAccelerated", pSet3DAccelerated, METH_VARARGS, "set whether application has 3D acceleration enabled or not"},
        {"isKeyDown", pIsKeyDown, METH_VARARGS, "isKeyDown()\nReturns a boolean indicating if the queried key is down"},
        {"getUserLanguage", pGetUserLanguage, METH_VARARGS, "getUserLanguage returns a string with the user locale"},
        {NULL, NULL, 0, NULL}
    };

    PyModuleDef pycap = {
        PyModuleDef_HEAD_INIT,
        "Pycap",
        "",
        -1,
        resMethods
    };

    PyModule_Create(&pycap);
    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in PycapApp(), while importing Pycap module.");
        PyErr_Print();
        return;
    }

    // Open game module
    PyObject *pName;
    pName = PyUnicode_FromString("game");
    if (pName == NULL) {
        PyErr_SetString(PyExc_Exception, "Failed to create PyString game.py.");
        PyErr_Print();
        return;
    }

    pModule = PyImport_Import(pName);
    if (pModule == NULL) {
        PyErr_SetString(PyExc_Exception, "Failed to import game.py.");
        PyErr_Print();
        return; // we're screwed.
    }
    Py_DECREF(pName);

    pDict = PyModule_GetDict(pModule); // grab namespace dictionary

    //-------------------
    // Initialize members

    // inherited
    // read inherited members from python dictionary
    PyObject *iniDict;
    iniDict = PyDict_GetItemString(pDict, "appIni");
    if (iniDict && PyDict_Check(iniDict)) {
        PyObject *inObject;

        inObject = PyDict_GetItemString(iniDict, "mCompanyName");
        if (inObject && PyUnicode_Check(inObject)) {
            mCompanyName = PyUnicode_AsUTF8(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mCompanyName correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mFullCompanyName");
        if (inObject && PyUnicode_Check(inObject)) {
            mFullCompanyName = PyUnicode_AsUTF8(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mFullCompanyName correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mProdName");
        if (inObject && PyUnicode_Check(inObject)) {
            mProdName = PyUnicode_AsUTF8(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mProdName correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mProductVersion");
        if (inObject && PyUnicode_Check(inObject)) {
            mProductVersion = PyUnicode_AsUTF8(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mProductVersion correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mTitle");
        if (inObject && PyUnicode_Check(inObject)) {
            mTitle = PyUnicode_AsUTF8(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mTitle correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mRegKey");
        if (inObject && PyUnicode_Check(inObject)) {
            mRegKey = PyUnicode_AsUTF8(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mRegKey correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mWidth");
        if (inObject && PyLong_Check(inObject)) {
            mWidth = PyLong_AsLong(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mWidth correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mHeight");
        if (inObject && PyLong_Check(inObject)) {
            mHeight = PyLong_AsLong(inObject);
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mHeight correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mAutoEnable3D");
        if (inObject && PyLong_Check(inObject)) {
            mAutoEnable3D = PyLong_AsLong(inObject) == 1;
        } else {
            PyErr_SetString(PyExc_Exception, "appIni doesn't specify mAutoEnable3D correctly");
            PyErr_Print();
            return;
        }

        inObject = PyDict_GetItemString(iniDict, "mTest3D");
        if (inObject && PyLong_Check(inObject)) {
            mTest3D = PyLong_AsLong(inObject) == 1;
        }

        inObject = PyDict_GetItemString(iniDict, "mWindowIconBMP");
        if (inObject && PyUnicode_Check(inObject)) {
            mWindowIconBMP = PyUnicode_AsUTF8(inObject);
        }
    } else {
        PyErr_SetString(PyExc_Exception, "appIni object is missing or not a dict");
        PyErr_Print();
        return;
    }

    if (mRegKey.empty()) {
        mRegKey = "TuxCap";
    }

    // Call parent. This will set AppResourceFolder, I hope.
    SexyAppBase::Init();

    // Redirect stdout and stderr to files (since we can't seem to use console output)

    // Note. AppDataFolder is set in SexyAppBase::Init()
    if (!FileExists(GetAppDataFolder() + "out.txt")) {
        CreateFile(GetAppDataFolder() + "out.txt");
        CreateFile(GetAppDataFolder() + "err.txt");
    }

    PyRun_SimpleString(("sys.stdout = open( '" + GetAppDataFolder() + "out.txt', 'w' )").c_str());
    PyRun_SimpleString(("sys.stderr = open( '" + GetAppDataFolder() + "err.txt', 'w' )").c_str());
}

//--------------------------------------------------
// LoadingThreadProc
//--------------------------------------------------

void PycapApp::LoadingThreadProc()
{
    // call parent (empty at the moment, but a good habit)
    SexyAppBase::LoadingThreadProc();

    // create the res object
    mResources = new PycapResources();
}

//--------------------------------------------------
// LoadingThreadCompleted
//--------------------------------------------------

void PycapApp::LoadingThreadCompleted()
{
    // call parent
    SexyAppBase::LoadingThreadCompleted();

    // check for a failed resource load (not using mLoadingFailed as this terminates the app badly)
    if (mResFailed || !PycapApp::sApp->pDict) {
        // Nothing much happens if we return before adding the board... just a black screen
        // Error message widget should be added here
        PyErr_SetString(PyExc_Exception, "The game did not load properly. Sorry, but it's not going to work.");
        PyErr_Print();
        mShutdown = true;
        return;
    }

    // create the initial board object
    PycapBoard* newBoard = new PycapBoard();

    // remove current board if appropriate
    if (mBoard != NULL) {
        mWidgetManager->RemoveWidget(mBoard);
    }

    // store link
    mBoard = newBoard;

    // resize the board to the application size
    newBoard->Resize(0, 0, PycapApp::sApp->mWidth, PycapApp::sApp->mHeight);

    // add the board widget
    mWidgetManager->AddWidget(newBoard);

    // give the board focus
    mWidgetManager->SetFocus(newBoard);
}

//--------------------------------------------------
// GotFocus
//--------------------------------------------------

void PycapApp::GotFocus()
{
    PyObject* pGFFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "gotFocus");

    if (pGFFunc && PyCallable_Check(pGFFunc)) {
        PyObject_CallObject(pGFFunc, NULL);
    }
}

//--------------------------------------------------
// LostFocus
//--------------------------------------------------

void PycapApp::LostFocus()
{
    PyObject* pLFFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "lostFocus");

    if (pLFFunc && PyCallable_Check(pLFFunc)) {
        PyObject_CallObject(pLFFunc, NULL);
    }
}

//--------------------------------------------------
// SwitchScreenMode
//--------------------------------------------------

void PycapApp::SwitchScreenMode(bool wantWindowed, bool is3d)
{
    // Super
    SexyAppBase::SwitchScreenMode(wantWindowed, is3d);

    // attempt to call python fullscreen or windowed notifier
    // failed attempts still notify, but notify correctly
    if (mIsWindowed) {
        // windowed
        PyObject* pWFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "onWindowed");
        if (pWFunc && PyCallable_Check(pWFunc)) {
            PyObject_CallObject(pWFunc, NULL);
        }
    } else {
        // fullscreen
        PyObject* pFSFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "onFullscreen");
        if (pFSFunc && PyCallable_Check(pFSFunc)) {
            PyObject_CallObject(pFSFunc, NULL);
        }
    }
}


//--------------------------------------------------
// pMarkDirty
//--------------------------------------------------

PyObject* PycapApp::pMarkDirty(PyObject* self, PyObject* args)
{
    // mark the board as dirty
    sApp->mBoard->MarkDirty();

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetColour
//--------------------------------------------------

PyObject* PycapApp::pSetColour(PyObject* self, PyObject* args)
{
    // parse the arguments
    int r, g, b, a;
    if (!PyArg_ParseTuple(args, "iiii", &r, &g, &b, &a)) {
        PyErr_SetString(PyExc_Exception, "setColour: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "SetColour() failed: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // create new colour object, set colour
    graphics->SetColor(Color(r, g, b, a));

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetFont
//--------------------------------------------------

PyObject* PycapApp::pSetFont(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    if (!PyArg_ParseTuple(args, "i", &i)) {
        PyErr_SetString(PyExc_Exception, "setFont: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "setFont() failed: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the font
    Font* font = sApp->mResources->getFont(i);
    if (!font) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "setFont: Failed to reference font.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // set the active font
    graphics->SetFont(font);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetColourize
//--------------------------------------------------

PyObject* PycapApp::pSetColourize(PyObject* self, PyObject* args)
{
    // parse the arguments
    int colourize;
    if (!PyArg_ParseTuple(args, "i", &colourize)) {
        PyErr_SetString(PyExc_Exception, "setColourize: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "setColourize: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // create new colour object, set colour
    graphics->SetColorizeImages(colourize != 0);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pFillRect
//--------------------------------------------------

PyObject* PycapApp::pFillRect(PyObject* self, PyObject* args)
{
    // parse the arguments
    int x, y, w, h;
    if (!PyArg_ParseTuple(args, "iiii", &x, &y, &w, &h)) {
        PyErr_SetString(PyExc_Exception, "fillRect: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "fillRect: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // create new colour object, set colour
    graphics->FillRect(x, y, w, h);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawLine
//--------------------------------------------------

PyObject* PycapApp::pDrawLine(PyObject* self, PyObject* args)
{
    // parse the arguments
    int sx, sy, ex, ey;
    if (!PyArg_ParseTuple(args, "iiii", &sx, &sy, &ex, &ey)) {
        PyErr_SetString(PyExc_Exception, "drawLine: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawLine: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // draw the line
    graphics->DrawLine(sx, sy, ex, ey);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawImage
//--------------------------------------------------

PyObject* PycapApp::pDrawImage(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i, x, y;
    if (!PyArg_ParseTuple(args, "iii", &i, &x, &y)) {
        PyErr_SetString(PyExc_Exception, "drawImage: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawImage: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the image
    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "drawImage: Failed to reference image.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // perform the blit
    graphics->DrawImage(image, x, y);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawImageF
//--------------------------------------------------

PyObject* PycapApp::pDrawImageF(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    float x, y;
    if (!PyArg_ParseTuple(args, "iff", &i, &x, &y)) {
        PyErr_SetString(PyExc_Exception, "drawImageF: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawImageF: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the image
    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "drawImageF: Failed to reference image.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // perform the blit
    graphics->DrawImageF(image, x, y);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawImageRot
//--------------------------------------------------

PyObject* PycapApp::pDrawImageRot(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    float x, y, r;
    if (!PyArg_ParseTuple(args, "ifff", &i, &x, &y, &r)) {
        PyErr_SetString(PyExc_Exception, "drawImageRot: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawImageRot: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the image
    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "drawImageRot: Failed to reference image.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // perform the blit
    graphics->DrawImageRotated(image, (int) x, (int) y, r);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawImageRotF
//--------------------------------------------------

PyObject* PycapApp::pDrawImageRotF(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    float x, y, r;
    if (!PyArg_ParseTuple(args, "ifff", &i, &x, &y, &r)) {
        PyErr_SetString(PyExc_Exception, "drawImageRotF: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawImageRotF: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the image
    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "drawImageRotF: Failed to reference image.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // perform the blit
    graphics->DrawImageRotatedF(image, x, y, r);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawImageScaled
//--------------------------------------------------

PyObject* PycapApp::pDrawImageScaled(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    float x, y, w, h;
    if (!PyArg_ParseTuple(args, "iffff", &i, &x, &y, &w, &h)) {
        PyErr_SetString(PyExc_Exception, "drawImageScaled: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawImageScaled: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the image
    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "drawImageScaled:Failed to reference image.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // perform the blit

    if (sApp->Is3DAccelerated())
        graphics->DrawImage(image, (int) x, (int) y, (int) w, (int) h);
    else {
        if (w < 0.0) {
            w = -w;
            graphics->DrawImageMirror(image, Rect((int) (x - w), (int) y, (int) w, (int) h), Rect(0, 0, image->GetWidth(), image->GetHeight()), true);
        } else {
            graphics->DrawImage(image, (int) x, (int) y, (int) w, (int) h);
        }
    }

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawString
//--------------------------------------------------

PyObject* PycapApp::pDrawString(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* string;
    float x, y;
    if (!PyArg_ParseTuple(args, "sff", &string, &x, &y)) {
        PyErr_SetString(PyExc_Exception, "drawString: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawString: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // perform the blit
    graphics->DrawString(string, (int) x, (int) y);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pShowMouse
//--------------------------------------------------

PyObject* PycapApp::pShowMouse(PyObject* self, PyObject* args)
{
    // parse the arguments
    int show;
    if (!PyArg_ParseTuple(args, "i", &show)) {
        PyErr_SetString(PyExc_Exception, "showMouse: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test argument
    if (show) {
        sApp->SetCursor(CURSOR_POINTER);
    } else {
        sApp->SetCursor(CURSOR_NONE);
    }

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawmodeNormal
//--------------------------------------------------

PyObject* PycapApp::pDrawmodeNormal(PyObject* self, PyObject* args)
{
    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawmodeNormal: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // set the draw mode
    graphics->SetDrawMode(Graphics::DRAWMODE_NORMAL);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawmodeAdd
//--------------------------------------------------

PyObject* PycapApp::pDrawmodeAdd(PyObject* self, PyObject* args)
{
    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawmodeAdd: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // set the draw mode
    graphics->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pPlaySound
//--------------------------------------------------

PyObject* PycapApp::pPlaySound(PyObject* self, PyObject* args)
{
    // parse the arguments
    // required
    int index;
    // optional
    float volume = 1.0f;
    float panning = 0.0f;
    float pitchAdjust = 0.0f;
    if (!PyArg_ParseTuple(args, "i|fff", &index, &volume, &panning, &pitchAdjust)) {
        PyErr_SetString(PyExc_Exception, "playSound: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that the sound exists
    if (!sApp->mResources->soundExists(index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to reference sound.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // set the sound parameters
    SoundInstance* sound = sApp->mSoundManager->GetSoundInstance(index);
    if (sound) {
        sound->SetVolume(volume);
        sound->SetPan(int( panning * 1000.0f));
        sound->AdjustPitch(pitchAdjust);

        // play the sound
        sound->Play(false, true); // Play sound. Always auto-release the instance.
    } else {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to reference sound.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pReadReg
//--------------------------------------------------

PyObject* PycapApp::pReadReg(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key)) {
        PyErr_SetString(PyExc_Exception, "readReg: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // attempt to read the string
    std::string string;
    if (sApp->RegistryReadString(key, &string)) {
        // return string from registry
        return Py_BuildValue("s", string.c_str());
    }

    Py_INCREF(Py_None);
    return Py_None;

}

//--------------------------------------------------
// pWriteReg
//--------------------------------------------------

PyObject* PycapApp::pWriteReg(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* key;
    char* string;
    if (!PyArg_ParseTuple(args, "ss", &key, &string)) {
        PyErr_SetString(PyExc_Exception, "writeReg: failed to parse arguments");
        PyErr_Print();
        return Py_BuildValue("i", 0);
    }

    // attempt to write the string
    // return whether or not we succeeded ('tho I'll probably just ignore it most of the time)
    if (sApp->RegistryWriteString(key, string)) {
        // success
        return Py_BuildValue("i", 1);
    } else {
        // failure
        return Py_BuildValue("i", 0);
    }
}

//--------------------------------------------------
// pPlayTune
//--------------------------------------------------

PyObject* PycapApp::pPlayTune(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    int repeatCount = 0; //do not loop
    if (!PyArg_ParseTuple(args, "i|i", &i, &repeatCount)) {
        PyErr_SetString(PyExc_Exception, "playTune: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    int index = sApp->mResources->getTune(i);
    if (index == -1) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to reference tune.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    sApp->mMusicInterface->PlayMusic(i, 0, repeatCount == 0);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pStopTune
//--------------------------------------------------

PyObject* PycapApp::pStopTune(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i = -1;
    if (!PyArg_ParseTuple(args, "|i", &i)) {
        PyErr_SetString(PyExc_Exception, "stopTune: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    int index = sApp->mResources->getTune(i);
    if (index == -1) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to reference tune.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    sApp->mMusicInterface->StopMusic(i);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetVolume
//--------------------------------------------------

PyObject* PycapApp::pSetVolume(PyObject* self, PyObject* args)
{
    // parse the arguments
    float vol;

    if (!PyArg_ParseTuple(args, "f", &vol)) {
        PyErr_SetString(PyExc_Exception, "setVolume: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    sApp->mMusicInterface->SetVolume((double) vol);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetTuneVolume
//--------------------------------------------------

PyObject* PycapApp::pSetTuneVolume(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    float vol;

    if (!PyArg_ParseTuple(args, "if", &index, &vol)) {
        PyErr_SetString(PyExc_Exception, "setTuneVolume: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    sApp->mMusicInterface->SetSongVolume(index, (double) vol);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetClipRect
//--------------------------------------------------

PyObject* PycapApp::pSetClipRect(PyObject* self, PyObject* args)
{
    // parse the arguments
    int x, y, w, h;
    if (!PyArg_ParseTuple(args, "iiii", &x, &y, &w, &h)) {
        PyErr_SetString(PyExc_Exception, "setClipRect: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "setClipRect: Not currently drawing");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }
    // set the clip region
    graphics->SetClipRect(x, y, w, h);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pClearClipRect
//--------------------------------------------------

PyObject* PycapApp::pClearClipRect(PyObject* self, PyObject* args)
{
    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "ClearClipRect() failed: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // clear the clip region
    graphics->ClearClipRect();

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetTranslation
//--------------------------------------------------

PyObject* PycapApp::pSetTranslation(PyObject* self, PyObject* args)
{
    // parse the arguments
    int x, y;
    if (!PyArg_ParseTuple(args, "ii", &x, &y)) {
        PyErr_SetString(PyExc_Exception, "setTranslation: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "setTranslation: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }
    // set the translation
    graphics->mTransX = x;
    graphics->mTransY = y;

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSetFullscreen
//--------------------------------------------------

PyObject* PycapApp::pSetFullscreen(PyObject* self, PyObject* args)
{
    // parse the arguments
    int fullscreen;
    if (!PyArg_ParseTuple(args, "i", &fullscreen)) {
        PyErr_SetString(PyExc_Exception, "setFullScreen: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // Super
    sApp->SwitchScreenMode(!fullscreen, sApp->Is3DAccelerated());

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pGetFullscreen
//--------------------------------------------------

PyObject* PycapApp::pGetFullscreen(PyObject* self, PyObject* args)
{
    if (sApp->mIsWindowed) {
        return Py_BuildValue("i", 0);
    } else {
        return Py_BuildValue("i", 1);
    }
}

//--------------------------------------------------
// pAllowAllAccess
//--------------------------------------------------

PyObject* PycapApp::pAllowAllAccess(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName)) {
        PyErr_SetString(PyExc_Exception, "allowAllAccess: failed to parse arguments");
        PyErr_Print();
        return Py_BuildValue("i", 0);
    }

    // attempt to unlock the file
    // return whether or not we succeeded ('tho I'll probably just ignore it most of the time)
    if (AllowAllAccess(fileName)) {
        // success
        return Py_BuildValue("i", 1);
    } else {
        // failure
        return Py_BuildValue("i", 0);
    }
}

PyObject* PycapApp::pGetKeyCode(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* key;
    if (!PyArg_ParseTuple(args, "s", &key)) {
        PyErr_SetString(PyExc_Exception, "getKeyCode: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    KeyCode k = GetKeyCodeFromName(key);

    // success
    return Py_BuildValue("i", k);
}

//--------------------------------------------------
// pGetIs3DAccelerated
//--------------------------------------------------

PyObject* PycapApp::pGetIs3DAccelerated(PyObject* self, PyObject* args)
{
    if (sApp->Is3DAccelerated()) {
        return Py_BuildValue("i", 1);
    } else {
        return Py_BuildValue("i", 0);
    }
}

//--------------------------------------------------
// pIsKeyDown
//--------------------------------------------------

PyObject* PycapApp::pIsKeyDown(PyObject* self, PyObject* args)
{
    // parse the arguments
    int keycode;
    if (!PyArg_ParseTuple(args, "i", &keycode)) {
        PyErr_SetString(PyExc_Exception, "isKeyDown: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    if (gSexyAppBase->mWidgetManager->GetKeyDown((KeyCode)keycode))
        return Py_BuildValue("i", 1);
    return Py_BuildValue("i", 0);
}

//--------------------------------------------------
// pDetectUserLanguage
//--------------------------------------------------

PyObject* PycapApp::pGetUserLanguage(PyObject* self, PyObject* args)
{
    std::string st = gSexyAppBase->GetUserLanguage();

    // convert user language to a python string & return it
    return Py_BuildValue("s", st.c_str());
}

//--------------------------------------------------
// pGetAppDataFolder
//--------------------------------------------------

PyObject* PycapApp::pGetAppDataFolder(PyObject* self, PyObject* args)
{
    // get the folder string
    std::string string = gSexyAppBase->GetAppDataFolder();
    if (string.empty()) {
        PyErr_SetString(PyExc_Exception, "AppDataFolder not set");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // convert folder name to a python string & return it
    return Py_BuildValue("s", string.c_str());
}

//--------------------------------------------------
// pGetAppResourceFolder
//--------------------------------------------------

PyObject* PycapApp::pGetAppResourceFolder(PyObject* self, PyObject* args)
{
    // get the folder string
    std::string string = gSexyAppBase->GetAppResourceFolder();

    // convert foler name to a python string & return it
    return Py_BuildValue("s", string.c_str());
}

//--------------------------------------------------
// pDrawImageRotScaled
//--------------------------------------------------

PyObject* PycapApp::pDrawImageRotScaled(PyObject* self, PyObject* args)
{
    // parse the arguments
    int i;
    float x, y, r, scaleX, scaleY;
    if (!PyArg_ParseTuple(args, "ifffff", &i, &x, &y, &r, &scaleX, &scaleY)) {
        PyErr_SetString(PyExc_Exception, "drawImageRotScaled: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawImageRotScaled: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get the image
    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to reference image.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    if (sApp->Is3DAccelerated()) {

        Sexy::Transform t;

        t.Scale(scaleX, scaleY);
        t.RotateRad(r);
        t.Translate(image->GetWidth() / 2, image->GetHeight() / 2);

        // perform the blit
        graphics->DrawImageTransform(image, t, x, y);

    }

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pSet3DAccelerated
//--------------------------------------------------

PyObject* PycapApp::pSet3DAccelerated(PyObject* self, PyObject* args)
{
    int accelerate;
    if (!PyArg_ParseTuple(args, "i", &accelerate)) {
        PyErr_SetString(PyExc_Exception, "set3DAccelerated: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    sApp->SwitchScreenMode(sApp->mIsWindowed, accelerate);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//functions by Tony Oakden

//--------------------------------------------------
// pDrawQuad
//--------------------------------------------------

PyObject* PycapApp::pDrawQuad(PyObject* self, PyObject* args)
{
    // parse the arguments
    int x1, y1, x2, y2, x3, y3, x4, y4;
    if (!PyArg_ParseTuple(args, "iiiiiiii", &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4)) {
        PyErr_SetString(PyExc_Exception, "drawQuad: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawQuad failed: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    Point qaudPoints[4];
    qaudPoints[0] = Point(x1, y1);
    qaudPoints[1] = Point(x2, y2);
    qaudPoints[2] = Point(x3, y3);
    qaudPoints[3] = Point(x4, y4);
    graphics->PolyFill(qaudPoints, 4);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}


//--------------------------------------------------
// pDrawTri
//--------------------------------------------------

PyObject* PycapApp::pDrawTri(PyObject* self, PyObject* args)
{
    // parse the arguments
    int x1, y1, x2, y2, x3, y3;
    if (!PyArg_ParseTuple(args, "iiiiii", &x1, &y1, &x2, &y2, &x3, &y3)) {
        PyErr_SetString(PyExc_Exception, "drawTri: failed to parse arguments");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawTri() failed: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    Point triPoints[3];
    triPoints[0] = Point(x1, y1);
    triPoints[1] = Point(x2, y2);
    triPoints[2] = Point(x3, y3);
    graphics->PolyFill(triPoints, 3);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pDrawQuadTextured
//--------------------------------------------------

PyObject* PycapApp::pDrawQuadTextured(PyObject* self, PyObject* args)
{
    // parse the arguments
    float x1, y1, x2, y2, x3, y3, x4, y4;
    int i;
    float u1, v1, u2, v2, u3, v3, u4, v4;
    if (!PyArg_ParseTuple(args, "iffffffff", &i, &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4))
        return NULL;

    // check that we're currently drawing
    Graphics* graphics = sApp->mBoard->getGraphics();
    if (!graphics) {
        // fail, 'cos we can only do this while drawing
        PyErr_SetString(PyExc_Exception, "drawQuadTextured() failed: Not currently drawing!");
        PyErr_Print();
        Py_INCREF(Py_None);
        return Py_None;
    }

    Image* image = sApp->mResources->getImage(i);
    if (!image) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Failed to reference image.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }
    u1 = 0;
    v1 = 0;
    u2 = float(0.99);
    v2 = 0;
    u3 = float(0.99);
    v3 = float(0.99);
    u4 = 0;
    v4 = float(0.99);
    TriVertex vertex1 = TriVertex(x1, y1, u1, v1);
    TriVertex vertex2 = TriVertex(x2, y2, u2, v2);
    TriVertex vertex3 = TriVertex(x3, y3, u3, v3);
    TriVertex vertex4 = TriVertex(x4, y4, u4, v4);
    graphics->DrawTriangleTex(image, vertex1, vertex2, vertex3);
    graphics->DrawTriangleTex(image, vertex1, vertex4, vertex3);

    // return, 'cos we're done
    Py_INCREF(Py_None);
    return Py_None;
}
