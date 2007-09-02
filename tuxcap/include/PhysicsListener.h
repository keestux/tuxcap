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
