/*
 ** Haaf's Game Engine 1.5
 ** Copyright (C) 2003-2004, Relish Games
 ** hge.relishgames.com
 **
 ** hgeParticleManager helper class implementation
 ** Hacked on by
 **
 ** Kevin Lynx
 ** James Poag
 ** W.P. van Paassen
 */

#include "hgeparticle.h"
#include "ParticlePhysicsSystem.h"

using namespace Sexy;
using namespace HGE;

hgeParticleManager::hgeParticleManager(float fps)
{
    fFPS = fps;
    tX = tY = 0.0f;
    psList.clear();
}

hgeParticleManager::~hgeParticleManager()
{
    std::vector<hgeParticleSystem*>::iterator it = psList.begin();
    while (it != psList.end()) {
        delete *it;
        ++it;
    }
}

void hgeParticleManager::SetEmissions(int theRate)
{
    std::vector<hgeParticleSystem*>::iterator it = psList.begin();
    while (it != psList.end()) {
        (*it)->info.nEmission = theRate;
        ++it;
    }
}

hgeParticleSystem* hgeParticleManager::SpawnPS(const char *filename, DDImage *sprite, float x, float y, bool parseMetaData, bool old_format, Physics* physics)
{
    if (psList.size() >= MAX_PSYSTEMS)
        return 0;

    hgeParticleSystem* system;

    if (physics == NULL)
        system = new hgeParticleSystem(filename, sprite, fFPS, parseMetaData, old_format);
    else
        system = new ParticlePhysicsSystem(filename, sprite, physics, fFPS, parseMetaData, old_format);

    if (!system->bInitOK)
        return 0;

    system->FireAt(x, y);
    system->Translate(tX, tY);
    psList.push_back(system);
    return system;
}

hgeParticleSystem* hgeParticleManager::SpawnPS(hgeParticleSystemInfo *psi, float x, float y, Physics* physics)
{
    if (psList.size() >= MAX_PSYSTEMS)
        return 0;

    hgeParticleSystem* system;

    if (physics != NULL) {
        system = new ParticlePhysicsSystem(psi, physics, fFPS);
    }
    else {
        system = new hgeParticleSystem(psi, fFPS);
    }

    if (!system->bInitOK)
        return 0;

    system->FireAt(x, y);
    system->Translate(tX, tY);
    psList.push_back(system);
    return system;
}

hgeParticleSystem* hgeParticleManager::SpawnPS(hgeParticleSystem *system, float x, float y, Physics* physics)
{
    if (psList.size() >= MAX_PSYSTEMS || !system->bInitOK)
        return NULL;

    ParticlePhysicsSystem* s = dynamic_cast<ParticlePhysicsSystem*> (system);

    hgeParticleSystem* psystem;

    if (s != NULL) {
        psystem = new ParticlePhysicsSystem(*s);
    }
    else {
	if (physics != NULL)
            psystem = new ParticlePhysicsSystem(*system, physics);
	else
            psystem = new hgeParticleSystem(*system);
    }
    psystem->FireAt(x, y);
    psystem->Translate(tX, tY);
    psList.push_back(psystem);
    return psystem;
}

void hgeParticleManager::Update(float dt)
{
    std::vector<hgeParticleSystem*>::iterator it = psList.begin();
    while (it != psList.end()) {
        (*it)->Update(dt);
        if ((*it)->GetAge() <= -2.0f && (*it)->GetParticlesAlive() == 0) {
            delete *it;
            psList.erase(it);
        }
        else
            ++it;
    }
}

void hgeParticleManager::Render(Graphics *g)
{
    std::vector<hgeParticleSystem*>::const_iterator it = psList.begin();
    while (it != psList.end()) {
        (*it)->Render(g);
        ++it;
    }
}

bool hgeParticleManager::IsPSAlive(hgeParticleSystem *ps) const
{
    std::vector<hgeParticleSystem*>::const_iterator it = psList.begin();
    while (it != psList.end()) {
        if ((*it) == ps) 
            return true;
        ++it;
    }
    return false;
}

void hgeParticleManager::Translate(float x, float y)
{
    std::vector<hgeParticleSystem*>::iterator it = psList.begin();
    while (it != psList.end()) {
        (*it)->Translate(x,y);
        ++it;
    }
    tX = x;
    tY = y;
}

void hgeParticleManager::KillPS(hgeParticleSystem *ps)
{
    std::vector<hgeParticleSystem*>::iterator it = psList.begin();
    while (it != psList.end()) {
        if ((*it) == ps) {
            delete *it;
            psList.erase(it);
            break;
        }
        else
            ++it;
    }
}

void hgeParticleManager::KillAll()
{
    std::vector<hgeParticleSystem*>::iterator it = psList.begin();
    while (it != psList.end()) {
        delete *it;
        ++it;
    }
    psList.clear();
}
