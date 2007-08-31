/* W.P. van Paassen */

#ifndef __SEXYPHYSICS_H__
#define __SEXYPHYSICS_H__

#include <stdlib.h>
#include <chipmunk/chipmunk.h>
#include "PhysicsListener.h"
#include "SexyVector.h"

namespace Sexy
{
    class PhysicsObject {

    private:
      PhysicsObject():body(NULL), shape(NULL), space(NULL), is_static(false), shape_type(-1){};
      PhysicsObject(cpFloat mass, cpFloat inertia, cpSpace* space, bool is_static=false);
    PhysicsObject(cpBody* body, cpShape* shape):body(body), shape(shape), space(0), shape_type(shape->type), is_static(false){}
      ~PhysicsObject();
            
      friend class Physics;
      
    public:

      cpBody* body; //FIXME move to private
      cpShape* shape;
      cpSpace* space;

      bool is_static;
      int shape_type;

      //body functions
      
      void SetMass(cpFloat m) { cpBodySetMass(body, m); }
      void SetMoment(cpFloat i) { cpBodySetMoment(body, i); }
      void SetAngle(cpFloat a) { cpBodySetAngle(body, a); }
      void ResetForces() { cpBodyResetForces(body);}
      void SetPosition(const SexyVector2&p) { body->p = cpv(p.x,p.y);}
      void ApplyImpulse(const SexyVector2& j, const SexyVector2& r) { cpBodyApplyImpulse(body, cpv(j.x,j.y), cpv(r.x,r.y)); }
      void ApplyForce(const SexyVector2& f, const SexyVector2& r) { cpBodyApplyForce(body, cpv(f.x,f.y), cpv(r.x,r.y)); }
      float GetAngle();
      SexyVector2 GetRotation();
      SexyVector2 GetPosition();

      //shape functions

      void CreateCircleShape(cpFloat radius, const SexyVector2& offset);
      void CreateSegmentShape(const SexyVector2& begin, const SexyVector2& end, cpFloat radius);
      void CreatePolyShape(int numVerts, SexyVector2* vectors, const SexyVector2& offset);
      void SetElasticity(cpFloat e);
      void SetFriction(cpFloat u);

      int GetNumberVertices();
      SexyVector2 GetVertex(int index);
      SexyVector2 GetSegmentShapeBegin();
      SexyVector2 GetSegmentShapeEnd();
      float GetSegmentShapeRadius();
      float GetCircleShapeRadius();
      SexyVector2 GetCircleShapeCenter();

      enum SHAPE_TYPE {
        CIRCLE_SHAPE = 0,
        SEGMENT_SHAPE,
        POLY_SHAPE, 
        NR_SHAPE_TYPES
      };

      
      
    };

    class Physics  {

  public:

    Physics();

    ~Physics();

    void Init();
    bool IsInitialized(){return space != 0;}

    void SetGravity(const SexyVector2& gravity);
    void SetDamping(cpFloat damping);

    void Update();
    void Draw(Graphics* g);

    void SetSteps(int steps) { this->steps = steps;}

    PhysicsObject* CreateObject(cpFloat mass, cpFloat inertia);
    PhysicsObject* CreateStaticObject();
    void DestroyObject(PhysicsObject** object);

    void SetPhysicsListener(PhysicsListener* p) { listener = p;}

    //help functions

      static cpFloat ComputeMomentForPoly(cpFloat moment, int numVerts, SexyVector2* vectors, const SexyVector2& offset) {
        return cpMomentForPoly(moment, numVerts, (cpVect*)vectors, cpv(offset.x, offset.y));
      }

      static cpFloat ComputeMomentForCircle(cpFloat moment, cpFloat r1, cpFloat r2, const SexyVector2& offset) {
        return cpMomentForCircle(moment, r1, r2, cpv(offset.x, offset.y));
      }

      static SexyVector2 RotateVector(const SexyVector2& v1, const SexyVector2& v2) {
        cpVect r = cpvrotate(cpv(v1.x,v1.y), cpv(v2.x,v2.y));
        return SexyVector2(r.x,r.y);
      }
    
  private:
    
    cpSpace* space;
    int steps;
    cpFloat delta;

    static PhysicsListener* listener;
    static void HashQuery(void* ptr, void* data);

  };


};

#endif
