/* W.P. van Paassen */

#ifndef __SEXYPHYSICS_H__
#define __SEXYPHYSICS_H__

#include <stdlib.h>
#include <vector>
#include <chipmunk/chipmunk.h>
#include "PhysicsListener.h"
#include "SexyVector.h"

namespace Sexy
{
    class Physics  {

      friend class PhysicsObject;

  public:

    Physics();
    ~Physics();

    void Init();
    bool IsInitialized(){return space != 0;}

    void SetGravity(const SexyVector2& gravity);
    void SetDamping(cpFloat damping);
    void SetIterations(int iter);
    void ResizeStaticHash(float dimension, int count);
    void ResizeActiveHash(float dimension, int count);

    void Update();
    void Draw(Graphics* g);
    void Clear();

    void SetSteps(int steps);

    PhysicsObject* CreateObject(cpFloat mass, cpFloat inertia);
    PhysicsObject* CreateStaticObject();
    void DestroyObject(PhysicsObject* object); 

    void SetPhysicsListener(PhysicsListener* p) { listener = p;}

    void RegisterCollisionType(unsigned long type_a, unsigned long type_b = 0, void* data = NULL);
    void UnregisterCollisionType(unsigned long type_a, unsigned long type_b = 0);

    std::vector<PhysicsObject*>& GetPhysicsObjects() { return objects;}

    //help functions

    void ApplySpringForce(PhysicsObject* obj1, PhysicsObject* obj2, const SexyVector2& anchor1, 
                           const SexyVector2& anchor2, float rest_length, float spring, float damping); 

    void CreatePinJoint(const PhysicsObject* obj1, const PhysicsObject* obj2, const SexyVector2& anchor1, const SexyVector2& anchor2);
    void CreateSlideJoint(const PhysicsObject* obj1, const PhysicsObject* obj2, const SexyVector2& anchor1, const SexyVector2& anchor2, float min, float max);
    void CreatePivotJoint(const PhysicsObject* obj1, const PhysicsObject* obj2, const SexyVector2& pivot);

    //TODO store joints for remove joint

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
    std::vector<PhysicsObject*> objects;

    static PhysicsListener* listener;
    static void AllCollisions(void* ptr, void* data);
    static void HashQuery(void* ptr, void* data);
    static int CollFunc(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, void *data);
  };

    class PhysicsObject {

    private:
    PhysicsObject():body(NULL), physics(NULL), is_static(false){}
      PhysicsObject(cpFloat mass, cpFloat inertia, Physics* physics, bool is_static=false);
      PhysicsObject(cpBody* body, cpShape* shape);
      ~PhysicsObject();
            
      friend class Physics;
      friend class CollisionObject;

      cpBody* body; 
      std::vector<cpShape*> shapes;
      Physics* physics;
      
    public:

      bool is_static;

      //body functions
      
      void SetMass(cpFloat m) { cpBodySetMass(body, m); }
      void SetMoment(cpFloat i) { cpBodySetMoment(body, i); }
      void SetAngle(cpFloat a) { cpBodySetAngle(body, a); }
      void ResetForces() { cpBodyResetForces(body);}
      void SetAngularVelocity(cpFloat  w);
      void SetVelocity(const SexyVector2& v);
      void SetPosition(const SexyVector2&p) { body->p = cpv(p.x,p.y);} 
      void UpdatePosition();
      void UpdateVelocity();
      void ApplyImpulse(const SexyVector2& j, const SexyVector2& r) { cpBodyApplyImpulse(body, cpv(j.x,j.y), cpv(r.x,r.y)); }
      void ApplyForce(const SexyVector2& f, const SexyVector2& r) { cpBodyApplyForce(body, cpv(f.x,f.y), cpv(r.x,r.y)); }
      float GetAngle();
      SexyVector2 GetRotation();
      SexyVector2 GetPosition();
      SexyVector2 GetVelocity();
      
      //shape functions

      void AddCircleShape(cpFloat radius, const SexyVector2& offset);
      void AddSegmentShape(const SexyVector2& begin, const SexyVector2& end, cpFloat radius);
      void AddPolyShape(int numVerts, SexyVector2* vectors, const SexyVector2& offset);
      void SetElasticity(cpFloat e, int shape_index=0);
      void SetFriction(cpFloat u, int shape_index=0);
      void SetCollisionType(int type, int shape_index=0);
      int GetCollisionType(int shape_index=0);
      int GetNumberVertices(int shape_index=0);
      SexyVector2 GetVertex(int vertex_index, int shape_index=0);
      SexyVector2 GetSegmentShapeBegin(int shape_index=0);
      SexyVector2 GetSegmentShapeEnd(int shape_index=0);
      float GetSegmentShapeRadius(int shape_index=0);
      float GetCircleShapeRadius(int shape_index=0);
      SexyVector2 GetCircleShapeCenter(int shape_index=0);
      int GetShapeType(int shape_index=0);

      enum SHAPE_TYPE {
        CIRCLE_SHAPE = 0,
        SEGMENT_SHAPE,
        POLY_SHAPE, 
        NR_SHAPE_TYPES
      };            
    };
    
    class CollisionPoint {
    public:
    CollisionPoint(const SexyVector2& point,const SexyVector2& normal, float distance): 
      point(point),normal(normal), distance(distance){}
      ~CollisionPoint(){}

      SexyVector2 point;
      SexyVector2 normal;
      float distance;      
    };

    class CollisionObject {
    public:

      CollisionObject(const PhysicsObject& object1, const PhysicsObject& object2, const std::vector<CollisionPoint>& points): 
      object1(object1), object2(object2), points(points){}
      ~CollisionObject(){}

      PhysicsObject object1;
      PhysicsObject object2;
      std::vector<CollisionPoint> points;
    };
};

#endif
