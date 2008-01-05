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

//TODO make physics group an argument to particlesystem

using namespace HGE;

namespace Sexy {

  class ParticlePhysicsSystem: public hgeParticleSystem {

 public:

    ~ParticlePhysicsSystem() {
      for(int i=0; i<nParticlesAlive; i++)
	{
          physics->DestroyObject(particles[i].ph_object);
        }
    }

  ParticlePhysicsSystem(const char *filename, DDImage *sprite, Physics* physics, float fps=0.0f, bool parseMetaData = true, bool old_format=true): hgeParticleSystem(filename,
                                                                                                                                                                     sprite, fps, parseMetaData, old_format), physics(physics), collision_type(124769), collision_group(214964){}

  ParticlePhysicsSystem(hgeParticleSystemInfo *psi, Physics* physics, float fps=0.0f): hgeParticleSystem(psi, fps), physics(physics), collision_type(124769), collision_group(214964){}

  ParticlePhysicsSystem(const ParticlePhysicsSystem &ps):hgeParticleSystem(ps) {
      physics = ps.physics;
      collision_type = ps.collision_type;
      collision_group = ps.collision_group;
    }

    ParticlePhysicsSystem(const hgeParticleSystem &ps, Physics* physics):hgeParticleSystem(ps), physics(physics) {}
    
    void _update(float fDeltaTime);

    void SetCollisionType(unsigned int type) { collision_type = type; }
    void SetCollisionGroup(unsigned int group) { collision_group = group; }
    unsigned int GetCollisionType() const { return collision_type; }
    unsigned int GetCollisionGroup() const { return collision_group; }

  private:
    Physics* physics;
    unsigned int collision_type;
    unsigned int collision_group;

};

}


#endif /* _PARTICLEPHYSICSSYSTEM_H_ */
