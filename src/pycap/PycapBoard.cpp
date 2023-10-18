//--------------------------------------------------
// PycapBoard
//
// Base game board for Pycap
// Manages hooks to python game code
//
// Jarrad 'Farbs' Woods
// W.P. van Paassen
//--------------------------------------------------

// includes
#include "PycapBoard.h"
#include "PycapApp.h"

#include "Graphics.h"
#include <Python.h>

#include <math.h>


// namespace
using namespace Sexy;


// functions

//--------------------------------------------------
// PycapBoard
//--------------------------------------------------

PycapBoard::PycapBoard()
{
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error at start of PycapBoard()");
        PyErr_Print();
        return;
    }

    // grab frequently used python functions
    pUpdateFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "update");
    pDrawFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "draw");
    pKeyDownFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "keyDown");
    pKeyUpFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "keyUp");
    pExitGame = PyDict_GetItemString(PycapApp::sApp->pDict, "exitGame");
    pMouseEnterFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "mouseEnter");
    pMouseLeaveFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "mouseLeave");
    pMouseMoveFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "mouseMove");
    pMouseDownFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "mouseDown");
    pMouseUpFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "mouseUp");
    pMouseWheelFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "mouseWheel");
    // legacy spelling
    if (!pKeyDownFunc)
        pKeyDownFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "keydown");
    if (!pKeyUpFunc)
        pKeyUpFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "keyup");


    // init remaining members
    graphics = NULL;

    // call python game init function
    PyObject* pInitFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "init");

    if (pInitFunc) {
        if (PyCallable_Check(pInitFunc)) {
            PyObject_CallObject(pInitFunc, NULL);
	    if (PyErr_Occurred()) {
		PyErr_SetString(PyExc_Exception, "Some kind of python error after PycapBoard() calling init");
		PyErr_Print();
		return;
	    }
        } else {
            //PycapApp::sApp->Popup( StrFormat( "\"init\" found, but not callable" ) );
        }
    }

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in PycapBoard()");
        PyErr_Print();
        return;
    }

    // request initial draw
    MarkDirty();
}

//--------------------------------------------------
// ~PycapBoard
//--------------------------------------------------

PycapBoard::~PycapBoard()
{
    // call python shutdown function
    PyObject* pFiniFunc = PyDict_GetItemString(PycapApp::sApp->pDict, "fini");

    if (pFiniFunc && PyCallable_Check(pFiniFunc)) {
        PyObject_CallObject(pFiniFunc, NULL);
    }
    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in ~PycapBoard()");
        PyErr_Print();
    }
}

//--------------------------------------------------
// Update
//--------------------------------------------------

void PycapBoard::UpdateF(float delta)
{
    // call parent
    Widget::UpdateF(delta);

    // Python exit-check
    // Checked on entering incase a non-update function has set it
    if (pExitGame) {
        PyObject* pExit = PyObject_CallObject(pExitGame, NULL);
        if (PyLong_Check(pExit) && PyLong_AsLong(pExit) != 0) {
            // drop the return value
            Py_DECREF(pExit);

            // exit the program
            PycapApp::sApp->mShutdown = true;

            // no need to update
            return;
        }
        // drop the return value
        Py_DECREF(pExit);
    }

    // Python update hook
    // The python code should call dirty if the screen needs to be redrawn
    if (pUpdateFunc) {
        PyObject* pArgs = PyTuple_New(1);
        PyObject* pDelta = PyFloat_FromDouble(delta);
        PyTuple_SetItem(pArgs, 0, pDelta);
        PyObject_CallObject(pUpdateFunc, pArgs);
        Py_DECREF(pDelta);
        Py_DECREF(pArgs);
    }

    // Python exit-check
    // Checked on exiting updatef incase it has set it
    if (pExitGame) {
        PyObject* pExit = PyObject_CallObject(pExitGame, NULL);
        if (PyLong_Check(pExit) && PyLong_AsLong(pExit) != 0) {
            // drop the return value
            Py_DECREF(pExit);

            // exit the program
            PycapApp::sApp->mShutdown = true;

            // no need to update
            return;
        }
        // drop the return value
        Py_DECREF(pExit);
    }

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in Update");
        PyErr_Print();
        PycapApp::sApp->mShutdown = true;
    }
}

//--------------------------------------------------
// Draw
//--------------------------------------------------

void PycapBoard::Draw(Graphics *g)
{
    // exit early if no python draw function
    if (!pDrawFunc)
        return;

    // enter draw code
    graphics = g;

    // call draw function
    PyObject_CallObject(pDrawFunc, NULL);

    // exit draw code
    graphics = NULL;

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in Draw");
        PyErr_Print();
        PycapApp::sApp->mShutdown = true;
    }
}

//--------------------------------------------------
// KeyDown
//--------------------------------------------------

void PycapBoard::KeyDown(KeyCode key)
{
    // exit early if no python keydown function
    if (!pKeyDownFunc)
        return;

    // Python keydown hook
    PyObject* pArgs = PyTuple_New(1);
    PyObject* pKey = PyLong_FromLong(key);
    PyTuple_SetItem(pArgs, 0, pKey);
    PyObject_CallObject(pKeyDownFunc, pArgs);
    Py_DECREF(pArgs);

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in KeyDown");
        PyErr_Print();
        PycapApp::sApp->mShutdown = true;
    }
}

//--------------------------------------------------
// KeyUp
//--------------------------------------------------

void PycapBoard::KeyUp(KeyCode key)
{
    // exit early if no python keyup function
    if (!pKeyUpFunc)
        return;

    // Python keyup hook
    PyObject* pArgs = PyTuple_New(1);
    PyObject* pKey = PyLong_FromLong(key);
    PyTuple_SetItem(pArgs, 0, pKey);
    PyObject_CallObject(pKeyUpFunc, pArgs);
    Py_DECREF(pArgs);

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in KeyUp");
        PyErr_Print();
        PycapApp::sApp->mShutdown = true;
    }
}

//--------------------------------------------------
// MouseEnter
//--------------------------------------------------

void PycapBoard::MouseEnter()
{
    // call python function if it exists
    if (pMouseEnterFunc)
        PyObject_CallObject(pMouseEnterFunc, NULL);

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in MouseEnter");
        PyErr_Print();
        PycapApp::sApp->mShutdown = true;
    }
}

//--------------------------------------------------
// MouseLeave
//--------------------------------------------------

void PycapBoard::MouseLeave()
{
    // call python function if it exists
    if (pMouseLeaveFunc)
        PyObject_CallObject(pMouseLeaveFunc, NULL);

    // general error location warning
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in MouseLeave");
        PyErr_Print();
        PycapApp::sApp->mShutdown = true;
    }
}

//--------------------------------------------------
// MouseMove
//--------------------------------------------------

void PycapBoard::MouseMove(int x, int y)
{
    // Python mouse move hook
    if (pMouseMoveFunc) {
        PyObject* pArgs = PyTuple_New(2);
        PyObject* pX = PyLong_FromLong(x);
        PyObject* pY = PyLong_FromLong(y);
        PyTuple_SetItem(pArgs, 0, pX);
        PyTuple_SetItem(pArgs, 1, pY);
        PyObject_CallObject(pMouseMoveFunc, pArgs);
        Py_DECREF(pArgs);

        // general error location warning
        if (PyErr_Occurred()) {
            PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in MouseMove");
            PyErr_Print();
            PycapApp::sApp->mShutdown = true;
        }
    }
}

//--------------------------------------------------
// MouseDrag
//--------------------------------------------------

void PycapBoard::MouseDrag(int x, int y)
{
    // This gets called instead of mousemove when dragging.
    // For our purposes, they're the same... so do the same thing!
    MouseMove(x, y);
}

//--------------------------------------------------
// MouseDown
//--------------------------------------------------

void PycapBoard::MouseDown(int x, int y, int theBtnNum, int theClickCount)
{
    // Python mouse down hook
    if (pMouseDownFunc) {
        PyObject* pArgs = PyTuple_New(3);
        PyObject* pX = PyLong_FromLong(x);
        PyObject* pY = PyLong_FromLong(y);
        PyObject* pButton = PyLong_FromLong(theBtnNum);
        PyTuple_SetItem(pArgs, 0, pX);
        PyTuple_SetItem(pArgs, 1, pY);
        PyTuple_SetItem(pArgs, 2, pButton);
        PyObject_CallObject(pMouseDownFunc, pArgs);
        Py_DECREF(pArgs);

        // general error location warning
        if (PyErr_Occurred()) {
            PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in MouseDown");
            PyErr_Print();
            PycapApp::sApp->mShutdown = true;
        }
    }
}

//--------------------------------------------------
// MouseUp
//--------------------------------------------------

void PycapBoard::MouseUp(int x, int y, int theBtnNum, int theClickCount)
{
    // Python mouse up hook
    if (pMouseUpFunc) {
        PyObject* pArgs = PyTuple_New(3);
        PyObject* pX = PyLong_FromLong(x);
        PyObject* pY = PyLong_FromLong(y);
        PyObject* pButton = PyLong_FromLong(theBtnNum);
        PyTuple_SetItem(pArgs, 0, pX);
        PyTuple_SetItem(pArgs, 1, pY);
        PyTuple_SetItem(pArgs, 2, pButton);
        PyObject_CallObject(pMouseUpFunc, pArgs);
        Py_DECREF(pArgs);

        // general error location warning
        if (PyErr_Occurred()) {
            PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in MouseUp");
            PyErr_Print();
            PycapApp::sApp->mShutdown = true;
        }
    }
}

//--------------------------------------------------
// MouseWheel
//--------------------------------------------------

void PycapBoard::MouseWheel(int delta)
{
    // Python mouse move hook
    if (pMouseWheelFunc) {
        PyObject* pArgs = PyTuple_New(1);
        PyObject* pX = PyLong_FromLong(delta);
        PyTuple_SetItem(pArgs, 0, pX);
        PyObject_CallObject(pMouseWheelFunc, pArgs);
        Py_DECREF(pArgs);

        // general error location warning
        if (PyErr_Occurred()) {
            PyErr_SetString(PyExc_Exception, "Some kind of python error occurred in MouseWheel");
            PyErr_Print();
            PycapApp::sApp->mShutdown = true;
        }
    }
}
