//--------------------------------------------------
// PycapApp
//
// Pycap application object
//
// Original bindings by Jarrad "Farbs" Woods
//--------------------------------------------------

// includes
#include "PycapApp.h"
#include "PycapBoard.h"

#include "WidgetManager.h"
#include "Graphics.h"
#include "Font.h"
#include "SoundManager.h"
#include "SoundInstance.h"
#include "MusicInterface.h"
#include "KeyCodes.h"

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

	// own members
	sApp				= this;
	mBoard				= NULL;
	mResources			= NULL;
	mResFailed			= false;
	//-------------------
}

//--------------------------------------------------
// ~PycapApp
//--------------------------------------------------
PycapApp::~PycapApp()
{
  // clean up board if necessary
  if( mBoard != NULL )
    {
      mWidgetManager->RemoveWidget( mBoard );
      delete mBoard;
    }

  // clean up resources object
  if( mResources != NULL )
    {
      delete mResources;
    }

  // clean up app pointer if it's pointing to this
  if( sApp == this )
    {
      sApp = NULL;
    }

  // clean up python
  Py_DECREF(pModule);	// drop the module
  Py_Finalize();		// shut down the interpreter
}

//--------------------------------------------------
// Init
//--------------------------------------------------
void PycapApp::Init()
{
  // Set up python
  Py_Initialize();

  PyRun_SimpleString("import sys");
  
  if (GetAppResourceFolder() != "") {
    PyRun_SimpleString(("sys.path.append(\"" + GetAppResourceFolder() +"\")").c_str());
  } 
  else
    PyRun_SimpleString("sys.path.append(\".\")");

  // Set up Pycap module
  static PyMethodDef resMethods[]	= {
    {"markDirty", pMarkDirty, METH_VARARGS, "markDirty()\nMark the screen dirty & call a refresh."},
    {"fillRect", pFillRect, METH_VARARGS, "fillRect( x, y, width, height )\nFill a specified rect with the current colour."},
    {"setColour", pSetColour, METH_VARARGS, "setColour( red, green, blue, alpha )\nSet the draw colour. Use a value between 0 and 255 for each component."},
    {"setFont", pSetFont, METH_VARARGS, "setFont( font )\nSet the active font."},
    {"setColourize", pSetColourize, METH_VARARGS, "setColourize( on )\nEnable/Disable colourized drawing."},
    {"drawImage", pDrawImage, METH_VARARGS, "drawImage( image, x, y )\nDraw an image resource at pixel coords."},
    {"drawImageF", pDrawImageF, METH_VARARGS, "drawImageF( image, fx, fy )\nDraw an image resource at float coords."},
    {"drawImageRot", pDrawImageRot, METH_VARARGS, "drawImageRot( image, x, y, angle )\nDraw an image resource at pixel coords rotated by a given angle."},
    {"drawImageRotF", pDrawImageRotF, METH_VARARGS, "drawImageRot( image, x, y, angle )\nDraw an image resource at float coords rotated by a given angle."},
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
    {"setTuneVolume", pSetTuneVolume, METH_VARARGS, "setTuneVolume( volume )\nChange the global volume for all midi"},
    {"setClipRect", pSetClipRect, METH_VARARGS, "setClipRect( x, y, width, height )\nSet the clipping rectangle."},
    {"clearClipRect", pClearClipRect, METH_VARARGS, "clearClipRect()\nClear the clipping rectangle."},
    {"setTranslation", pSetTranslation, METH_VARARGS, "setTranslation( x, y )\nSet the translation applied to all draw calls."},
    {"setFullscreen", pSetFullscreen, METH_VARARGS, "setFullscreen( fullscreen )\nSet whether the app should be in fullscreen or windowed mode."},
    {"getFullscreen", pGetFullscreen, METH_VARARGS, "getFullscreen()\nGet whether the app is in fullscreen or windowed mode."},
    {"getKeyCode", pGetKeyCode, METH_VARARGS, "getKeyCode\n."},
    {"allowAllAccess", pAllowAllAccess, METH_VARARGS, "allowAllAccess( fileName )\nTell the OS that all users can view and modify a file. Required for Vista."},
    {"getAppDataFolder", pGetAppDataFolder, METH_VARARGS, "getAppDataFolder()\nGet the folder that game data should be saved to. Required for Vista."},
    {"getAppResourceFolder", pGetAppResourceFolder, METH_VARARGS, "getAppResourceFolder()\nGet the folder where the game resources are stored. Required for GNU/Linux."},
    {NULL, NULL, 0, NULL}
  };
  Py_InitModule("Pycap", resMethods);
  // general error location warning
  if (PyErr_Occurred())
    {
      PyErr_Print();
      //Popup( StrFormat( "Some kind of python error occurred in PycapApp(), while importing Pycap module." ) );
      return;
    }

  // Open game module
  PyObject *pName;
  pName = PyString_FromString( "game" );
  if (pName == NULL) {
    return;
  }

  pModule = PyImport_Import(pName);
  if (pModule == NULL)
    {
      //Popup( StrFormat( "Failed to load game.py" ) );
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
  iniDict = PyDict_GetItemString( pDict, "appIni" );
  if ( iniDict && PyDict_Check( iniDict ) )
    {
      PyObject *inObject;

      inObject = PyDict_GetItemString( iniDict, "mCompanyName" );
      if ( inObject && PyString_Check( inObject ) )
        {
          mCompanyName = PyString_AsString( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mCompanyName correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mFullCompanyName" );
      if ( inObject && PyString_Check( inObject ) )
        {
          mFullCompanyName = PyString_AsString( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mFullCompanyName correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mProdName" );
      if ( inObject && PyString_Check( inObject ) )
        {
          mProdName = PyString_AsString( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mProdName correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mProductVersion" );
      if ( inObject && PyString_Check( inObject ) )
        {
          mProductVersion = PyString_AsString( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mProductVersion correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mTitle" );
      if ( inObject && PyString_Check( inObject ) )
        {
          mTitle = PyString_AsString( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mTitle correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mRegKey" );
      if ( inObject && PyString_Check( inObject ) )
        {
          mRegKey = PyString_AsString( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mRegKey correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mWidth" );
      if ( inObject && PyInt_Check( inObject ) )
        {
          mWidth = PyInt_AsLong( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mWidth correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mHeight" );
      if ( inObject && PyInt_Check( inObject ) )
        {
          mHeight = PyInt_AsLong( inObject );
        }
      else
        {
          //Popup( "appIni doesn't specify mHeight correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mAutoEnable3D" );
      if ( inObject && PyInt_Check( inObject ) )
        {
          mAutoEnable3D = PyInt_AsLong( inObject ) == 1;
        }
      else
        {
          //Popup( "appIni doesn't specify mAutoEnable3D correctly" );
          PyErr_Print();
          return;
        }

      inObject = PyDict_GetItemString( iniDict, "mVSyncUpdates" );
      if ( inObject && PyInt_Check( inObject ) )
        {
          mVSyncUpdates = PyInt_AsLong( inObject ) == 1;
        }
      else
        {
          //Popup( "appIni doesn't specify mVSyncUpdates correctly" );
          PyErr_Print();
          return;
        }
    }
  else
    {
      //Popup( "appIni object is missing or not a dict" );
      PyErr_Print();
      return;
    }

  if (mRegKey.empty()) {
    mRegKey = "TuxCap";
  }

  // call parent
  SexyAppBase::Init();

  PyRun_SimpleString(("sys.path.append(\"" + GetAppDataFolder() + "\")").c_str());

  // Redirect stdout and stderr to files (since we can't seem to use console output)
  PyRun_SimpleString( ( "sys.stdout = open( \"" + GetAppDataFolder() + "out.txt\", 'w' )" ).c_str() );
  PyRun_SimpleString( ( "sys.stderr = open( \"" + GetAppDataFolder() + "err.txt\", 'w' )" ).c_str() );

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
  if( mResFailed )
    {
      // Nothing much happens if we return before adding the board... just a black screen
      // Error message widget should be added here
      //Popup("The game did not load properly. Sorry, but it's not going to work.");
      return;
    }

  // create the initial board object
  PycapBoard* newBoard = new PycapBoard();

  // remove current board if appropriate
  if( mBoard != NULL )
    {
      mWidgetManager->RemoveWidget( mBoard );
    }

  // store link
  mBoard = newBoard;

  // resize the board to the application size
  newBoard->Resize( 0, 0, PycapApp::sApp->mWidth, PycapApp::sApp->mHeight );

  // add the board widget
  mWidgetManager->AddWidget( newBoard );

  // give the board focus
  mWidgetManager->SetFocus( newBoard );
}

//--------------------------------------------------
// GotFocus
//--------------------------------------------------
void PycapApp::GotFocus()
{
  PyObject* pGFFunc = PyDict_GetItemString( PycapApp::sApp->pDict, "gotFocus" );

  if ( pGFFunc && PyCallable_Check( pGFFunc ) )
    {
      PyObject_CallObject( pGFFunc, NULL );
    }
}

//--------------------------------------------------
// LostFocus
//--------------------------------------------------
void PycapApp::LostFocus()
{
  PyObject* pLFFunc = PyDict_GetItemString( PycapApp::sApp->pDict, "lostFocus" );

  if ( pLFFunc && PyCallable_Check( pLFFunc ) )
    {
      PyObject_CallObject( pLFFunc, NULL );
    }
}

//--------------------------------------------------
// SwitchScreenMode
//--------------------------------------------------
void PycapApp::SwitchScreenMode( bool wantWindowed, bool is3d )
{
  // Super
  SexyAppBase::SwitchScreenMode( wantWindowed, is3d );

  // attempt to call python fullscreen or windowed notifier
  // failed attempts still notify, but notify correctly
  if ( mIsWindowed )
    {
      // windowed
      PyObject* pWFunc = PyDict_GetItemString( PycapApp::sApp->pDict, "onWindowed" );
      if ( pWFunc && PyCallable_Check( pWFunc ) )
        {
          PyObject_CallObject( pWFunc, NULL );
        }
    }
  else
    {
      // fullscreen
      PyObject* pFSFunc = PyDict_GetItemString( PycapApp::sApp->pDict, "onFullscreen" );
      if ( pFSFunc && PyCallable_Check( pFSFunc ) )
        {
          PyObject_CallObject( pFSFunc, NULL );
        }
    }
}


//--------------------------------------------------
// pMarkDirty
//--------------------------------------------------
PyObject* PycapApp::pMarkDirty( PyObject* self, PyObject* args )
{
  // parse the arguments
  //if( !PyArg_ParseTuple( args, NULL ) )
  //   return NULL;

  // mark the board as dirty
  sApp->mBoard->MarkDirty();

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pSetColour
//--------------------------------------------------
PyObject* PycapApp::pSetColour( PyObject* self, PyObject* args )
{
  // parse the arguments
  int r, g, b, a;
  if( !PyArg_ParseTuple( args, "iiii", &r, &g, &b, &a ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "SetColour() failed: Not currently drawing!" ) );
      return NULL;
    }

  // create new colour object, set colour
  graphics->SetColor( Color( r, g, b, a ) );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pSetFont
//--------------------------------------------------
PyObject* PycapApp::pSetFont( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i;
  if( !PyArg_ParseTuple( args, "i", &i ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "SetFont() failed: Not currently drawing!" ) );
      return NULL;
    }

  // get the font
  Font* font = sApp->mResources->getFont( i );
  if( !font )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference font." );

      // exit, returning None/NULL
      return NULL;
    }

  // set the active font
  graphics->SetFont( font );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pSetColourize
//--------------------------------------------------
PyObject* PycapApp::pSetColourize( PyObject* self, PyObject* args )
{
  // parse the arguments
  int colourize;
  if( !PyArg_ParseTuple( args, "i", &colourize ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "SetColourize() failed: Not currently drawing!" ) );
      return NULL;
    }

  // create new colour object, set colour
  graphics->SetColorizeImages( colourize != 0 );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pFillRect
//--------------------------------------------------
PyObject* PycapApp::pFillRect( PyObject* self, PyObject* args )
{
  // parse the arguments
  int x, y, w, h;
  if( !PyArg_ParseTuple( args, "iiii", &x, &y, &w, &h ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "FillRect() failed: Not currently drawing!" ) );
      return NULL;
    }

  // create new colour object, set colour
  graphics->FillRect( x, y, w, h );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawImage
//--------------------------------------------------
PyObject* PycapApp::pDrawImage( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i, x, y;
  if( !PyArg_ParseTuple( args, "iii", &i, &x, &y ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawImage() failed: Not currently drawing!" ) );
      return NULL;
    }

  // get the image
  Image* image = sApp->mResources->getImage( i );
  if( !image )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference image." );

      // exit, returning None/NULL
      return NULL;
    }

  // perform the blit
  graphics->DrawImage( image, x, y );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawImageF
//--------------------------------------------------
PyObject* PycapApp::pDrawImageF( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i;
  float x, y;
  if( !PyArg_ParseTuple( args, "iff", &i, &x, &y ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawImageF() failed: Not currently drawing!" ) );
      return NULL;
    }

  // get the image
  Image* image = sApp->mResources->getImage( i );
  if( !image )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference image." );

      // exit, returning None/NULL
      return NULL;
    }

  // perform the blit
  graphics->DrawImageF( image, x, y );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawImageRot
//--------------------------------------------------
PyObject* PycapApp::pDrawImageRot( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i;
  float x, y, r;
  if( !PyArg_ParseTuple( args, "ifff", &i, &x, &y, &r ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawImageRot() failed: Not currently drawing!" ) );
      return NULL;
    }

  // get the image
  Image* image = sApp->mResources->getImage( i );
  if( !image )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference image." );

      // exit, returning None/NULL
      return NULL;
    }

  // perform the blit
  graphics->DrawImageRotated( image, (int)x, (int)y, r );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawImageRotF
//--------------------------------------------------
PyObject* PycapApp::pDrawImageRotF( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i;
  float x, y, r;
  if( !PyArg_ParseTuple( args, "ifff", &i, &x, &y, &r ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawImageRotF() failed: Not currently drawing!" ) );
      return NULL;
    }

  // get the image
  Image* image = sApp->mResources->getImage( i );
  if( !image )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference image." );

      // exit, returning None/NULL
      return NULL;
    }

  // perform the blit
  graphics->DrawImageRotatedF( image, x, y, r );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawImageScaled
//--------------------------------------------------
PyObject* PycapApp::pDrawImageScaled( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i;
  float x, y, w, h;
  if( !PyArg_ParseTuple( args, "iffff", &i, &x, &y, &w, &h ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawImageScaled() failed: Not currently drawing!" ) );
      return NULL;
    }

  // get the image
  Image* image = sApp->mResources->getImage( i );
  if( !image )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference image." );

      // exit, returning None/NULL
      return NULL;
    }

  // perform the blit
  graphics->DrawImage( image, (int)x, (int)y, (int)w, (int)h );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawString
//--------------------------------------------------
PyObject* PycapApp::pDrawString( PyObject* self, PyObject* args )
{
  // parse the arguments
  char* string;
  float x, y;
  if( !PyArg_ParseTuple( args, "sff", &string, &x, &y ) )
    return NULL;

  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawString() failed: Not currently drawing!" ) );
      return NULL;
    }

  // perform the blit
  graphics->DrawString( string, (int)x, (int)y );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pShowMouse
//--------------------------------------------------
PyObject* PycapApp::pShowMouse( PyObject* self, PyObject* args )
{
  // parse the arguments
  int show;
  if( !PyArg_ParseTuple( args, "i", &show ) )
    return NULL;

  // test argument
  if( show )
    {
      sApp->SetCursor( CURSOR_POINTER );
    }
  else
    {
      sApp->SetCursor( CURSOR_NONE );
    }

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawmodeNormal
//--------------------------------------------------
PyObject* PycapApp::pDrawmodeNormal( PyObject* self, PyObject* args )
{
  // parse the arguments
  //if( !PyArg_ParseTuple( args, NULL ) )
  //   return NULL;
	
  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawmodeNormal() failed: Not currently drawing!" ) );
      return NULL;
    }

  // set the draw mode
  graphics->SetDrawMode( Graphics::DRAWMODE_NORMAL );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pDrawmodeAdd
//--------------------------------------------------
PyObject* PycapApp::pDrawmodeAdd( PyObject* self, PyObject* args )
{
  // parse the arguments
  //if( !PyArg_ParseTuple( args, NULL ) )
  //   return NULL;
	
  // check that we're currently drawing
  Graphics* graphics = sApp->mBoard->getGraphics();
  if( !graphics )
    {
      // fail, 'cos we can only do this while drawing
      //sApp->Popup( StrFormat( "DrawmodeAdd() failed: Not currently drawing!" ) );
      return NULL;
    }

  // set the draw mode
  graphics->SetDrawMode( Graphics::DRAWMODE_ADDITIVE );

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pPlaySound
//--------------------------------------------------
PyObject* PycapApp::pPlaySound( PyObject* self, PyObject* args )
{
  // parse the arguments
  // required
  int index;
  // optional
  float	volume = 1.0f;
  float	panning = 0.0f;
  float	pitchAdjust = 0.0f;
  if( !PyArg_ParseTuple( args, "i|fff", &index, &volume, &panning, &pitchAdjust ) )
    return NULL;

  // check that the sound exists
  if( !sApp->mResources->soundExists( index ) )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference sound." );

      // exit, returning None/NULL
      return NULL;
    }

  // set the sound parameters
  SoundInstance* sound = sApp->mSoundManager->GetSoundInstance( index );
  if( sound )
    {
      sound->SetVolume( volume );
      sound->SetPan( int( panning * 1000.0f ) );
      sound->AdjustPitch( pitchAdjust );

      // play the sound
      sound->Play( false, true );	// Play sound. Always auto-release the instance.
    }

  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pReadReg
//--------------------------------------------------
PyObject* PycapApp::pReadReg( PyObject* self, PyObject* args )
{
  // parse the arguments
  char* key;
  if( !PyArg_ParseTuple( args, "s", &key ) )
    return NULL;

  // attempt to read the string
  std::string string;
  if( sApp->RegistryReadString( key, &string ) )
    {
      // return string from registry
      return Py_BuildValue( "s", string.c_str() );
    }
  else
    {
      Py_INCREF( Py_None );
      return Py_None;
    }
}

//--------------------------------------------------
// pWriteReg
//--------------------------------------------------
PyObject* PycapApp::pWriteReg( PyObject* self, PyObject* args )
{
  // parse the arguments
  char* key;
  char* string;
  if( !PyArg_ParseTuple( args, "ss", &key, &string ) )
    return NULL;

  // attempt to write the string
  // return whether or not we succeeded ('tho I'll probably just ignore it most of the time)
  if( sApp->RegistryWriteString( key, string ) )
    {
      // success
      return Py_BuildValue( "i", 1 );
    }
  else
    {
      // failure
      return Py_BuildValue( "i", 0 );
    }
}

//--------------------------------------------------
// pPlayTune
//--------------------------------------------------
PyObject* PycapApp::pPlayTune( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i;
  int repeatCount = -1; //loop eternally
  if( !PyArg_ParseTuple( args, "i|i", &i, &repeatCount ) )
    return Py_None;

  int index = sApp->mResources->getTune( i );
  if( index == -1 )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference tune." );

      // exit, returning None/NULL
      return Py_None;
    }

  sApp->mMusicInterface->PlayMusic( i,  0, repeatCount != -1);
  
  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pStopTune
//--------------------------------------------------
PyObject* PycapApp::pStopTune( PyObject* self, PyObject* args )
{
  // parse the arguments
  int i = -1;
  if( !PyArg_ParseTuple( args, "|i", &i ) )
    return Py_None;

  int index = sApp->mResources->getTune( i );
  if( index == -1 )
    {
      // throw an exception
      PyErr_SetString( PyExc_IOError, "Failed to reference tune." );

      // exit, returning None/NULL
      return Py_None;
    }

  sApp->mMusicInterface->StopMusic( i );
  
  // return, 'cos we're done
  Py_INCREF( Py_None );
  return Py_None;
}

//--------------------------------------------------
// pSetTuneVolume
//--------------------------------------------------
PyObject* PycapApp::pSetTuneVolume( PyObject* self, PyObject* args )
{
	// parse the arguments
	long l;
	if( !PyArg_ParseTuple( args, "l", &l ) )
        return Py_None;
	l = -1000000;

        sApp->mMusicInterface->SetVolume( (double)l ); //FIXME should be the tune volume, probably not the overall volume, so we need a tune index as a parameter

	// return, 'cos we're done
	Py_INCREF( Py_None );
 	return Py_None;
}

//--------------------------------------------------
// pSetClipRect
//--------------------------------------------------
PyObject* PycapApp::pSetClipRect( PyObject* self, PyObject* args )
{
	// parse the arguments
	int x, y, w, h;
	if( !PyArg_ParseTuple( args, "iiii", &x, &y, &w, &h ) )
        return NULL;

	// check that we're currently drawing
	Graphics* graphics = sApp->mBoard->getGraphics();
	if( !graphics )
	{
		// fail, 'cos we can only do this while drawing
		//sApp->//Popup( StrFormat( "SetClipRect() failed: Not currently drawing!" ) );
		return NULL;
	}
	// set the clip region
	graphics->SetClipRect( x,y,w,h );

	// return, 'cos we're done
	Py_INCREF( Py_None );
 	return Py_None;
}

//--------------------------------------------------
// pClearClipRect
//--------------------------------------------------
PyObject* PycapApp::pClearClipRect( PyObject* self, PyObject* args )
{
	// check that we're currently drawing
	Graphics* graphics = sApp->mBoard->getGraphics();
	if( !graphics )
	{
		// fail, 'cos we can only do this while drawing
		//sApp->//Popup( StrFormat( "ClearClipRect() failed: Not currently drawing!" ) );
		return NULL;
	}
	// clear the clip region
	graphics->ClearClipRect();

	// return, 'cos we're done
	Py_INCREF( Py_None );
 	return Py_None;
}

//--------------------------------------------------
// pSetTranslation
//--------------------------------------------------
PyObject* PycapApp::pSetTranslation( PyObject* self, PyObject* args )
{
	// parse the arguments
	int x, y;
	if( !PyArg_ParseTuple( args, "ii", &x, &y ) )
        return NULL;

	// check that we're currently drawing
	Graphics* graphics = sApp->mBoard->getGraphics();
	if( !graphics )
	{
		// fail, 'cos we can only do this while drawing
		//sApp->Popup( StrFormat( "SetTranslation() failed: Not currently drawing!" ) );
		return NULL;
	}
	// set the translation
	graphics->mTransX = x;
	graphics->mTransY = y;

	// return, 'cos we're done
	Py_INCREF( Py_None );
 	return Py_None;
}

//--------------------------------------------------
// pSetFullscreen
//--------------------------------------------------
PyObject* PycapApp::pSetFullscreen( PyObject* self, PyObject* args )
{
	// parse the arguments
	int fullscreen;
	if( !PyArg_ParseTuple( args, "i", &fullscreen ) )
        return NULL;

	// set fullscreen to true
	sApp->SwitchScreenMode( !fullscreen, sApp->Is3DAccelerated() );

	// return, 'cos we're done
	Py_INCREF( Py_None );
 	return Py_None;
}

//--------------------------------------------------
// pGetFullscreen
//--------------------------------------------------
PyObject* PycapApp::pGetFullscreen( PyObject* self, PyObject* args )
{
	if( sApp->mIsWindowed )
	{
		return Py_BuildValue( "i", 0 );
	}
	else
	{
		return Py_BuildValue( "i", 1 );
	}
}

//--------------------------------------------------
// pAllowAllAccess
//--------------------------------------------------
PyObject* PycapApp::pAllowAllAccess( PyObject* self, PyObject* args )
{
	// parse the arguments
	char* fileName;
	if( !PyArg_ParseTuple( args, "s", &fileName ) )
          return NULL;

	// attempt to unlock the file
	// return whether or not we succeeded ('tho I'll probably just ignore it most of the time)
	if( AllowAllAccess( fileName ) )
	{
		// success
	    return Py_BuildValue( "i", 1 );
	}
	else
	{
		// failure
	    return Py_BuildValue( "i", 0 );
	}
}

PyObject* PycapApp::pGetKeyCode( PyObject* self, PyObject* args )
{
	// parse the arguments
	char* key;
	if( !PyArg_ParseTuple( args, "s", &key ) )
          return NULL;

         KeyCode k = GetKeyCodeFromName(key);

		// success
         return Py_BuildValue( "i", k );
}

//--------------------------------------------------
// pGetAppDataFolder
//--------------------------------------------------
PyObject* PycapApp::pGetAppDataFolder( PyObject* self, PyObject* args )
{
	// get the folder string
	std::string string = GetAppDataFolder();

	// convert foler name to a python string & return it
	return Py_BuildValue( "s", string.c_str() );
}

//--------------------------------------------------
// pGetAppResourceFolder
//--------------------------------------------------
PyObject* PycapApp::pGetAppResourceFolder( PyObject* self, PyObject* args )
{
	// get the folder string
	std::string string = GetAppResourceFolder();

	// convert foler name to a python string & return it
	return Py_BuildValue( "s", string.c_str() );
}
