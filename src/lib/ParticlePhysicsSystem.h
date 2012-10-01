/* Physics enabled particle engine derived from HGE's particle engine, see directory hgeparticle for HGE's files and copyright notices*/
/* Copyright (c) 2008 W.P. van Paassen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _PARTICLEPHYSICSSYSTEM_H_
#define _PARTICLEPHYSICSSYSTEM_H_

#include "Physics.h"
#include "hgeparticle.h"

using namespace HGE;

namespace Sexy {

class ParticlePhysicsSystem : public hgeParticleSystem {
public:

    ~ParticlePhysicsSystem()
    {
	std::vector<hgeParticle*>::iterator it = particles.begin();
	while (it != particles.end()) {
	    if ((*it)->ph_object)
		physics->DestroyObject((*it)->ph_object);
	    delete *it;
	    ++it;
	}
	particles.clear();
    }

    ParticlePhysicsSystem(const char *filename, DDImage *sprite, Physics* physics, float fps = 0.0f, bool parseMetaData = true, bool old_format = true) : hgeParticleSystem(filename,
    sprite, fps, parseMetaData, old_format), physics(physics), collision_type(124769), collision_group(214964)
    {
    }

    ParticlePhysicsSystem(hgeParticleSystemInfo *psi, Physics* physics, float fps = 0.0f) : hgeParticleSystem(psi, fps), physics(physics), collision_type(124769), collision_group(214964)
    {
    }

    ParticlePhysicsSystem(const ParticlePhysicsSystem &ps) : hgeParticleSystem(ps)
    {
        this->physics = ps.physics;
        this->collision_type = ps.collision_type;
        this->collision_group = ps.collision_group;

	//need to deep copy the physics objects!!
	this->particles.clear();

	std::vector<hgeParticle*>::const_iterator it = ps.particles.begin();
	while (it != ps.particles.end()) {
	    hgeParticle* p = new hgeParticle();
	    *p  = **it;
	    if ((*it)->ph_object) {
		//create physic object
		p->ph_object = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(1.0f, 0.0f, 2.0f, SexyVector2(0.0f,0.0f)));
		p->ph_object->SetPosition((*it)->ph_object->GetPosition());
		p->ph_object->AddCircleShape(2.0f, SexyVector2(0,0),0.9f,2.5f);
		p->ph_object->SetVelocity((*it)->ph_object->GetVelocity());
		p->ph_object->SetCollisionType(this->collision_type);
		p->ph_object->SetGroup(this->collision_group);
	    }
	    this->particles.push_back(p);
	    ++it;
	}
    }

    ParticlePhysicsSystem(const hgeParticleSystem &ps, Physics* physics) : hgeParticleSystem(ps), physics(physics), collision_type(124769), collision_group(214964)
    {
    }

    void _update(float fDeltaTime);

    void SetCollisionType(unsigned int type)
    {
        collision_type = type;
    }

    void SetCollisionGroup(unsigned int group)
    {
        collision_group = group;
    }

    unsigned int GetCollisionType() const
    {
        return collision_type;
    }

    unsigned int GetCollisionGroup() const
    {
        return collision_group;
    }

private:
    Physics* physics;
    unsigned int collision_type;
    unsigned int collision_group;

};

}

#endif /* _PARTICLEPHYSICSSYSTEM_H_ */
