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

#include "ParticlePhysicsSystem.h"
#include "hgeRandom.h"

void ParticlePhysicsSystem::_update(float fDeltaTime)
{
    int i;
    float ang;
    hgeParticle *par;
    hgeVector vecAccel, vecAccel2;


    if (fAge != -2.0f) {
        fAge += fDeltaTime;
        if (fAge >= info.fLifetime) {
            fAge = -2.0f;
        }
    }

    // Play Animation
    if(mAnimPlaying)
        _updatePlay(fDeltaTime);

    // update all alive particles

    if(bUpdateBoundingBox) 
        rectBoundingBox.Clear();

    std::vector<hgeParticle*>::iterator it = particles.begin();
    while (it != particles.end()) {
        (*it)->fAge += fDeltaTime;
        if ((*it)->fAge >= (*it)->fTerminalAge) {
            if ((*it)->ph_object)
                physics->DestroyObject((*it)->ph_object);
            delete *it;
            particles.erase(it);
            continue;
        }

        vecAccel = (*it)->vecLocation-vecLocation;
        vecAccel.Normalize();
        vecAccel2 = vecAccel;
        vecAccel *= (*it)->fRadialAccel;

        // vecAccel2.Rotate(M_PI_2);
        // the following is faster
        ang = vecAccel2.x;
        vecAccel2.x = -vecAccel2.y;
        vecAccel2.y = ang;

        vecAccel2 *= (*it)->fTangentialAccel;
        (*it)->vecVelocity += (vecAccel+vecAccel2)*fDeltaTime;
        (*it)->vecVelocity.y += (*it)->fGravity*fDeltaTime;

        if (bOldFormat)
            (*it)->vecLocation += (*it)->vecVelocity;
        else
            (*it)->vecLocation += (*it)->vecVelocity * fDeltaTime;

        (*it)->fSpin += (*it)->fSpinDelta*fDeltaTime;
        (*it)->fSize += (*it)->fSizeDelta*fDeltaTime;
        (*it)->colColor += (*it)->colColorDelta*fDeltaTime; //-----use hgeColor

        if (bUpdateBoundingBox) 
            rectBoundingBox.Encapsulate((*it)->vecLocation.x, (*it)->vecLocation.y);
        ++it;
    }

    // generate new particles

    if (fAge != -2.0f) {
        float fParticlesNeeded = info.nEmission*fDeltaTime + fEmissionResidue;
        int nParticlesCreated = (unsigned int)fParticlesNeeded;
        fEmissionResidue=fParticlesNeeded-nParticlesCreated;

        for (i = 0; i < nParticlesCreated; i++) {
            if (particles.size() >=MAX_PARTICLES) 
                break;

            hgeParticle* par = new hgeParticle();

            par->ph_object = NULL;

            par->fAge = 0.0f;

            //random
            par->fTerminalAge = Random_Float(info.fParticleLifeMin, info.fParticleLifeMax);

            par->vecLocation = vecPrevLocation+(vecLocation-vecPrevLocation)*Random_Float(0.0f, 1.0f);
            par->vecLocation.x += Random_Float(-2.0f, 2.0f);
            par->vecLocation.y += Random_Float(-2.0f, 2.0f);

            ang=info.fDirection-M_PI_2+Random_Float(0,info.fSpread)-info.fSpread/2.0f;
            if(info.bRelative) ang += (vecPrevLocation-vecLocation).Angle()+M_PI_2;
            par->vecVelocity.x = cosf(ang);
            par->vecVelocity.y = sinf(ang);
            par->vecVelocity *= Random_Float(info.fSpeedMin, info.fSpeedMax);

            par->fGravity = Random_Float(info.fGravityMin, info.fGravityMax);
            par->fRadialAccel = Random_Float(info.fRadialAccelMin, info.fRadialAccelMax);
            par->fTangentialAccel = Random_Float(info.fTangentialAccelMin, info.fTangentialAccelMax);

            par->fSize = Random_Float(info.fSizeStart, info.fSizeStart+(info.fSizeEnd-info.fSizeStart)*info.fSizeVar);
            par->fSizeDelta = (info.fSizeEnd-par->fSize) / par->fTerminalAge;

            par->fSpin = Random_Float(info.fSpinStart, info.fSpinStart+(info.fSpinEnd-info.fSpinStart)*info.fSpinVar);
            par->fSpinDelta = (info.fSpinEnd-par->fSpin) / par->fTerminalAge;

            ////-----use hgeColor
            par->colColor.r = Random_Float(info.colColorStart.r, info.colColorStart.r+(info.colColorEnd.r-info.colColorStart.r)*info.fColorVar);
            par->colColor.g = Random_Float(info.colColorStart.g, info.colColorStart.g+(info.colColorEnd.g-info.colColorStart.g)*info.fColorVar);
            par->colColor.b = Random_Float(info.colColorStart.b, info.colColorStart.b+(info.colColorEnd.b-info.colColorStart.b)*info.fColorVar);
            par->colColor.a = Random_Float(info.colColorStart.a, info.colColorStart.a+(info.colColorEnd.a-info.colColorStart.a)*info.fAlphaVar);

            par->colColorDelta.r = (info.colColorEnd.r-par->colColor.r) / par->fTerminalAge;
            par->colColorDelta.g = (info.colColorEnd.g-par->colColor.g) / par->fTerminalAge;
            par->colColorDelta.b = (info.colColorEnd.b-par->colColor.b) / par->fTerminalAge;
            par->colColorDelta.a = (info.colColorEnd.a-par->colColor.a) / par->fTerminalAge;

            //create physic object
            par->ph_object = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(1.0f, 0.0f, 2.0f, SexyVector2(0.0f,0.0f)));
            par->ph_object->SetPosition(SexyVector2(par->vecLocation.x, par->vecLocation.y));
            par->ph_object->AddCircleShape(2.0f, SexyVector2(0,0),0.9f,2.5f);
            par->ph_object->SetVelocity(SexyVector2(par->vecVelocity.x, par->vecVelocity.y));
            par->ph_object->SetCollisionType(collision_type);
            par->ph_object->SetGroup(collision_group);
            
            if (bUpdateBoundingBox) 
                rectBoundingBox.Encapsulate(par->vecLocation.x, par->vecLocation.y);

            particles.push_back(par);
        }
    }

    vecPrevLocation = vecLocation;
}
