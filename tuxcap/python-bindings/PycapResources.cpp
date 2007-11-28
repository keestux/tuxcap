//--------------------------------------------------
// PycapResources
//
// Pycap application resources
// Handles access to images, sounds etc by python
//
//--------------------------------------------------

// includes
#include "PycapResources.h"
#include "PycapApp.h"

#include "DDImage.h"
#include "ImageFont.h"
//#include "SysFont.h"
#include "SoundManager.h"

#ifndef INITGUID
#define INITGUID
#endif


#include <Python.h>

// namespace
using namespace Sexy;

// static data definition
PycapResources* PycapResources::sRes = NULL;

// functions

//--------------------------------------------------
// PycapResources
//--------------------------------------------------
PycapResources::PycapResources()
{
	//--------------------------------
	// Initialize non-resource members
	
	sRes		= this;
#if 0
	// create direct music loader
	CoInitialize( NULL );
    if( FAILED( CoCreateInstance(	CLSID_DirectMusicLoader,
									NULL,
									CLSCTX_INPROC, 
									IID_IDirectMusicLoader,
									(void**)&musicLoader ) ) )
    {
        musicLoader = NULL;
		//PycapApp::sApp->Popup( "PycapResources::PycapResources() failed to create music loader." );
    }
	// set search directory to application root
    HRESULT hr = musicLoader->SetSearchDirectory(GUID_DirectMusicAllTypes,
		L".", FALSE);
    if( FAILED( hr ) ) 
    {
		//PycapApp::sApp->Popup( "PycapResources::PycapResources() failed to set music loader search directory." );
    }
#endif	
	//--------------------------------

	//---------------------------
	// Set up PycapRes module
	static PyMethodDef resMethods[]	= {
		{"loadImage", pLoadImage, METH_VARARGS, "loadImage( fileName )\nLoad an image from file, and return its resource index."},
		{"imageWidth", pImageWidth, METH_VARARGS, "imageWidth( image )\nGet the width of an image resource."},
		{"imageHeight", pImageHeight, METH_VARARGS, "imageHeight( image )\nGet the height of an image resource."},
		{"unloadImage", pUnloadImage, METH_VARARGS, "unloadImage( image )\nUnload an image created by loadImage."},
		{"loadFont", pLoadFont, METH_VARARGS, "loadFont( fileName )\nLoad an image font from file, and return its resource index."},
		{"sysFont", pSysFont, METH_VARARGS, "sysFont( fontName )\nCreate an instance of a system font and return its resource index."},
		{"stringWidth", pStringWidth, METH_VARARGS, "stringWidth( string, font )\nGet the width of a string drawn using a given font."},
		{"fontAscent", pFontAscent, METH_VARARGS, "fontAscent( font )\nGet the ascent of a given font."},
		{"unloadFont", pUnloadFont, METH_VARARGS, "unloadFont( font )\nUnload a font created by loadFont or sysFont."},
		{"setFontScale", pSetFontScale, METH_VARARGS, "setFontScale( font, scale )\nSet the draw scale of an image font object."},
		{"loadSound", pLoadSound, METH_VARARGS, "loadSound( fileName )\nLoad a sound file, and return its resource index."},
		{"unloadSound", pUnloadSound, METH_VARARGS, "unloadSound( sound )\nUnload a sound file from its resource index."},
		{"loadTune", pLoadTune, METH_VARARGS, "loadTune( fileName )\nLoad a midi file, and return its resource index."},
		{"unloadTune", pUnloadTune, METH_VARARGS, "unloadTune( tune )\nUnload a midi file created by loadTune."},
		{NULL, NULL, 0, NULL}
	};
	Py_InitModule("PycapRes", resMethods);
	// general error location warning
	if (PyErr_Occurred())
	{
		PyErr_Print();
		//PycapApp::sApp->Popup( StrFormat( "Some kind of python error occurred in PycapResources(), while importing PycapRes." ) );
	}

	//--------------------------------


	//---------------------------
	// Load and process resources
	
	PyObject* pLoadFunc = PyDict_GetItemString( PycapApp::sApp->pDict, "loadBase" );

    if ( pLoadFunc && PyCallable_Check( pLoadFunc ) )
	{
        PyObject_CallObject( pLoadFunc, NULL );
    }
	// general error location warning
	if (PyErr_Occurred())
	{
		PyErr_Print();
		//PycapApp::sApp->Popup( StrFormat( "Some kind of python error occurred in PycapResources(), while running loadBase" ) );
	}

	//---------------------------
}

//--------------------------------------------------
// ~PycapResources
//--------------------------------------------------
PycapResources::~PycapResources()
{
	//------------------------------
	// Clean up non-resource members

	if( sRes == this )
	{
		sRes = NULL;
	}

#if 0
	if( musicLoader )
	{
		musicLoader->Release();
	}
#endif
	//------------------------------

	//-------------------
	// Clean up resources
	
	// images
	for( std::vector<Image*>::iterator iit = images.begin(); iit != images.end(); ++iit )
	{
		delete *iit;
	}
	images.clear();
	freeImages.clear();

	// fonts
	for( std::vector<Font*>::iterator fit = fonts.begin(); fit != fonts.end(); ++fit )
	{
		delete *fit;
	}
	fonts.clear();
	freeFonts.clear();

	// sounds
	PycapApp::sApp->mSoundManager->ReleaseSounds();
	sounds.clear();
	freeSounds.clear();

#if 0
	// music
	for( std::vector<IDirectMusicSegment*>::iterator tit = tunes.begin(); tit != tunes.end(); ++tit )
	{
		if( *tit )
		{
			(*tit)->Release();
		}
	}
#endif
	//-------------------
}

//--------------------------------------------------
// getImage
//--------------------------------------------------
Image* PycapResources::getImage( int index )
{
	// check bounds
	if( index >= sRes->images.size() )
	{
		// exit, returning None/NULL
		return NULL;
	}

	// return a pointer to the image object (or NULL if it's been unloaded)
	return images[index];
}


//--------------------------------------------------
// loadImage
//--------------------------------------------------
Image* PycapResources::loadImage( const std::string& fileName )
{
	Image* newImage = (DDImage*) PycapApp::sApp->GetImage( fileName );
	if( newImage == NULL )
	{
		PycapApp::sApp->resLoadFailed();
		//PycapApp::sApp->Popup( fileName + " could not be loaded." );
		return NULL;
	}

	// palletize
	( (DDImage*)newImage )->Palletize(); // attempt to palletize, don't worry if it fails

	// return new image
	return newImage;
}

//--------------------------------------------------
// getFont
//--------------------------------------------------
Font* PycapResources::getFont( int index )
{
	// check bounds
	if( index >= sRes->fonts.size() )
	{
		// exit, returning None/NULL
		return NULL;
	}

	// return a pointer to the font object (or NULL if it's been unloaded)
	return fonts[index];
}


//--------------------------------------------------
// loadFont
//--------------------------------------------------
Font* PycapResources::loadFont( const std::string& fileName )
{
	ImageFont* newFont = new ImageFont( PycapApp::sApp, fileName );
	if( !newFont->mFontData->mInitialized )
	{
		delete newFont;
		PycapApp::sApp->resLoadFailed();
		//PycapApp::sApp->Popup( fileName + " could not be loaded." );
		return NULL;
	}

	// return new font
	return newFont;
}

//--------------------------------------------------
// sysFont
//--------------------------------------------------
Font* PycapResources::sysFont(
						const std::string& faceName,
						int pointSize,
						int script,
						bool bold,
						bool italics,
						bool underline
						)
{
#if 0
	SysFont* newFont = new SysFont( PycapApp::sApp, faceName, pointSize, script, bold, italics, underline );
	if( !newFont->mHFont )
	{
		delete newFont;
		PycapApp::sApp->resLoadFailed();
		//PycapApp::sApp->Popup( faceName + " system font could not be created." );
		return NULL;
	}

	// return new font
	return newFont;
#else
        return NULL;
#endif
}

//--------------------------------------------------
// getSound
//--------------------------------------------------
bool PycapResources::soundExists( int index )
{
	// check bounds
	if( index >= sRes->sounds.size() )
	{
		// out of bounds, so sound doesn't exist
		return false;
	}

	// return whether the sound exists or not from our record
	return sounds[index];
}


//--------------------------------------------------
// loadSound
//--------------------------------------------------
bool PycapResources::loadSound( int id, const std::string& fileName )
{
	// attempt to load
	if( !PycapApp::sApp->mSoundManager->LoadSound( id, fileName ) )
	{
		// report error
		PycapApp::sApp->resLoadFailed();
		//PycapApp::sApp->Popup( fileName + " could not be loaded." );
		return false;
	}

	// success!
	return true;
}

//--------------------------------------------------
// getTune
//--------------------------------------------------
IDirectMusicSegment* PycapResources::getTune( int index )
{
	// check bounds
	if( index >= sRes->tunes.size() )
	{
		// exit, returning None/NULL
		return NULL;
	}

	// return a pointer to the tune object (or NULL if it's been unloaded)
	return tunes[index];
}

//--------------------------------------------------
// loadTune
//--------------------------------------------------
IDirectMusicSegment* PycapResources::loadTune( const std::string& fileName )
{
#if 0
	// largely copied from dx tutorial...
	IDirectMusicSegment* newTune = NULL;
	DMUS_OBJECTDESC ObjDesc; 
	ObjDesc.guidClass = CLSID_DirectMusicSegment;
    ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
    wcscpy_s( ObjDesc.wszFileName, StringToWString( fileName ).c_str() );
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
    HRESULT hr = musicLoader->GetObject(	&ObjDesc,
											IID_IDirectMusicSegment2,
											(void**) &newTune );
	if( FAILED( hr ) )
	{
		PycapApp::sApp->resLoadFailed();
		//PycapApp::sApp->Popup( fileName + " could not be loaded." );
		return NULL;
	}
	newTune->SetParam(GUID_StandardMIDIFile, -1, 0, 0, (void*)PycapApp::sApp->mDMPerformance);
    newTune->SetParam(GUID_Download, -1, 0, 0, (void*)PycapApp::sApp->mDMPerformance);

	// return new tune
	return newTune;
#else
        return NULL;
#endif
}

//--------------------------------------------------
// pLoadImage
//--------------------------------------------------
PyObject* PycapResources::pLoadImage( PyObject* self, PyObject* args )
{
	// parse the arguments
	char* filename;
    if( !PyArg_ParseTuple( args, "s", &filename ) )
        return NULL;

	// load from the file
	Image* newImage = sRes->loadImage( filename );
	if( !newImage )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Failed to load an image file." );

		// exit, returning None/NULL
		return NULL;
	}

	// add image to our collection
	int index;
	// test for free slot
	if( sRes->freeImages.empty() )
	{
		// add new entry in images
		sRes->images.push_back( newImage );

		// set index
		index = sRes->images.size() - 1;
	}
	else
	{
		// set index
		index = sRes->freeImages.back();

		// reuse slot
		( sRes->images[index] ) = newImage;

		// remove free index
		sRes->freeImages.pop_back();
	}

	// return image index value
    return Py_BuildValue( "i", index );
}

//--------------------------------------------------
// pImageWidth
//--------------------------------------------------
PyObject* PycapResources::pImageWidth( PyObject* self, PyObject* args )
{
	// parse the arguments
    int index;
	if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->images.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get image width: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->images[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get image width: Image not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// return image index value
    return Py_BuildValue( "i", sRes->images[index]->mWidth );
}

//--------------------------------------------------
// pImageHeight
//--------------------------------------------------
PyObject* PycapResources::pImageHeight( PyObject* self, PyObject* args )
{
	// parse the arguments
    int index;
	if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->images.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get image height: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->images[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get image height: Image not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// return image index value
    return Py_BuildValue( "i", sRes->images[index]->mHeight );
}

//--------------------------------------------------
// pUnloadImage
//--------------------------------------------------
PyObject* PycapResources::pUnloadImage( PyObject* self, PyObject* args )
{
	// parse the arguments
	int index;
    if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->images.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't unload image: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->images[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't unload image: Image not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// unload the image
	delete sRes->images[index];
	sRes->images[index] = NULL;

	// record empty slot
	sRes->freeImages.push_front( index );

	// done
	return Py_None;
}

//--------------------------------------------------
// pLoadFont
//--------------------------------------------------
PyObject* PycapResources::pLoadFont( PyObject* self, PyObject* args )
{
	// parse the arguments
	char* filename;
    if( !PyArg_ParseTuple( args, "s", &filename ) )
        return NULL;

	// load from the file
	Font* newFont = sRes->loadFont( filename );
	if( !newFont )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Failed to load a font file." );

		// exit, returning None/NULL
		return NULL;
	}

	// add font to our collection
	int index;
	// test for free slot
	if( sRes->freeFonts.empty() )
	{
		// add new entry in fonts
		sRes->fonts.push_back( newFont );

		// set index
		index = sRes->fonts.size() - 1;
	}
	else
	{
		// set index
		index = sRes->freeFonts.back();

		// reuse slot
		( sRes->fonts[index] ) = newFont;

		// remove free index
		sRes->freeFonts.pop_back();
	}

	// return font index value
    return Py_BuildValue( "i", index );
}

//--------------------------------------------------
// pSysFont
//--------------------------------------------------
PyObject* PycapResources::pSysFont( PyObject* self, PyObject* args )
{
#if 0
	// parse the arguments
	// required
	char* faceName;
	int pointSize;
	// optional
	int script		= ANSI_CHARSET;
	int bold		= 0;
	int italics		= 0;
	int underline	= 0;
    if( !PyArg_ParseTuple( args, "si|iiii", &faceName, &pointSize, &script, &bold, &italics, &underline ) )
        return NULL;

	// create
	Font* newFont = sRes->sysFont( faceName, pointSize, script, bold != 0, italics != 0, underline != 0 );
	if( !newFont )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Failed to create a system font." );

		// exit, returning None/NULL
		return NULL;
	}

	// add font to our collection
	int index;
	// test for free slot
	if( sRes->freeFonts.empty() )
	{
		// add new entry in fonts
		sRes->fonts.push_back( newFont );

		// set index
		index = sRes->fonts.size() - 1;
	}
	else
	{
		// set index
		index = sRes->freeFonts.back();

		// reuse slot
		( sRes->fonts[index] ) = newFont;

		// remove free index
		sRes->freeFonts.pop_back();
	}

	// return font index value
    return Py_BuildValue( "i", index );
#else
    return NULL;
#endif
}

//--------------------------------------------------
// pStringWidth
//--------------------------------------------------
PyObject* PycapResources::pStringWidth( PyObject* self, PyObject* args )
{
	// parse the arguments
	char* string;
    int index;
	if( !PyArg_ParseTuple( args, "si", &string, &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->fonts.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get string width: Font index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->fonts[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get string width: Font not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// return string width using font
    return Py_BuildValue( "i", sRes->fonts[index]->StringWidth( string ) );
}

//--------------------------------------------------
// pFontAscent
//--------------------------------------------------
PyObject* PycapResources::pFontAscent( PyObject* self, PyObject* args )
{
	// parse the arguments
    int index;
	if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->fonts.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get font height: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->fonts[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't get font height: Font not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// return font ascent value
    return Py_BuildValue( "i", sRes->fonts[index]->mAscent );
}

//--------------------------------------------------
// pUnloadFont
//--------------------------------------------------
PyObject* PycapResources::pUnloadFont( PyObject* self, PyObject* args )
{
	// parse the arguments
	int index;
    if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->fonts.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't unload font: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->fonts[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't unload font: Font not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// unload the font
	delete sRes->fonts[index];
	sRes->fonts[index] = NULL;

	// record empty slot
	sRes->freeFonts.push_front( index );

	// done
	return Py_None;
}

//--------------------------------------------------
// pSetFontScale
//--------------------------------------------------
PyObject* PycapResources::pSetFontScale( PyObject* self, PyObject* args )
{
	// parse the arguments
	int index;
	float scale;
    if( !PyArg_ParseTuple( args, "if", &index, &scale ) )
        return NULL;

	// test for out of range
	if( index >= sRes->fonts.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't set font point: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( sRes->fonts[index] == NULL )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't set font point font: Font not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// cast to a supporting font type
	ImageFont* imageFont = dynamic_cast<ImageFont*>( sRes->fonts[index] );
	if( !imageFont )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't set font point font: Only supported by image fonts." );

		// exit safely
		return Py_None;
	}

	// set scale
	imageFont->SetScale( scale );

	// done
	return Py_None;
}

//--------------------------------------------------
// pLoadSound
//--------------------------------------------------
PyObject* PycapResources::pLoadSound( PyObject* self, PyObject* args )
{
	// parse the arguments
	char* filename;
    if( !PyArg_ParseTuple( args, "s", &filename ) )
        return NULL;

	// find a free slot
	// (Popcap's sound manager requires us to choose one)
	int slot;
	if( sRes->freeSounds.empty() )
	{
		slot = sRes->sounds.size();
	}
	else
	{
		slot = sRes->freeSounds.back();
	}

	// attempt to load from the file
	if( !sRes->loadSound( slot, filename ) )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Failed to load a sound file." );

		// exit, returning None/NULL
		return NULL;
	}

	// record that we've added the sound
	if( sRes->freeSounds.empty() )
	{
		// add new entry in sounds
		sRes->sounds.push_back( true );
	}
	else
	{
		// reuse slot
		( sRes->sounds[slot] ) = true;

		// remove free index
		sRes->freeSounds.pop_back();
	}

	// return sound index value
    return Py_BuildValue( "i", slot );
}

//--------------------------------------------------
// pUnloadSound
//--------------------------------------------------
PyObject* PycapResources::pUnloadSound( PyObject* self, PyObject* args )
{
	// parse the arguments
	int index;
    if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// test for out of range
	if( index >= sRes->sounds.size() )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't unload sound: Index out of range." );

		// exit, returning None/NULL
		return NULL;
	}

	// test for already unloaded
	if( !sRes->sounds[index] )
	{
		// throw an exception
		PyErr_SetString( PyExc_IOError, "Couldn't unload sound: Sound not loaded." );

		// exit, returning None/NULL
		return NULL;
	}

	// unload the sound
	PycapApp::sApp->mSoundManager->ReleaseSound( index );
	sRes->sounds[index] = false;

	// record empty slot
	sRes->freeSounds.push_front( index );

	// done
	return Py_None;
}

//--------------------------------------------------
// pLoadTune
//--------------------------------------------------
PyObject* PycapResources::pLoadTune( PyObject* self, PyObject* args )
{
#if 0
	// parse the arguments
	char* filename;
    if( !PyArg_ParseTuple( args, "s", &filename ) )
        return NULL;

	// if initialized
	if( PycapApp::sApp->midiInitialized() )
	{
		// load from the file
		IDirectMusicSegment* newTune = sRes->loadTune( filename );
		if( !newTune )
		{
			// throw an exception
			PyErr_SetString( PyExc_IOError, "Failed to load a midi file." );

			// exit, returning None/NULL
			return NULL;
		}

		// add tune to our collection
		int index;
		// test for free slot
		if( sRes->freeTunes.empty() )
		{
			// add new entry in tunes
			sRes->tunes.push_back( newTune );

			// set index
			index = sRes->tunes.size() - 1;
		}
		else
		{
			// set index
			index = sRes->freeTunes.back();

			// reuse slot
			( sRes->tunes[index] ) = newTune;

			// remove free index
			sRes->freeTunes.pop_back();
		}

		// return tune index value
		return Py_BuildValue( "i", index );
	}
#endif
	// failed
	return Py_None;
}

//--------------------------------------------------
// pUnloadTune
//--------------------------------------------------
PyObject* PycapResources::pUnloadTune( PyObject* self, PyObject* args )
{
#if 0
	// parse the arguments
	int index;
    if( !PyArg_ParseTuple( args, "i", &index ) )
        return NULL;

	// if initialized
	if( PycapApp::sApp->midiInitialized() )
	{
		// test for out of range
		if( index >= sRes->tunes.size() )
		{
			// throw an exception
			PyErr_SetString( PyExc_IOError, "Couldn't unload tune: Index out of range." );

			// exit, returning None/NULL
			return NULL;
		}

		// test for already unloaded
		if( sRes->tunes[index] == NULL )
		{
			// throw an exception
			PyErr_SetString( PyExc_IOError, "Couldn't unload tune: Tune not loaded." );

			// exit, returning None/NULL
			return NULL;
		}

		// unload the tune
		sRes->tunes[index]->Release();
		sRes->tunes[index] = NULL;

		// record empty slot
		sRes->freeTunes.push_front( index );
	}

#endif
	// done
	return Py_None;
}
