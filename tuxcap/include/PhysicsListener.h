#ifndef __SEXYPHYSICSLISTENER_H__
#define __SEXYPHYSICSLISTENER_H__

namespace Sexy {

  class PhysicsObject;
  class Graphics;

  class PhysicsListener { 

  public:
    virtual void DrawPhysicsObject(PhysicsObject* object, Graphics* g){}
    
  };

};

#endif
