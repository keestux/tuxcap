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

PhysicsObject::~PhysicsObject() {}

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
  assert(sizeof(SexyVector2) == sizeof(cpVect));

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

void PhysicsObject::SetCollisionType(int type) {
  assert(shape != NULL);
  shape->collision_type = type;
}

int PhysicsObject::GetCollisionType() {
  assert(shape != NULL);
  return shape->collision_type;
}

void PhysicsObject::SetFriction(cpFloat u) {
  assert(shape != NULL);
  shape->u = u;
}

int PhysicsObject::GetNumberVertices() {
  assert(shape_type == POLY_SHAPE);
  return ((cpPolyShape*)shape)->numVerts;
}

SexyVector2 PhysicsObject::GetVertex(int index) {
  assert(shape_type == POLY_SHAPE);
  cpVect position = cpvadd(body->p, cpvrotate(((cpPolyShape*)shape)->verts[index], body->rot));
  return SexyVector2(position.x, position.y);
}

SexyVector2 PhysicsObject::GetSegmentShapeBegin() {
  assert (shape_type == SEGMENT_SHAPE);
  cpVect position = cpvadd(body->p, cpvrotate(((cpSegmentShape*)shape)->a, body->rot));
  return SexyVector2(position.x, position.y);  
}  

SexyVector2 PhysicsObject::GetSegmentShapeEnd() {
  assert(shape_type == SEGMENT_SHAPE);
  cpVect position = cpvadd(body->p, cpvrotate(((cpSegmentShape*)shape)->b, body->rot));
  return SexyVector2(position.x, position.y);  
}

float PhysicsObject::GetSegmentShapeRadius() {
  assert (shape_type == SEGMENT_SHAPE);
  return (float)((cpSegmentShape*)shape)->r;
}

float PhysicsObject::GetCircleShapeRadius() {
  assert(shape_type == CIRCLE_SHAPE);
  return (float)((cpCircleShape*)shape)->r;    
}

SexyVector2 PhysicsObject::GetCircleShapeCenter() {
  assert(shape_type == CIRCLE_SHAPE);
  cpVect position = cpvadd(body->p, cpvrotate(((cpCircleShape*)shape)->c, body->rot));     //FIXME does c stand for center of gravity??
  return SexyVector2(position.x, position.y);    
}

Physics::Physics():space(NULL),steps(1){
    cpInitChipmunk();
}

Physics::~Physics() {
  Clear();
}

void Physics::Init() {
  if (space == NULL) {
    cpResetShapeIdCounter();
    space = cpSpaceNew();
    assert(space != NULL);

    cpSpaceSetDefaultCollisionPairFunc(space, NULL, NULL);

    space->gravity = cpv(0,100);

    delta = 1.0f/60.0f/(cpFloat)steps;
  }
}

void Physics::SetSteps(int steps) {
  this->steps = steps;
  delta = 1.0f/60.0f/(cpFloat)steps;
}

int Physics::CollFunc(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, void *data) {
  assert(listener != NULL);

  PhysicsObject obj1(a->body, a);
  PhysicsObject obj2(b->body, b);

  std::vector<CollisionPoint> points;
  for (int i = 0; i < numContacts; ++i) {
    points.push_back(CollisionPoint(SexyVector2(contacts[i].p.x, contacts[i].p.y), 
                                    SexyVector2(contacts[i].n.x, contacts[i].n.y),contacts[i].dist));
  }
  CollisionObject col(obj1, obj2, points); 
  listener->HandleTypedCollision(&col);
}

void Physics::AllCollisions(void* ptr, void* data) { 
  assert(listener != NULL && ptr != NULL);
  cpArbiter *arb = static_cast<cpArbiter*>(ptr);

  //cpVect sum_impulse = cpContactsSumImpulses(arb->contacts, arb->numContacts);
  //cpVect sum_impulse_with_friction = cpContactsSumImpulsesWithFriction(arb->contacts, arb->numContacts);

  PhysicsObject obj1(arb->a->body, arb->a);
  PhysicsObject obj2(arb->b->body, arb->b);

  std::vector<CollisionPoint> points;
  for (int i = 0; i < arb->numContacts; ++i) {
    points.push_back(CollisionPoint(SexyVector2(arb->contacts[i].p.x, arb->contacts[i].p.y), 
                                    SexyVector2(arb->contacts[i].n.x, arb->contacts[i].n.y),arb->contacts[i].dist));
  }
  CollisionObject col(obj1, obj2, points); 
  listener->HandleCollision(&col);
}

void Physics::HashQuery(void* ptr, void* data) { 
  assert(listener != NULL && ptr != NULL);
  cpShape* shape = static_cast<cpShape*>(ptr);
  PhysicsObject obj(shape->body, shape);
  listener->DrawPhysicsObject(&obj, static_cast<Graphics*>(data));
}

void Physics::Update() {
  for(int i=0; i<steps; i++){
    cpSpaceStep(space, delta);
  }

  cpArrayEach(space->arbiters, &AllCollisions, NULL);
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

void Physics::SetIterations(int iter) {
  assert(space != NULL);
  space->iterations = iter;
}

void Physics::DestroyObject(PhysicsObject* object) { //TODO delete too and remove from vector
  if (object->shape != NULL) {
    if (object->is_static)
      cpSpaceRemoveStaticShape(space, object->shape);
    else 
      cpSpaceRemoveShape(space, object->shape);    
  }
  if (object->shape != NULL)
    cpSpaceRemoveBody(space, object->body);
}

void Physics::ResizeStaticHash(float dimension, int count) {
  assert(space != NULL);
  cpSpaceResizeStaticHash(space, dimension, count);
}

void Physics::ResizeActiveHash(float dimension, int count) {
  assert(space != NULL);
  cpSpaceResizeStaticHash(space, dimension, count);
}

void Physics::Clear() {
  //TODO store physicsobjects in list/vector to clean up and avoid memory leaks
  cpSpaceFreeChildren(space);
  cpSpaceFree(space);
  space = NULL;
}

PhysicsObject* Physics::CreateObject(cpFloat mass, cpFloat inertia) {
  if (!IsInitialized())
    return NULL;
 return new PhysicsObject(mass, inertia, space);
}

PhysicsObject* Physics::CreateStaticObject() {
  return new PhysicsObject(INFINITY, INFINITY, space, true);
}

void Physics::RegisterCollisionType(unsigned long type_a, unsigned long type_b, void* data) {
  cpSpaceAddCollisionPairFunc(space, type_a, type_b, &CollFunc, data);
}

void Physics::UnregisterCollisionType(unsigned long type_a, unsigned long type_b) {
  cpSpaceRemoveCollisionPairFunc(space, type_a, type_b);
}
