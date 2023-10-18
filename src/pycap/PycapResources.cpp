//--------------------------------------------------
// PycapResources
//
// Pycap application resources
// Handles access to images, sounds etc by python
//
// Jarrad 'Farbs' Woods
// W.P. van Paassen
// Tony Oakden
//--------------------------------------------------

// includes
#include "PycapResources.h"
#include "PycapApp.h"

#include "DDImage.h"
#include "ImageFont.h"
//#include "SysFont.h"
#include "SoundManager.h"
#include "MusicInterface.h"

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

    sRes = this;
    //--------------------------------

    //---------------------------
    // Set up PycapRes module
    static PyMethodDef resMethods[] = {
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
        {"loadTune", pLoadTune, METH_VARARGS, "loadTune( fileName )\nLoad a music file, and return its resource index."},
        {"unloadTune", pUnloadTune, METH_VARARGS, "unloadTune( tune )\nUnload a music file created by loadTune."},
        {"getPixel", pGetPixel, METH_VARARGS, "getPixel( image, x, y )\nReturns a tuple representing the colour and alpha data of the specified pixel."},
        {"setPixel", pSetPixel, METH_VARARGS, "setPixel( image, x, y, r, g, b, a )\nSets the colour and alpha data of the specified pixel. Change will not be visible on screen until refreshPixels is called."},
        {"refreshPixels", pRefreshPixels, METH_VARARGS, "refreshPixels( image )\nSubmits pixel data changes to the image. Without calling this setPixel will have no visible effect. This only needs to be called once to send all setPixel changes though, so try to batch up all your changes into one refresh."},
        {"imageGreyScale", pImageGreyScale, METH_VARARGS, "convert an image to grey scale"},
        {"imageGetHighBound", pImageGetHighBound, METH_VARARGS, "Get the highest none alpha pixel"},
        {"imageGetLowBound", pImageGetLowBound, METH_VARARGS, "Get the lowest none alpha pixel"},
        {"mashPalette", pMashPalette, METH_VARARGS, ""},
        {"mashImage", pMashImage, METH_VARARGS, ""},
        {NULL, NULL, 0, NULL}
    };
    PyModuleDef pycapRes = {
        PyModuleDef_HEAD_INIT,
		"PycapRes",
		"",
		-1,
		resMethods
    };
    PyModule_Create(&pycapRes);
    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in PycapResources(), while importing PycapRes.");
        PyErr_Print();
        return;
    }

    //--------------------------------


    //---------------------------
    // Load and process resources

    if (!PycapApp::sApp->pDict) {
        return;
    }
    PyObject* pLoadFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "loadBase");

    if (pLoadFunc && PyCallable_Check(pLoadFunc)) {
        PyObject_CallObject(pLoadFunc, NULL);
    }
    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in PycapResources(), while running loadBase.");
        PyErr_Print();
        return;
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

    if (sRes == this) {
        sRes = NULL;
    }
    //------------------------------

    //-------------------
    // Clean up resources

    // images
    for (std::vector<Image*>::iterator iit = images.begin(); iit != images.end(); ++iit) {
        delete *iit;
    }
    images.clear();
    freeImages.clear();

    // fonts
    for (std::vector<Font*>::iterator fit = fonts.begin(); fit != fonts.end(); ++fit) {
        delete *fit;
    }
    fonts.clear();
    freeFonts.clear();

    // sounds
    if (PycapApp::sApp->mSoundManager) {
        PycapApp::sApp->mSoundManager->ReleaseSounds();
    }
    sounds.clear();
    freeSounds.clear();

    // music
    for (std::vector<int>::iterator tit = tunes.begin(); tit != tunes.end(); ++tit) {
        if (*tit != -1) {
            if (PycapApp::sApp->mMusicInterface) {
                PycapApp::sApp->mMusicInterface->UnloadMusic(*tit);
            }
        }
    }

    //-------------------
}

//--------------------------------------------------
// getImage
//--------------------------------------------------

Image* PycapResources::getImage(int index)
{
    // check bounds
    if (index >= (int) sRes->images.size()) {
        // exit, returning None/NULL
        return NULL;
    }

    // return a pointer to the image object (or NULL if it's been unloaded)
    return images[index];
}


//--------------------------------------------------
// loadImage
//--------------------------------------------------

Image* PycapResources::loadImage(const std::string& fileName)
{
    Image* newImage = (DDImage*) PycapApp::sApp->GetImage(fileName);
    if (newImage == NULL) {
        PycapApp::sApp->resLoadFailed();
        PyErr_SetString(PyExc_Exception, ("Image " + fileName + " could not be loaded").c_str());
        PyErr_Print();
        return NULL;
    }

    // palletize
    ((DDImage*) newImage)->Palletize(); // attempt to palletize, don't worry if it fails

    // return new image
    return newImage;
}

//--------------------------------------------------
// getFont
//--------------------------------------------------

Font* PycapResources::getFont(int index)
{
    // check bounds
    if (index >= (int) sRes->fonts.size()) {
        // exit, returning None/NULL
        return NULL;
    }

    // return a pointer to the font object (or NULL if it's been unloaded)
    return fonts[index];
}


//--------------------------------------------------
// loadFont
//--------------------------------------------------

Font* PycapResources::loadFont(const std::string& fileName)
{
    ImageFont* newFont = new ImageFont(PycapApp::sApp, fileName);
    if (!newFont->mFontData->mInitialized) {
        delete newFont;
        PycapApp::sApp->resLoadFailed();
        PyErr_SetString(PyExc_Exception, ("Font " + fileName + " could not be loaded").c_str());
        PyErr_Print();
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
    SysFont* newFont = new SysFont(PycapApp::sApp, faceName, pointSize, script, bold, italics, underline);
    if (!newFont->mHFont) {
        delete newFont;
        PycapApp::sApp->resLoadFailed();
        //PycapApp::sApp->Popup( faceName + " system font could not be created." );
        return NULL;
    }

    // return new font
    return newFont;
#else
    PyErr_SetString(PyExc_Exception, ("System Font " + faceName + " could not be loaded").c_str());
    PyErr_Print();
    return NULL;
#endif
}

//--------------------------------------------------
// getSound
//--------------------------------------------------

bool PycapResources::soundExists(int index)
{
    // check bounds
    if (index >= (int) sRes->sounds.size()) {
        // out of bounds, so sound doesn't exist
        return false;
    }

    // return whether the sound exists or not from our record
    return sounds[index];
}


//--------------------------------------------------
// loadSound
//--------------------------------------------------

bool PycapResources::loadSound(int id, const std::string& fileName)
{
    // attempt to load
    if (!PycapApp::sApp->mSoundManager || !PycapApp::sApp->mSoundManager->LoadSound(id, fileName)) {
        // report error
        PycapApp::sApp->resLoadFailed();
        PyErr_SetString(PyExc_Exception, ("Sound " + fileName + " could not be loaded").c_str());
        PyErr_Print();

        return false;
    }

    // success!
    return true;
}

//--------------------------------------------------
// getTune
//--------------------------------------------------

int PycapResources::getTune(int index)
{
    if (index == -1) {
        return tunes.size() - 1;
    }


    // check bounds
    if (index >= (int) sRes->tunes.size()) {
        // exit, returning None/NULL
        return -1;
    }

    return index;
}

//--------------------------------------------------
// pLoadImage
//--------------------------------------------------

PyObject* PycapResources::pLoadImage(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* filename;
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "loadImage: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // load from the file
    Image* newImage = sRes->loadImage(filename);
    if (!newImage) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "loadImage: Failed to load image file");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // add image to our collection
    int index;
    // test for free slot
    if (sRes->freeImages.empty()) {
        // add new entry in images
        sRes->images.push_back(newImage);

        // set index
        index = sRes->images.size() - 1;
    } else {
        // set index
        index = sRes->freeImages.back();

        // reuse slot
        (sRes->images[index]) = newImage;

        // remove free index
        sRes->freeImages.pop_back();
    }

    // return image index value
    return Py_BuildValue("i", index);
}

//--------------------------------------------------
// pImageWidth
//--------------------------------------------------

PyObject* PycapResources::pImageWidth(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "imageWidth: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get image width: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get image width: Image not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // return image index value
    return Py_BuildValue("i", sRes->images[index]->GetWidth());
}

//--------------------------------------------------
// pImageHeight
//--------------------------------------------------

PyObject* PycapResources::pImageHeight(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "imageHeight: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get image height: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get image height: Image not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // return image index value
    return Py_BuildValue("i", sRes->images[index]->GetHeight());
}

//--------------------------------------------------
// pUnloadImage
//--------------------------------------------------

PyObject* PycapResources::pUnloadImage(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "unloadImage: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't unload image: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't unload image: Image not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // unload the image
    delete sRes->images[index];
    sRes->images[index] = NULL;

    // record empty slot
    sRes->freeImages.push_front(index);

    // done
    return Py_None;
}

//--------------------------------------------------
// pLoadFont
//--------------------------------------------------

PyObject* PycapResources::pLoadFont(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* filename;
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "loadFont: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // load from the file
    Font* newFont = sRes->loadFont(filename);
    if (!newFont) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to load a font file.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // add font to our collection
    int index;
    // test for free slot
    if (sRes->freeFonts.empty()) {
        // add new entry in fonts
        sRes->fonts.push_back(newFont);

        // set index
        index = sRes->fonts.size() - 1;
    } else {
        // set index
        index = sRes->freeFonts.back();

        // reuse slot
        (sRes->fonts[index]) = newFont;

        // remove free index
        sRes->freeFonts.pop_back();
    }

    // return font index value
    return Py_BuildValue("i", index);
}

//--------------------------------------------------
// pSysFont
//--------------------------------------------------

PyObject* PycapResources::pSysFont(PyObject* self, PyObject* args)
{
#if 0
    // parse the arguments
    // required
    char* faceName;
    int pointSize;
    // optional
    int script = ANSI_CHARSET;
    int bold = 0;
    int italics = 0;
    int underline = 0;
    if (!PyArg_ParseTuple(args, "si|iiii", &faceName, &pointSize, &script, &bold, &italics, &underline))
        return NULL;

    // create
    Font* newFont = sRes->sysFont(faceName, pointSize, script, bold != 0, italics != 0, underline != 0);
    if (!newFont) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to create a system font.");

        // exit, returning None/NULL
        return NULL;
    }

    // add font to our collection
    int index;
    // test for free slot
    if (sRes->freeFonts.empty()) {
        // add new entry in fonts
        sRes->fonts.push_back(newFont);

        // set index
        index = sRes->fonts.size() - 1;
    } else {
        // set index
        index = sRes->freeFonts.back();

        // reuse slot
        (sRes->fonts[index]) = newFont;

        // remove free index
        sRes->freeFonts.pop_back();
    }

    // return font index value
    return Py_BuildValue("i", index);
#else
    // throw an exception
    PyErr_SetString(PyExc_Exception, "sysfont: not supported!!");
    PyErr_Print();

    // exit, returning None/NULL
    Py_INCREF(Py_None);
    return Py_None;
#endif
}

//--------------------------------------------------
// pStringWidth
//--------------------------------------------------

PyObject* PycapResources::pStringWidth(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* string;
    int index;
    if (!PyArg_ParseTuple(args, "si", &string, &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "stringWidth: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->fonts.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get string width: Font index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->fonts[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get string width: Font not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // return string width using font
    return Py_BuildValue("i", sRes->fonts[index]->StringWidth(string));
}

//--------------------------------------------------
// pFontAscent
//--------------------------------------------------

PyObject* PycapResources::pFontAscent(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "fontAscent: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }
    // test for out of range
    if (index >= (int) sRes->fonts.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get font height: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // test for already unloaded
    if (sRes->fonts[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get font height: Font not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // return font ascent value
    return Py_BuildValue("i", sRes->fonts[index]->mAscent);
}

//--------------------------------------------------
// pUnloadFont
//--------------------------------------------------

PyObject* PycapResources::pUnloadFont(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "unloadFont: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->fonts.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't unload font: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->fonts[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't unload font: Font not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // unload the font
    delete sRes->fonts[index];
    sRes->fonts[index] = NULL;

    // record empty slot
    sRes->freeFonts.push_front(index);

    // done
    return Py_None;
}

//--------------------------------------------------
// pSetFontScale
//--------------------------------------------------

PyObject* PycapResources::pSetFontScale(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    float scale;
    if (!PyArg_ParseTuple(args, "if", &index, &scale)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "setFontScale: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->fonts.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't set font point: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // test for already unloaded
    if (sRes->fonts[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't set font point font: Font not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // cast to a supporting font type
    ImageFont* imageFont = dynamic_cast<ImageFont*> (sRes->fonts[index]);
    if (!imageFont) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't set font point font: Only supported by image fonts.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // set scale
    imageFont->SetScale(scale);

    // done
    return Py_None;
}

//--------------------------------------------------
// pLoadSound
//--------------------------------------------------

PyObject* PycapResources::pLoadSound(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* filename;
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "loadSound: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // find a free slot
    // (Popcap's sound manager requires us to choose one)
    int slot;
    if (sRes->freeSounds.empty()) {
        slot = sRes->sounds.size();
    } else {
        slot = sRes->freeSounds.back();
    }

    // attempt to load from the file
    if (!sRes->loadSound(slot, filename)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to load a sound file.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // record that we've added the sound
    if (sRes->freeSounds.empty()) {
        // add new entry in sounds
        sRes->sounds.push_back(true);
    } else {
        // reuse slot
        (sRes->sounds[slot]) = true;

        // remove free index
        sRes->freeSounds.pop_back();
    }

    // return sound index value
    return Py_BuildValue("i", slot);
}

//--------------------------------------------------
// pUnloadSound
//--------------------------------------------------

PyObject* PycapResources::pUnloadSound(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "unloadSound: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->sounds.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't unload sound: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (!sRes->sounds[index]) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't unload sound: Sound not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // unload the sound
    PycapApp::sApp->mSoundManager->ReleaseSound(index);
    sRes->sounds[index] = false;

    // record empty slot
    sRes->freeSounds.push_front(index);

    // done
    return Py_None;
}

//--------------------------------------------------
// pLoadTune
//--------------------------------------------------

PyObject* PycapResources::pLoadTune(PyObject* self, PyObject* args)
{
    // parse the arguments
    char* filename;
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "loadTune: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    int index = sRes->tunes.size();

    if (!PycapApp::sApp->mMusicInterface || !PycapApp::sApp->mMusicInterface->LoadMusic(index, filename)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Failed to load a music file.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    sRes->tunes.push_back(index);

    // return tune index value
    return Py_BuildValue("i", index);
}

//--------------------------------------------------
// pUnloadTune
//--------------------------------------------------

PyObject* PycapResources::pUnloadTune(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "unloadTune: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    if (index >= (int) sRes->tunes.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Unable to unload music, invalid index!");
        // exit, returning None/NULL
        return Py_None;
    }

    std::vector<int>::iterator it = std::find(sRes->tunes.begin(), sRes->tunes.end(), index);
    if (it != sRes->tunes.end()) {
        PycapApp::sApp->mMusicInterface->UnloadMusic(index);
        *it = -1;
    } else {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Unable to unload music, music not found!");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // done
    return Py_None;
}

//--------------------------------------------------
// pGetPixel
//--------------------------------------------------

PyObject* PycapResources::pGetPixel(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    int x, y;
    if (!PyArg_ParseTuple(args, "iii", &index, &x, &y)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "getPixel: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get pixel for image: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get pixel for image: Image not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // grab pixels
    DDImage* image = (DDImage*) (sRes->images[index]);
    if (image) {
        uint32_t* bits = image->GetBits();
        if (bits) {
            // get requested pixel
            if (x < image->GetWidth() && y < image->GetHeight()) {
                // grab
                uint32_t p = bits[ x + y * image->GetWidth() ];

                // extract colour values
                unsigned char alpha = (unsigned char) (p >> 24);
                unsigned char red = (unsigned char) ((p >> 16) & 0xFF);
                unsigned char green = (unsigned char) ((p >> 8) & 0xFF);
                unsigned char blue = (unsigned char) (p & 0xFF);

                // return colour values
                return Py_BuildValue("iiii", red, green, blue, alpha);
            } else {
                // throw an exception
                PyErr_SetString(PyExc_Exception, "Couldn't get pixel: Coordinate is out of image bounds.");
                PyErr_Print();

                // exit, returning None/NULL
                Py_INCREF(Py_None);
                return Py_None;

            }
        } else {
            // throw an exception
            PyErr_SetString(PyExc_Exception, "Couldn't get pixel for image: GetBits() failed.");
            PyErr_Print();

            // exit, returning None/NULL
            Py_INCREF(Py_None);
            return Py_None;

        }
    } else {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't get pixel for image: Image wouldn't cast to DDImage.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }
}

//--------------------------------------------------
// pSetPixel
//--------------------------------------------------

PyObject* PycapResources::pSetPixel(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    int x, y;
    int r, g, b, a;
    if (!PyArg_ParseTuple(args, "iiiiiii", &index, &x, &y, &r, &g, &b, &a)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "setPixel: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't set pixel for image: Index out of range.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't set pixel for image: Image not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // grab pixels
    DDImage* image = (DDImage*) (sRes->images[index]);
    if (image) {
        uint32_t* bits = image->GetBits();
        if (bits) {
            // set requested pixel
            if (x < image->GetWidth() && y < image->GetHeight()) {
                // set
                bits[ x + y * image->GetWidth() ] =
                        (((uint32_t) a) << 24) | // alpha
                        (((uint32_t) r) << 16) | // red
                        (((uint32_t) g) << 8) | // green
                        (((uint32_t) b) << 0); // blue

                // done
                Py_INCREF(Py_None);
                return Py_None;
            } else {
                // throw an exception
                PyErr_SetString(PyExc_Exception, "Couldn't set pixel: Coordinate is out of image bounds.");
                PyErr_Print();

                // exit, returning None/NULL
                Py_INCREF(Py_None);
                return Py_None;

            }
        } else {
            // throw an exception
            PyErr_SetString(PyExc_Exception, "Couldn't set pixel for image: GetBits() failed.");
            PyErr_Print();

            // exit, returning None/NULL
            Py_INCREF(Py_None);
            return Py_None;

        }
    } else {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't set pixel for image: Image wouldn't cast to DDImage.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }
}

//--------------------------------------------------
// pRefreshPixels
//--------------------------------------------------

PyObject* PycapResources::pRefreshPixels(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "refreshPixels: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't refresh pixels for image: Index out of range.");

        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't refresh pixels for image: Image not loaded.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // grab image
    DDImage* image = (DDImage*) (sRes->images[index]);
    if (image) {
        // update
        image->BitsChanged();

        // done
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't refresh pixels for image: Image wouldn't cast to DDImage.");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }
}

struct PaletteMashLookup {
    uint32_t OldValue;
    uint32_t NewValue;
};

//--------------------------------------------------
// pMashPalette
//--------------------------------------------------

PyObject* PycapResources::pMashPalette(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "mashPalette: failed to parse arguments");
        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't mash palette for image: Image not loaded.");

        PyErr_Print();

        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // pixels
    DDImage* image = (DDImage*) (sRes->images[index]);
    if (image) {
        uint32_t* bits = image->GetBits();
        if (bits) {
            // palette lookup goes here
            std::list<PaletteMashLookup> lookup;

            // pass through image, remapping palette entries as they're found
            for (int i = 0; i < image->GetWidth() * image->GetHeight(); ++i) {
                // grab pixel
                uint32_t p = bits[i];
                uint32_t m;

                // try to find in palette
                bool done = false;
                for (std::list<PaletteMashLookup>::iterator it = lookup.begin(); !done && it != lookup.end(); ++it) {
                    // match?
                    if ((*it).OldValue == (p & 0xffffff)) {
                        // match!
                        m = (*it).NewValue;
                        done = true;
                    }
                }

                // add new entry if not found
                if (!done) {
                    PaletteMashLookup newEntry;
                    newEntry.OldValue = p & 0xffffff;
                    newEntry.NewValue = Rand() & 0xffffff;
                    m = newEntry.NewValue;
                    lookup.push_back(newEntry);
                }

                // set new pixel value
                p = (p & 0xff000000) | m;
                bits[i] = p;
            }

            // update image data
            image->BitsChanged();

            // clean up
            lookup.empty();
        }
    }

    // done
    Py_INCREF(Py_None);
    return Py_None;
}

//--------------------------------------------------
// pMashImage
//--------------------------------------------------

PyObject* PycapResources::pMashImage(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "mashImage: failed to parse arguments");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "Couldn't mash image: Image not loaded.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // pixels

    DDImage* image = (DDImage*) (sRes->images[index]);
    if (image) {
        uint32_t* bits = image->GetBits();
        if (bits) {
            // store strips in here
            uint32_t* strip = new uint32_t[image->GetWidth()];

            // for each line
            for (int y = 0; y < image->GetHeight(); ++y) {
                // copy line into buffer
                memcpy(strip, &bits[ image->GetWidth() * y ], image->GetWidth() * sizeof ( uint32_t));

                // pick an offset
                int offset = rand() % image->GetWidth();

                // copy from buffer back into line
                memcpy(&bits[ image->GetWidth() * y ], &strip[offset], (image->GetWidth() - offset) * sizeof ( uint32_t));
                memcpy(&bits[ image->GetWidth() * y + image->GetWidth() - offset ], strip, offset * sizeof ( uint32_t));
            }

            // update image data
            image->BitsChanged();

            // clean up
            delete[] strip;
        }
    }

    // done
    Py_INCREF(Py_None);
    return Py_None;
}

//functions by Tony Oakden

PyObject* PycapResources::pImageGreyScale(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "imageGreyScale: failed to parse arguments");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Couldn't convert to grey scale");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;

    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Couldn't convert to grey scale: Image not loaded.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    DDImage* image = (DDImage*) (sRes->images[index]);
    uint32_t* bits = image->GetBits();

    // 3. Now we will loop over each pixel in the image. The size of the bits array
    // is simply the width times the height.
    for (int i = 0; i < image->GetWidth() * image->GetHeight(); i++) {
        // 4. Get the ARGB color value for this pixel
        uint32_t c = bits[i];

        // 5. To illustrate the ARGB storage format, we will assign each
        // component to a variable, although we're actually only going to care
        // about the RGB values, for this particular example. The 4 lines below
        // extract out the individual ARGB values.
        unsigned char alpha = (unsigned char) (c >> 24);
        unsigned char red = (unsigned char) ((c >> 16) & 0xFF);
        unsigned char green = (unsigned char) ((c >> 8) & 0xFF);
        unsigned char blue = (unsigned char) (c & 0xFF);

        // 6. Just like the Color class, the ARGB values are from 0-255.
        // Let's alter these to produce a grayscale image using one of many
        // conversion methods. This method uses 30% of the red value,
        // 59% of the green value, and 11% of the blue value:
        uint32_t gray = (uint32_t) ((float) red * 0.30f + (float) green * 0.59f + (float) blue * 0.11f);
        // 7. Now we need to put the pixel data back into the image's data.
        // We do the opposite of how we extracted the ARGB values above and
        // use a left shift instead of a right shift:

        //            alpha          red           green       blue
        //bits[i] = (alpha << 24) | (gray << 16) | (gray << 8) | gray;
        bits[i] = (alpha << 24) | (alpha << 16) | (gray << 8) | gray;
    }

    // The image won't use this modified data until we inform it that we've 
    // done some messing around with it. We do that with the BitsChanged()
    // function call. After that, we're all done! Pretty simple. It just
    // depends on what you want to actually do with the RGBA data. Extracting
    // the information and putting it back is as simple as a few shifts.
    image->BitsChanged();
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* PycapResources::pImageGetLowBound(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "imageGetLowBound: failed to parse arguments");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Couldn't convert to grey scale");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Couldn't convert to grey scale: Image not loaded.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    DDImage* image = (DDImage*) (sRes->images[index]);
    uint32_t* bits = image->GetBits();
    int imageHeight = image->GetHeight();
    int imageWidth = image->GetWidth();
    for (int row = imageHeight - 1; row > 0; row--) {
        for (int collumn = 0; collumn < imageWidth; collumn += 2) {
            uint32_t index = (row * imageWidth) + collumn;
            uint32_t c = bits[index];
            unsigned char alpha = (unsigned char) (c >> 24);
            if (alpha > 20) {
                return Py_BuildValue("i", row);
            }

        }
    }
    image->BitsChanged();
    return Py_BuildValue("i", 0); //only get here if the image contains no alpha above 20
}

PyObject* PycapResources::pImageGetHighBound(PyObject* self, PyObject* args)
{
    // parse the arguments
    int index;
    if (!PyArg_ParseTuple(args, "i", &index)) {
        // throw an exception
        PyErr_SetString(PyExc_Exception, "imageGetHighBound: failed to parse arguments");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for out of range
    if (index >= (int) sRes->images.size()) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Couldn't convert to grey scale");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    // test for already unloaded
    if (sRes->images[index] == NULL) {
        // throw an exception
        PyErr_SetString(PyExc_IOError, "Couldn't convert to grey scale: Image not loaded.");
        PyErr_Print();
        // exit, returning None/NULL
        Py_INCREF(Py_None);
        return Py_None;
    }

    DDImage* image = (DDImage*) (sRes->images[index]);
    uint32_t* bits = image->GetBits();
    int imageHeight = image->GetHeight();
    int imageWidth = image->GetWidth();
    for (int row = 0; row < imageHeight; row++) {
        for (int collumn = 0; collumn < imageWidth; collumn += 2) {
            uint32_t index = (row * imageWidth) + collumn;
            uint32_t c = bits[index];
            unsigned char alpha = (unsigned char) (c >> 24);
            if (alpha > 20)
                return Py_BuildValue("i", row);

        }
    }
    return Py_BuildValue("i", 0); //only get here if the image contains no alpha above 20
}
