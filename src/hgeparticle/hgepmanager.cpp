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

using        namespace      Sexy;
using	namespace	HGE;

hgeParticleManager::hgeParticleManager(float fps)
{
	nPS=0;
	fFPS=fps;
	tX=tY=0.0f;
}

hgeParticleManager::~hgeParticleManager()
{
	int i;
	for(i=0;i<nPS;i++) delete psList[i];
}
void hgeParticleManager::SetEmissions(int theRate)
{
	int i;
	for(i=0;i<nPS;i++) psList[i]->info.nEmission = theRate;
}

hgeParticleSystem* hgeParticleManager::SpawnPS(const char *filename, DDImage *sprite, float x, float y,  bool parseMetaData, bool old_format, Physics* physics)
{
	if(nPS==MAX_PSYSTEMS) 
          return 0;

        hgeParticleSystem* system;

        if (physics == NULL)
          system = new hgeParticleSystem(filename, sprite, fFPS, parseMetaData, old_format);
        else
          system = new ParticlePhysicsSystem(filename, sprite, physics, fFPS, parseMetaData, old_format);

        if (!system->bInitOK)
          return 0;

	psList[nPS]=system;
	psList[nPS]->FireAt(x,y);
	psList[nPS]->Translate(tX,tY);
	nPS++;
	return psList[nPS-1];
}


hgeParticleSystem* hgeParticleManager::SpawnPS(hgeParticleSystemInfo *psi, float x, float y, Physics* physics)
{
	if(nPS==MAX_PSYSTEMS) 
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

	psList[nPS]=system;

	psList[nPS]->FireAt(x,y);
	psList[nPS]->Translate(tX,tY);
	nPS++;
	return psList[nPS-1];
}

hgeParticleSystem* hgeParticleManager::SpawnPS(hgeParticleSystem *system, float x, float y, Physics* physics)
{
	if(nPS==MAX_PSYSTEMS || !system->bInitOK) 
          return 0;

        hgeParticleSystem* psystem;
        ParticlePhysicsSystem* s = dynamic_cast<ParticlePhysicsSystem*>(system);

        if (s != NULL) 
          psystem = new ParticlePhysicsSystem(*s);
        else if (physics != NULL)
          psystem = new ParticlePhysicsSystem(*system, physics);
        else
          psystem = new hgeParticleSystem(*system);

	psList[nPS]= psystem;
	psList[nPS]->FireAt(x,y);
	psList[nPS]->Translate(tX,tY);
	nPS++;
	return psList[nPS-1];
}

void hgeParticleManager::Update(float dt)
{
	int i;
	for(i=0;i<nPS;i++)
	{
		psList[i]->Update(dt);
		if(psList[i]->GetAge()==-2.0f && psList[i]->GetParticlesAlive()==0)
		{
			delete psList[i];
			psList[i]=psList[nPS-1];
			nPS--;
			i--;
		}
	}
}

void hgeParticleManager::Render( Graphics *g )
{
	int i;
	for(i=0;i<nPS;i++) psList[i]->Render( g );
}

bool hgeParticleManager::IsPSAlive(hgeParticleSystem *ps) const
{
	int i;
	for(i=0;i<nPS;i++) if(psList[i]==ps) return true;
	return false;
}

void hgeParticleManager::Translate(float x, float y)
{
	int i;
	for(i=0;i<nPS;i++) psList[i]->Translate(x,y);
	tX=x; tY=y;
}

void hgeParticleManager::KillPS(hgeParticleSystem *ps)
{
	int i;
	for(i=0;i<nPS;i++)
	{
		if(psList[i]==ps)
		{
			delete psList[i];
			psList[i]=psList[nPS-1];
			nPS--;
			return;
		}
	}
}

void hgeParticleManager::KillAll()
{
	int i;
	for(i=0;i<nPS;i++) delete psList[i];
	nPS=0;
}
