/* W.P. van Paassen */

#include "Physics.h"
#include <assert.h>
#include <stdlib.h>

using namespace Sexy;

PhysicsListener* Physics::listener = NULL;

PhysicsObject::PhysicsObject(cpFloat mass, cpFloat inertia, cpSpace* space, bool is_static):space(space), is_static(is_static), shape_type(-1) {
  body = cpBodyNew(mass, inertia);
  if (!is_static)
    cpSpaceAddBody(space, body);
  shape = NULL;
}

PhysicsObject::~PhysicsObject() {
  //FIXME
#if 0
  if (shape != NULL) {
    if (space != NULL)
      cpSpaceRemoveShape(space, shape);
    else
      cpSpaceRemoveStaticShape(space, shape);
  }

  cpSpaceRemoveBody(space, body);
#endif
  body = NULL;
  shape = NULL;
  space = NULL;
}

float PhysicsObject::GetAngle() {
  assert(body != NULL);
  return (float)body->a;
}

SexyVector2 PhysicsObject::GetRotation() {
  assert(body != NULL);
  return SexyVector2(body->rot.x, body->rot.y);
}

SexyVector2 PhysicsObject::GetPosition() {
  assert(body != NULL);
  return SexyVector2(body->p.x, body->p.y);
}

void PhysicsObject::CreateCircleShape(cpFloat radius, const SexyVector2& offset) {
  assert(shape == NULL && body != NULL);
  shape = cpCircleShapeNew(body, radius, cpv(offset.x, offset.y));
  assert(shape != NULL);
  if (space != NULL) {
    if (is_static)
      cpSpaceAddStaticShape(space, shape);
    else
      cpSpaceAddShape(space, shape);
  }
}

void PhysicsObject::CreateSegmentShape(const SexyVector2& begin, const SexyVector2& end, cpFloat radius) {
  assert(shape == NULL && body != NULL);
  shape = cpSegmentShapeNew(body, cpv(begin.x, begin.y), cpv(end.x,end.y), radius);
  assert(shape != NULL);
 if (space != NULL) {
    if (is_static)
      cpSpaceAddStaticShape(space, shape);
    else
      cpSpaceAddShape(space, shape);
  }
}

void PhysicsObject::CreatePolyShape(int numVerts, SexyVector2* vectors, const SexyVector2& offset) {
  //TODO free shape if not null
  assert(shape == NULL && body != NULL);
  shape = cpPolyShapeNew(body, numVerts, (cpVect*)vectors, cpv(offset.x, offset.y));
  assert(shape != NULL);
 if (space != NULL) {
    if (is_static)
      cpSpaceAddStaticShape(space, shape);
    else
      cpSpaceAddShape(space, shape);
  }
}

void PhysicsObject::SetElasticity(cpFloat e) {
  assert(shape != NULL);
  shape->e = e;
}

void PhysicsObject::SetFriction(cpFloat u) {
  assert(shape != NULL);
  shape->u = u;
}

int PhysicsObject::GetNumberVertices() {
  if (shape_type == POLY_SHAPE) {
    return ((cpPolyShape*)shape)->numVerts;
  }
  return -1;
}

SexyVector2* PhysicsObject::GetVertices() {
  if (shape_type == POLY_SHAPE) {
    return (SexyVector2*)(((cpPolyShape*)shape)->verts);
  }
  return NULL;
}

SexyVector2 PhysicsObject::GetSegmentShapeBegin() {
  if (shape_type == SEGMENT_SHAPE) {
    return SexyVector2(((cpSegmentShape*)shape)->a.x,((cpSegmentShape*)shape)->a.y) ;   
  }  
}

SexyVector2 PhysicsObject::GetSegmentShapeEnd() {
  if (shape_type == SEGMENT_SHAPE) {
    return SexyVector2(((cpSegmentShape*)shape)->b.x,((cpSegmentShape*)shape)->b.y) ;   
  }  
}

float PhysicsObject::GetSegmentShapeRadius() {
  if (shape_type == SEGMENT_SHAPE) {
    return (float)((cpSegmentShape*)shape)->r;
  }  
}

float PhysicsObject::GetCircleShapeRadius() {
  if (shape_type == CIRCLE_SHAPE) {
    return (float)((cpCircleShape*)shape)->r;    
  }    
}

SexyVector2 PhysicsObject::GetCircleShapeCenter() {
  if (shape_type == CIRCLE_SHAPE) {
    return SexyVector2(((cpCircleShape*)shape)->c.x,((cpCircleShape*)shape)->c.y);     //FIXME does c really stand for the center??
  }    
}

Physics::Physics():space(NULL),steps(1){}

Physics::~Physics() {

  //TODO store physicsobjects in list/vector to clean up and avoid memory leaks

  cpSpaceFreeChildren(space);
  cpSpaceFree(space);
  space = NULL;
}

void Physics::Init() {
  if (space == NULL) {
    
    cpInitChipmunk();

    cpResetShapeIdCounter();
    space = cpSpaceNew();
    assert(space != NULL);
    space->iterations = 20;
    cpSpaceResizeStaticHash(space, 40.0, 1000);
    cpSpaceResizeActiveHash(space, 40.0, 1000);
    space->gravity = cpv(0,100);

    delta = 1.0f/60.0f/(cpFloat)steps;
  }
}

void Physics::Update() {
  for(int i=0; i<steps; i++){
    cpSpaceStep(space, delta);
  }
}

void Physics::HashQuery(void* ptr, void* data) { 
  cpShape* shape = static_cast<cpShape*>(ptr);
  PhysicsObject obj(shape->body, shape);
  assert(listener != NULL);
  listener->DrawPhysicsObject(&obj, static_cast<Graphics*>(data));
}

void Physics::Draw(Graphics* g) {
  cpSpaceHashEach(space->activeShapes, &HashQuery, g);
  cpSpaceHashEach(space->staticShapes, &HashQuery, g);
}

void Physics::SetGravity(const SexyVector2& gravity) {
  assert(space != NULL);
  space->gravity = cpv(gravity.x, gravity.y);
}

void Physics::SetDamping(cpFloat damping) {
  assert(space != NULL);
  space->damping = damping;
}

void Physics::DestroyObject(PhysicsObject** object) {
  delete *object;
  *object = NULL;
}

PhysicsObject* Physics::CreateObject(cpFloat mass, cpFloat inertia) {
  if (!IsInitialized())
    return NULL;
 return new PhysicsObject(mass, inertia, space);
}

PhysicsObject* Physics::CreateStaticObject() {
  return new PhysicsObject(INFINITY, INFINITY, space, true);
}

