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
#include "SexyAppBase.h"
#include <vector>
#include <list>

#ifndef __PYCAPRESOURCES_H__
#define __PYCAPRESOURCES_H__

#include <Python.h>

// use the Sexy namespace
namespace Sexy
{

// class declarations
class Image;
class Font;
class ImageFont;
//class SysFont;

// Pycap Resources class
class PycapResources
{
    //------------
    // functions
    //------------

public:
    
    // constructor / destructor
    PycapResources();           // create object and load resources. Also sets up resource management module.
    virtual ~PycapResources();  // destroy object, clean up resources. This probably shouldn't leave the res management module active, but it does. Meh.

    // resource accessors
    Image*                  getImage( int index );
    Font*                   getFont( int index );
    bool                    soundExists( int index );
    int                     getTune( int index );

private:

    // resource loading
    Image*                  loadImage( const std::string& fileName );           // attempt load an image and convert pallete
    Font*                   loadFont( const std::string& fileName );            // attempt load an image font
    Font*                   sysFont(                                            // attempt to create a system font
                                const std::string& faceName,
                                int pointSize,
                                int script,
                                bool bold,
                                bool italics,
                                bool underline
                                );
    bool                    loadSound( int id, const std::string& fileName );   // attempt load a sound into a given slot

    // Python resource handling functions
    // Resources are simply referenced by index. Failed prints errors to err.txt.
    static PyObject* pLoadImage( PyObject* self, PyObject* args );      // attempt to load an image and convert palette. Returns image index on success.
    static PyObject* pImageWidth( PyObject* self, PyObject* args );     // get width of an image resource.
    static PyObject* pImageHeight( PyObject* self, PyObject* args );    // get height of an image resource.
    static PyObject* pUnloadImage( PyObject* self, PyObject* args );    // attempt to unload a given image.
    static PyObject* pLoadFont( PyObject* self, PyObject* args );       // attempt to load a font. Returns font index on success.
    static PyObject* pSysFont( PyObject* self, PyObject* args );        // attempt to create a system font. Returns font index on success.
    static PyObject* pStringWidth( PyObject* self, PyObject* args );    // get width of a string if drawn with a specific font.
    static PyObject* pFontAscent( PyObject* self, PyObject* args );     // get ascent of a font.
    static PyObject* pUnloadFont( PyObject* self, PyObject* args );     // attempt to unload a given font.
    static PyObject* pSetFontScale( PyObject* self, PyObject* args );   // set the draw scale of an image font object
    static PyObject* pLoadSound( PyObject* self, PyObject* args );      // attempt to load a sound.
    static PyObject* pUnloadSound( PyObject* self, PyObject* args );    // attempt to unload a given sound.
    static PyObject* pLoadTune( PyObject* self, PyObject* args );       // attempt to load a music file
    static PyObject* pUnloadTune( PyObject* self, PyObject* args );     // attempt to unload a given music file.
    static PyObject* pGetPixel( PyObject* self, PyObject* args );       // attempt to read pixel data from a locked image.
    static PyObject* pSetPixel( PyObject* self, PyObject* args );       // attempt to set pixel data from a locked image.
    static PyObject* pRefreshPixels( PyObject* self, PyObject* args );  // attempt to refresh an image from memory data.
    static PyObject* pMashPalette( PyObject* self, PyObject* args );    // write garbage into the specified image's palette :)
    static PyObject* pMashImage( PyObject* self, PyObject* args );      // distort image data in some way
    static PyObject* pImageGreyScale( PyObject* self, PyObject* args ); // convert image to grey scale.
    static PyObject* pImageGetLowBound( PyObject* self, PyObject* args );   // Get the lowest none alpha pixel.
    static PyObject* pImageGetHighBound( PyObject* self, PyObject* args );  // Get the highest none alpha pixel.

    //----------
    // members
    //----------

public:

    // instance
    static PycapResources* sRes;    // reference to self. Assumes use as a singleton (very ugly)
    static PyObject* initModule();

private:

    // resources
    std::vector<Image*>                 images;     // collection of image objects
    std::list<int>                      freeImages; // list of empty image slots
    std::vector<Font*>                  fonts;      // collection of font objects
    std::list<int>                      freeFonts;  // list of empty font slots
    std::vector<bool>                   sounds;     // flags indicating whether sounds are valid
    std::list<int>                      freeSounds; // list of empty sound slots
    std::vector<int>                    tunes;      // collection of music segment objects
};

}

#endif // __PYCPRESOURCES_H__
