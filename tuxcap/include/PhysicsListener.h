/* Sexy Chipmunk, a physics engine for the PopCap Games Framework using Scott Lembcke's excellent chipmunk physics library */
/* W.P. van Paassen 2007 */

#ifndef __SEXYPHYSICSLISTENER_H__
#define __SEXYPHYSICSLISTENER_H__

namespace Sexy {

  class PhysicsObject;
  class CollisionObject;
  class Graphics;

  class PhysicsListener { 

  public:
    virtual void DrawPhysicsObject(PhysicsObject* object, Graphics* g){}
    virtual void HandleCollision(CollisionObject* col){}
    virtual void HandleTypedCollision(CollisionObject* col){}
    virtual void BeforePhysicsStep(){}
    virtual void AfterPhysicsStep(){}
  };

};

#endif
