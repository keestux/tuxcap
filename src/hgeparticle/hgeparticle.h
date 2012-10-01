/*
** Haaf's Game Engine 1.5
** Copyright (C) 2003-2004, Relish Games
** hge.relishgames.com
**
** hgeParticleSystem helper class header
** 
** Hacked on by 
**
** Kevin Lynx
** James Poag
** W.P. van Paassen
*/

#ifndef HGEPARTICLE_H
#define HGEPARTICLE_H

#include <vector>

#include "Graphics.h"
#include "Physics.h"
#include "Point.h"
#include "DDImage.h"
#include "PakInterface.h"
#include "Logging.h"

#include "hgevector.h"
#include "hgecolor.h"
#include "hgerect.h"

using namespace Sexy;

namespace HGE
{

#define MAX_PARTICLES   500
#define MAX_PSYSTEMS    100

#ifndef M_PI
#define M_PI    3.14159265358979323846f
#define M_PI_2  1.57079632679489661923f
#define M_PI_4  0.785398163397448309616f
#define M_1_PI  0.318309886183790671538f
#define M_2_PI  0.636619772367581343076f
#endif

struct hgeParticle
{
    hgeVector   vecLocation;
    hgeVector   vecVelocity;

    float       fGravity;
    float       fRadialAccel;
    float       fTangentialAccel;

    float       fSpin;
    float       fSpinDelta;

    float       fSize;
    float       fSizeDelta;

    hgeColor    colColor;       // + alpha
    hgeColor    colColorDelta;

    float       fAge;
    float       fTerminalAge;
          
    PhysicsObject* ph_object;
};

struct hgeParticleSystemInfo
{
    DDImage       *sprite;

    int         nEmission; // particles per sec
    float       fLifetime;

    float       fParticleLifeMin;
    float       fParticleLifeMax;

    float       fDirection;
    float       fSpread;
    bool        bRelative;

    float       fSpeedMin;
    float       fSpeedMax;

    float       fGravityMin;
    float       fGravityMax;

    float       fRadialAccelMin;
    float       fRadialAccelMax;

    float       fTangentialAccelMin;
    float       fTangentialAccelMax;

    float       fSizeStart;
    float       fSizeEnd;
    float       fSizeVar;

    float       fSpinStart;
    float       fSpinEnd;
    float       fSpinVar;

    hgeColor    colColorStart; // + alpha
    hgeColor    colColorEnd;
    float       fColorVar;
    float       fAlphaVar;
};

class hgeParticleSystem
{
public:
    hgeParticleSystem(const char *filename, DDImage *sprite, float fps=0.0f, bool parseMetaData=true, bool old_format=true);
    hgeParticleSystem(hgeParticleSystemInfo *psi, float fps=0.0f);
    hgeParticleSystem(const hgeParticleSystem& system); //copy constructor
    virtual ~hgeParticleSystem();

#ifdef DEBUG
    void                        dumpInfo(const char *fname) const;
#endif

    virtual void                SaveFile(const char *filename);

    virtual void                Play(int thePlayMode = MAX_PLAYMODES); // Plays the Particle along the waypoint's path
    virtual void                Render( Graphics *g );
    virtual void                FireAt(float x, float y);
    virtual void                Fire();
    virtual void                Stop(bool bKillParticles=false);
    virtual void                Update(float fDeltaTime);
    virtual void                MoveTo(float x, float y, bool bMoveParticles=false);
    virtual void                Translate(float x, float y) { fTx=x; fTy=y; }
    virtual void                TrackBoundingBox(bool bTrack) { bUpdateBoundingBox=bTrack; }
    virtual void                SetParticleScale(float scale) { fParticleScale = scale; }
    virtual float               GetParticleScale() const { return fParticleScale; }
    virtual void                SetScale(float scale) { fScale = scale; }
    virtual float               GetScale() const { return fScale; }
    virtual int                 GetParticlesAlive() const { return particles.size(); }
    virtual float               GetAge() const { return fAge; }
    virtual void                GetPosition(float *x, float *y) const { *x=vecLocation.x; *y=vecLocation.y; }
    virtual void                GetTranslation(float *x, float *y) const { *x=fTx; *y=fTy; }
    virtual hgeRect*            GetBoundingBox(hgeRect *rect) const;

    virtual unsigned int        GetCollisionType() const;
    virtual unsigned int        GetCollisionGroup() const;
    virtual void                SetCollisionType(unsigned int type);
    virtual void                SetCollisionGroup(unsigned int group);
    virtual bool                SetDoNotDraw(bool b) { bool draw = doNotDraw; doNotDraw = b; return draw; }

    hgeParticleSystemInfo info;

    std::vector<Sexy::Point>        mPolygonClipPoints;
    std::vector<Sexy::Point>        mWayPoints;

    /*
      whether addtive blend
    */
    bool                mbAdditiveBlend;

    // Texture file to load
    std::string         mTextureName;

    // ANimation state variables
    int                 mPlayMode;
    float               mPlayTime;
    float               mPlayTimer;
    float               mPlayTimerStepSize;
    bool                mAnimPlaying;
    int                 mPlayMarker;
    int                 mPingPong;
    bool                bInitOK;

    LoggerFacil *       mLogFacil;

    enum { PING, PONG } ;

    enum
    {
        STOPPED = -1,
        PLAY_ONCE,
        PLAY_LOOPED,
        PLAY_PINGPONGED,
        MAX_PLAYMODES,
    } ;

protected:
    hgeParticleSystem();

    virtual void        _update(float fDeltaTime);
    virtual void        _updatePlay(float fDeltaTime);


    float               fScale; //scales the particle system
    float               fParticleScale;
    float               fUpdSpeed;
    float               fResidue;

    float               fAge;
    float               fEmissionResidue;

    hgeVector      vecPrevLocation;
    hgeVector      vecLocation;
    float               fTx, fTy;

    hgeRect          rectBoundingBox;
    bool               bUpdateBoundingBox;

    std::vector<hgeParticle*> particles;

    bool               doNotDraw;

    static bool         m_bInitRandom;
    bool                bOldFormat;

    virtual void        InitRandom();

    virtual void        SaveMetaData(FILE* aFile);

    virtual bool        wn_PnPoly(Sexy::Point theTestPoint);
    virtual bool        cn_PnPoly(Sexy::Point theTestPoint);

private:
    virtual void        ParseMetaData(void * _ic);

    // METADATA Tags Enumeration
    // When adding File attributes, add to the END of this list.
    enum {
        ADDITIVE = 0,
        POSITION = 1,
        TEXTURE_PATH = 2,
        POLYGON_POINTS = 3,
        WAY_POINTS = 4,
        ANIMATION_DATA= 5,
    };

};

class hgeParticleManager
{
public:
    hgeParticleManager(float fps=0.0f);
    virtual ~hgeParticleManager();

    virtual void        Update(float dt);
    virtual void        Render( Graphics *g );

    hgeParticleSystem*  SpawnPS(const char *filename, DDImage *sprite, float x, float y, bool parseMetaData = true, bool old_format=true, Physics* physics = NULL);
    hgeParticleSystem*  SpawnPS(hgeParticleSystemInfo *psi, float x, float y, Physics* physics = NULL);
    hgeParticleSystem*  SpawnPS(hgeParticleSystem *system, float x, float y, Physics* physics = NULL);

    virtual bool        IsPSAlive(hgeParticleSystem *ps) const;
    virtual void        Translate(float x, float y);
    virtual void        GetTranslation(float *dx, float *dy) const {*dx=tX; *dy=tY;}
    virtual void        KillPS(hgeParticleSystem *ps);
    virtual void        KillAll();
    virtual void        SetEmissions(int theRate);
    virtual void        SetFPS(float fps) { fFPS = fps; }
    virtual float       GetFPS() { return fFPS; }  

protected:
    hgeParticleManager(const hgeParticleManager &);
    hgeParticleManager& operator= (const hgeParticleManager &);

    float               fFPS;
    float               tX;
    float               tY;
    std::vector<hgeParticleSystem*>  psList;
};

}

#endif
