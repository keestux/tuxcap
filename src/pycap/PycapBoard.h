//--------------------------------------------------
// PycapBoard
//
// Base game board for Pycap
// Manages hooks to python game code
//
// Jarrad 'Farbs' Woods
// W.P. van Paassen
//--------------------------------------------------

#ifndef __PYCAPBOARD_H__
#define __PYCAPBOARD_H__

#include "Widget.h"
#include <Python.h>


// use the Sexy namespace
namespace Sexy
{

// class declarations
class Graphics;

// Pycap Board Application class
class PycapBoard : public Widget
{
    //------------
    // functions
    //------------

public:

    // constructor / destructor
    PycapBoard();
    virtual ~PycapBoard();

    // Standard Widget functions
    virtual void Draw(Graphics* g);
    virtual void UpdateF(float delta);
    virtual void KeyDown(KeyCode theKey);
    virtual void KeyUp(KeyCode theKey);
    virtual void MouseEnter();
    virtual void MouseLeave();
    virtual void MouseMove(int x, int y);
    virtual void MouseDrag(int x, int y);
    virtual void MouseDown(int x, int y, int theBtnNum, int theClickCount);
    virtual void MouseUp(int x, int y, int theBtnNum, int theClickCount);
    virtual void MouseWheel(int delta);

    // Accessors
    Graphics*   getGraphics()   { return graphics; }    // latest graphics context, or NULL if there is none currently active.

    //----------
    // members
    //----------
private:

    PyObject*   pUpdateFunc;        // python game update hook
    PyObject*   pDrawFunc;          // python game draw hook
    PyObject*   pKeyDownFunc;       // python game keydown hook
    PyObject*   pKeyUpFunc;         // python game keyup hook
    PyObject*   pExitGame;          // python exit game test hook
    PyObject*   pMouseEnterFunc;    // python game mouse enter hook
    PyObject*   pMouseLeaveFunc;    // python game mouse leave hook
    PyObject*   pMouseMoveFunc;     // python game mouse move hook
    PyObject*   pMouseDownFunc;     // python game mouse down hook
    PyObject*   pMouseUpFunc;       // python game mouse up hook
    PyObject*   pMouseWheelFunc;    // python game mouse wheel movement hook

    Graphics*   graphics;   // pointer to the latest graphics context. This should only be used if canDraw is true, but I won't enforce it.
};

}

#endif // __PYCAPBOARD_H__
