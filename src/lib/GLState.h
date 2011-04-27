/*
 * File:   GLState.h
 * Author: W.P van Paassen
 *
 * Created on April 27th, 2011, 4:12 PM
 */

#ifndef GLSTATE_H
#define GLSTATE_H

#include <SDL.h>
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

class GLState {

 public:
    /**
     * this function returns a pointer to the only instance of this class (singleton)
     * @return a pointer to the only instance of this class
     */
    static GLState* getInstance() {
        if (!_instance)
            _instance = new GLState();
        return _instance;
    }
    ~GLState();

    void enable(GLenum cap) {
        switch(cap) {
        case GL_BLEND:
            if (!blending) {
                glEnable(cap);
                blending = true;
            }
            break;
        case GL_TEXTURE_2D:
            if (!texture2D) {
                glEnable(cap);
                texture2D = true;
            }
            break;
        default:
            glEnable(cap);
        }
        
    }

    void disable(GLenum cap) {
        switch(cap) {
        case GL_BLEND:
            if (blending) {
                glDisable(cap);
                blending = false;
            }
            break;
        case GL_TEXTURE_2D:
            if (texture2D) {
                glDisable(cap);
                texture2D = false;
            }
            break;
        default:
            glDisable(cap);
        }
    }

    void enableClientState(GLenum cap) {
        switch(cap) {
        case GL_TEXTURE_COORD_ARRAY:
            if (!texturecoordarray) {
                glEnableClientState(cap);
                texturecoordarray = true;
            }
            break;
        case GL_VERTEX_ARRAY:
            if (!vertexarray) {
                glEnableClientState(cap);
                vertexarray = true;
            }
            break;
        case GL_COLOR_ARRAY:
            if (!colorarray) {
                glEnableClientState(cap);
                colorarray = true;
            }
            break;
        default:
            glEnableClientState(cap);
        }
    }

    void disableClientState(GLenum cap) {
        switch(cap) {
        case GL_TEXTURE_COORD_ARRAY:
            if (texturecoordarray) {
                glDisableClientState(cap);
                texturecoordarray = false;
            }
            break;
        case GL_VERTEX_ARRAY:
            if (vertexarray) {
                glDisableClientState(cap);
                vertexarray = false;
            }
        case GL_COLOR_ARRAY:
            if (colorarray) {
                glDisableClientState(cap);
                colorarray = false;
            }
            break;
        default:
            glDisableClientState(cap);
        }
    }

 protected:
    bool blending;
    bool texture2D;
    bool texturecoordarray;
    bool vertexarray;
    bool colorarray;

 private:
 GLState():blending(false), texture2D(false), texturecoordarray(false),
        vertexarray(false), colorarray(false){};
    static GLState* _instance;

};

#endif /*GLSTATE_H*/
