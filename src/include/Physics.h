/* Sexy Chipmunk, a physics engine for the PopCap Games Framework using Scott Lembcke's excellent chipmunk physics library */
/* Copyright (c) 2007-2008 W.P. van Paassen
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

#ifndef __SEXYPHYSICS_H__
#define __SEXYPHYSICS_H__

#include <stdlib.h>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>
#include "chipmunk.h"
#include "PhysicsListener.h"
#include "SexyVector.h"
#if defined(__APPLE__)
// uint32_t already defined.
#elif defined(__LINUX__)
#include <linux/types.h>
typedef     __u32       uint32_t;
#endif

namespace Sexy
{
  class CollisionPoint;
  class PhysicsObject;
  class Joint;
 
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
    void SetDelta(float delta) { this->delta = delta; } 

    PhysicsObject* CreateObject(cpFloat mass, cpFloat inertia);
    PhysicsObject* CreateStaticObject();
    void DestroyObject(PhysicsObject* object); 
    bool IsValidObject(PhysicsObject* object) const;

    void SetPhysicsListener(PhysicsListener* p) {   listener = p; }

    void RegisterCollisionType(uint32_t type_a, uint32_t type_b = 0);
    void UnregisterCollisionType(uint32_t type_a, uint32_t type_b = 0);

    std::vector<PhysicsObject*>& GetPhysicsObjects() { return objects;}

    //help functions

    void ApplySpringForce(PhysicsObject* obj1, PhysicsObject* obj2, const SexyVector2& anchor1, 
                           const SexyVector2& anchor2, float rest_length, float spring, float damping); 

    Joint CreatePinJoint(const PhysicsObject* obj1, const PhysicsObject* obj2, const SexyVector2& anchor1, const SexyVector2& anchor2);
    Joint CreateSlideJoint(const PhysicsObject* obj1, const PhysicsObject* obj2, const SexyVector2& anchor1, const SexyVector2& anchor2, float min, float max);
    Joint CreatePivotJoint(const PhysicsObject* obj1, const PhysicsObject* obj2, const SexyVector2& pivot);
    void RemoveJoint(const Joint& joint);
    void RemoveJoint(const PhysicsObject* obj1, const PhysicsObject* obj2);
    void RemoveJoints(const PhysicsObject* obj);
    bool IsJoined(const PhysicsObject* obj1, const PhysicsObject* obj2) const;
    std::vector<std::pair<SexyVector2, SexyVector2> > GetJoints(const PhysicsObject* obj1, const PhysicsObject* obj2) const;
    std::vector<std::pair<SexyVector2, SexyVector2> > GetJoints(const PhysicsObject* obj1) const;
    std::vector<std::pair<SexyVector2, SexyVector2> > GetJoints() const;
    std::set<PhysicsObject*> GetJoinedPhysicsObjects(const PhysicsObject* obj1) const;

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

    static SexyVector2 SumCollisionImpulses(int numContacts, CollisionPoint* contacts);      
    static SexyVector2 SumCollisionImpulsesWithFriction(int numContacts, CollisionPoint* contacts);      

  private:
    
    cpSpace* space;
    int steps;
    cpFloat delta;
    std::vector<PhysicsObject*> objects;
    std::vector<cpJoint*> joints;
    PhysicsListener* listener;

    void AddUniqueJoint(std::vector<std::pair<SexyVector2, SexyVector2> >* v, const SexyVector2& start, const SexyVector2& end) const;  
    const std::vector<cpJoint*> GetJointsOfObject(const PhysicsObject* obj) const;
    void RemoveJoint(const cpJoint* joint);

    static void AllCollisions(void* ptr, void* data);
    static void HashQuery(void* ptr, void* data);
    static int CollFunc(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, cpFloat normal_coef, void *data);
    static PhysicsObject* FindObject(std::vector<PhysicsObject*>* objects, cpBody* body, cpShape* shape);
    static PhysicsObject* FindObject(std::vector<PhysicsObject*>* objects, cpShape* shape);

    typedef struct typed_data { 
      Graphics* graphics;
      std::vector<PhysicsObject*>* objects;
      PhysicsListener* listener;
    } TypedData;
  };

    class PhysicsObject {

    private:
    PhysicsObject():body(NULL), physics(NULL), is_static(false){}
      PhysicsObject(cpFloat mass, cpFloat inertia, Physics* physics, bool is_static=false);
      ~PhysicsObject();
            
      friend class Physics;
      friend class CollisionObject;

      cpBody* body; 
      std::vector<cpShape*> shapes;
      Physics* physics;
      int colliding_shape_index;

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
      float GetAngle() const;
      SexyVector2 GetRotation() const;
      SexyVector2 GetPosition() const;
      SexyVector2 GetVelocity() const;
      
      //shape functions

      void AddCircleShape(cpFloat radius, const SexyVector2& offset, cpFloat elasticity, cpFloat friction);
      void AddSegmentShape(const SexyVector2& begin, const SexyVector2& end, cpFloat radius, cpFloat elasticity, cpFloat friction);
      void AddPolyShape(int numVerts, SexyVector2* vectors, const SexyVector2& offset, cpFloat elasticity, cpFloat friction);
      void SetCollisionType(unsigned int type, int shape_index=0);
      void SetGroup(unsigned int group, int shape_index=0);
      void SetLayers(unsigned int layers, int shape_index=0);
      void SetData(void* data, int shape_index=0);
      unsigned int GetCollisionType(int shape_index=0) const;
      unsigned int GetGroup(int shape_index=0) const;
      unsigned int GetLayers(int shape_index=0) const;
      void* GetData(int shape_index=0) const;
      int GetNumberVertices(int shape_index=0) const;
      SexyVector2 GetVertex(int vertex_index, int shape_index=0) const;
      SexyVector2 GetSegmentShapeBegin(int shape_index=0) const;
      SexyVector2 GetSegmentShapeEnd(int shape_index=0) const;
      float GetSegmentShapeRadius(int shape_index=0) const;
      float GetCircleShapeRadius(int shape_index=0) const;
      SexyVector2 GetCircleShapeCenter(int shape_index=0) const;
      int GetShapeType(int shape_index=0) const;
      int GetNumberOfShapes() const;
      int GetCollidingShapeIndex() const; 
            
      enum SHAPE_TYPE {
        CIRCLE_SHAPE = CP_CIRCLE_SHAPE,
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

      // Calculated by cpArbiterPreStep().
      SexyVector2  r1, r2;
      float  nMass, tMass, bounce;
      // Persistant contact information.
      float jnAcc, jtAcc, jBias;
      float bias;
        
        // Hash value used to (mostly) uniquely identify a contact.
      uint32_t hash;
    };

    class CollisionObject {
    public:

    CollisionObject(PhysicsObject* object1, PhysicsObject* object2, const CollisionPoint* points, int num_points, float normal_coef = 1.0f): 
      object1(object1), object2(object2), points(points), num_points(num_points), normal_coef(normal_coef){}
      ~CollisionObject(){}

      PhysicsObject* object1;
      PhysicsObject* object2;
      const CollisionPoint* points;
      int num_points;
      float normal_coef;
    };

    class Joint {
    private:
    Joint(cpJoint* joint, PhysicsObject* obj1, PhysicsObject* obj2, const SexyVector2& anchor1, const SexyVector2& anchor2): 
      joint(joint), object1(obj1), object2(obj2), anchor1(anchor1), anchor2(anchor2){}
    Joint(cpJoint* joint, PhysicsObject* obj1, PhysicsObject* obj2, const SexyVector2& pivot): 
      joint(joint), object1(obj1), object2(obj2), pivot(pivot){}

      friend class Physics;

      cpJoint* joint;
      PhysicsObject* object1;
      PhysicsObject* object2;
      SexyVector2 anchor1;
      SexyVector2 anchor2;
      SexyVector2 pivot;

    public:
      ~Joint(){}
      const PhysicsObject* GetPhysicsObject1() { return object1; }
      const PhysicsObject* GetPhysicsObject2() { return object2; }
      const SexyVector2* GetAnchor1() { if (joint->type == CP_PIN_JOINT || joint->type == CP_SLIDE_JOINT) return &anchor1; return NULL; } 
      const SexyVector2* GetAnchor2() { if (joint->type == CP_PIN_JOINT || joint->type == CP_SLIDE_JOINT) return &anchor2; return NULL; }
      const SexyVector2* GetPivot() { if (joint->type == CP_PIVOT_JOINT) return &pivot; return NULL; }
    };
};

#endif
