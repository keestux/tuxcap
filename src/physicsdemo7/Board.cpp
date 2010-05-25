/* W.P. van Paassen */

#include "Board.h"
#include "GameApp.h"
#include "Graphics.h"
#include "SexyVector.h"

// See the Draw method for more information on using the Color class.
#include "Color.h"

// We're going to add our own button widget, which requires knowing about the
// WidgetManager.
#include "WidgetManager.h"

// The SexyAppFramework resides in the "Sexy" namespace. As a convenience,
// you'll see in all the .cpp files "using namespace Sexy" to avoid
// having to prefix everything with Sexy::
using namespace Sexy;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Board::Board(GameApp* theApp)
{
  mApp = theApp;

  physics = new Physics();
  physics->SetPhysicsListener(this);
  physics->Init();

  InitDemo();
}

void Board::InitDemo() {

  physics->SetSteps(3);
  physics->ResizeStaticHash(50.0f,999);
  physics->ResizeActiveHash(50.0f,999);
  physics->SetGravity(SexyVector2(0,300));

  PhysicsObject* obj = physics->CreateStaticObject();
  obj->AddSegmentShape(SexyVector2(0,470), SexyVector2(640, 470), 0.0f,1.0f,1.0f);

  obj = physics->CreateStaticObject();
  obj->AddSegmentShape(SexyVector2(0,325), SexyVector2(250, 490), 0.0f,1.0f,1.0f);

  obj = physics->CreateStaticObject();
  obj->AddSegmentShape(SexyVector2(300,495), SexyVector2(660, 375), 0.0f,1.0f,1.0f);

  obj = physics->CreateStaticObject();
  obj->AddSegmentShape(SexyVector2(640,0), SexyVector2(640, 480), 0.0f,1.0f,1.0f);

  PhysicsObject* static_obj = physics->CreateStaticObject();
  PhysicsObject* static_obj2 = physics->CreateStaticObject();
  static_obj2->SetPosition(SexyVector2(440,200));
  PhysicsObject* obj1 = MakeBox(SexyVector2(460,200));
  PhysicsObject* obj2 = MakeBox(SexyVector2(500,200));
  PhysicsObject* obj3 = MakeBox(SexyVector2(540,200));
  PhysicsObject* obj4 = MakeBox(SexyVector2(580,200));

  physics->CreatePivotJoint(static_obj2, obj1, SexyVector2(440,200));
  physics->CreatePivotJoint(static_obj2, obj2, SexyVector2(440,200));
  physics->CreatePivotJoint(static_obj2, obj3, SexyVector2(440,200));
  physics->CreatePivotJoint(static_obj2, obj4, SexyVector2(440,200));

  obj1 = MakeBox(SexyVector2(220,180));
  obj2 = MakeBox(SexyVector2(260,180));
  obj3 = MakeBox(SexyVector2(300,180));
  obj4 = MakeBox(SexyVector2(340,180));

  float max = 25.0f;
  float min = 10.0f;
  physics->CreateSlideJoint(static_obj, obj1, SexyVector2(195,180), SexyVector2(-15,0), min,max);
  physics->CreateSlideJoint(obj1, obj2, SexyVector2(15,0), SexyVector2(-15,0), min,max);
  physics->CreateSlideJoint(obj2, obj3, SexyVector2(15,0), SexyVector2(-15,0), min,max);
  physics->CreateSlideJoint(obj3, obj4, SexyVector2(15,0), SexyVector2(-15,0), min,max);
  physics->CreateSlideJoint(obj4, static_obj, SexyVector2(15,0), SexyVector2(355,180), min,max);

  obj1 = MakeBox(SexyVector2(320,10));
  obj2 = MakeBox(SexyVector2(360,10));
  obj3 = MakeBox(SexyVector2(400,10));
  obj4 = MakeBox(SexyVector2(440,10));

  physics->CreatePinJoint(static_obj, obj1, SexyVector2(195,10), SexyVector2(-15,0));
  physics->CreatePinJoint(obj1, obj2, SexyVector2(15,0), SexyVector2(-15,0));
  physics->CreatePinJoint(obj2, obj3, SexyVector2(15,0), SexyVector2(-15,0));
  physics->CreatePinJoint(obj3, obj4, SexyVector2(15,0), SexyVector2(-15,0));
  physics->CreatePinJoint(obj4, static_obj, SexyVector2(15,0), SexyVector2(355,10));

  int num = 4;
  SexyVector2 verts[] = {
    SexyVector2(-20,-15),
    SexyVector2(-20, 15),
    SexyVector2( 20, 15),
    SexyVector2( 20,-15),
  };

  chassis = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
  chassis->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f),0.0f,1.0f);
  chassis->SetPosition(SexyVector2(60, 250));

  float radius = 15;
  float wheel_mass = 0.3;
  SexyVector2 offset = SexyVector2(radius + 30, 25);

  wheel1 = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(wheel_mass, 0.0f, radius, SexyVector2(0.0f,0.0f)));
  wheel1->SetPosition(chassis->GetPosition() + offset);
  wheel1->AddCircleShape(radius, SexyVector2(0,0),0.0f,2.5f);
  wheel1->SetVelocity(chassis->GetVelocity());

  physics->CreatePinJoint(chassis, wheel1, SexyVector2(0,0), SexyVector2(0,0));

  wheel2 = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(wheel_mass, 0.0f, radius, SexyVector2(0.0f,0.0f)));
  wheel2->SetPosition(chassis->GetPosition() + SexyVector2(-offset.x, offset.y));
  wheel2->AddCircleShape(radius, SexyVector2(0,0),0.0f,2.5f);
  wheel2->SetVelocity(chassis->GetVelocity());

  physics->CreatePinJoint(chassis, wheel2, SexyVector2(0,0), SexyVector2(0,0));
}

PhysicsObject* Board::MakeBox(const SexyVector2& position) 
{
  int num = 4;
  SexyVector2 verts[] = {
    SexyVector2(-15,-7),
    SexyVector2(-15, 7),
    SexyVector2( 15, 7),
    SexyVector2( 15,-7)
  };

  PhysicsObject* obj;
  obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
  obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f),0.0f,1.0f);
  obj->SetPosition(position);

  return obj;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Board::~Board()
{
  delete physics;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void Board::Update()
{
  // Let the parent class update as well. This will increment
  // the variable mUpdateCnt which is an integer that indicates 
  // how many times the Update() method has been called. Since our
  // Board class is updated 100 times per second, this variable will
  // increment 100 times per second. As you will see in later demos,
  // we will use this variable for animation since its value represents
  // hundredths of a second, which is for almost all games a good
  // enough timer value and doesn't rely on the system clock function
  // call.
  Widget::Update();

  physics->Update();
	
  // For this and most of the other demos, you will see the function
  // below called every Update() call. MarkDirty() tells the widget
  // manager that something has changed graphically in the widget and
  // that it needs to be repainted. All widgets follow this convention.
  //		In general, if you don't need
  // to update your drawing every time you call the Update method
  // (the most common case is when the game is paused) you should
  // NOT mark dirty. Why? If you aren't marking dirty every frame,
  // then you aren't drawing every frame and thus you use less CPU
  // time. Because people like to multitask, or they may be on a laptop
  // with limited battery life, using less CPU time lets people do
  // other things besides play your game. Of course, everyone
  // will want to play your game at all times, but it's good to be
  // nice to those rare people that might want to read email or
  // do other things at the same time. 
  //		In this particular demo, we
  // won't be nice, as the purpose is to bring you up to speed as
  // quickly as possible, and so we'll dispense with optimizations
  // for now, so you can concentrate on other core issues first.
  //		In general, this is the last method called in the Update
  // function, but that is not necessary. In fact, the MarkDirty
  // function can be called anywhere, in any method (although
  // calling it in the Draw method doesn't make sense since it is
  // already drawing) and even multiple times. Calling it multiple
  // times does not do anything: only the first call makes a difference.
  MarkDirty();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void Board::Draw(Graphics* g)
{
  // The Graphics object, "g", is 
  // automatically created and passed to this method by the 
  // WidgetManager and can be thought of as the main screen 
  // bitmap/canvas upon which all drawing will be done. This object
  // is double buffered automatically so you don't need to worry
  // about those details. All you need to do is instruct the object
  // that you would like to draw something to it, and when the
  // WidgetManager gets done letting all widgets draw to the
  // Graphics object, it will then blit everything to the screen
  // at once. 

  // First, let's start by clearing the screen to black. 
  // As you'll recall from Demo1, we set the color with SetColor
  // and pass in a Color object that contains either 3 or 4 parameters,
  // that represent the r,g,b,a values (alpha is 255 if only 3 specified).

  g->SetColor(Color(255, 255, 255));
  g->FillRect(0, 0, mWidth, mHeight);

  // Now let's try drawing a stretched image. We'll draw the original image
  // stretched to twice its size. Drawing a stretched image is exactly like
  // drawing a normal image, except that you have two extra parameters:
  // the stretched width and height. You can use this to draw a shrunk version
  // of the image as well (which we'll do second)	

  physics->Draw(g);

  //draw all joints
  g->SetColor(Color(0,0,255)); 
  std::vector<std::pair<SexyVector2, SexyVector2> > j = physics->GetJoints();
  std::vector<std::pair<SexyVector2, SexyVector2> >::const_iterator jit = j.begin();
    while (jit != j.end()) {      
      SexyVector2 start = (*jit).first;
      SexyVector2 end = (*jit).second;
      g->DrawLine((int)start.x, (int)start.y, (int)end.x, (int)end.y);
      ++jit;
    }

  //draw saved collision points

  g->SetColor(Color(255,0,0)); 
  std::vector<SexyVector2>::const_iterator it = points.begin();
  while (it != points.end()) {
    SexyVector2 pos = (*it);
      g->DrawLine((int)pos.x - 2, (int)pos.y, (int)pos.x + 2, (int)pos.y);
      g->DrawLine((int)pos.x, (int)pos.y - 2, (int)pos.x, (int)pos.y + 2);
    ++it;
  }
  points.clear();
}

void Board::HandleCollision(CollisionObject* col) {

  //Watch it!! col pointer and its object are invalid when this function exits, so if you want to store its data, copy it!

  for (int i = 0; i < col->num_points; ++i) {
    points.push_back(SexyVector2(col->points[i].point.x, col->points[i].point.y));
  }
}

void Board::DrawPhysicsObject(PhysicsObject* object, Graphics* g) {
  
  g->SetColor(Color(128,128,128)); 
  SexyVector2 pos = object->GetPosition();

  if (object->GetShapeType() == object->CIRCLE_SHAPE) {

    float radius =  object->GetCircleShapeRadius();
    SexyVector2 center = object->GetCircleShapeCenter();
    float angle = object->GetAngle();

    const int segs = 15;
    const float coef = 2.0f*M_PI/(float)segs;        

    for(int n = 0; n < segs - 1; n++){
      float rads_begin = n*coef;
      float rads_end = (n+1)*coef;

      g->DrawLine((int)(radius*cosf(rads_begin + angle) + center.x), (int)(radius*sinf(rads_begin + angle) + center.y),
                  (int)(radius*cosf(rads_end + angle) + center.x), (int)(radius*sinf(rads_end + angle) + center.y));
    }

    float rads_begin = (segs-1)*coef;
    g->DrawLine((int)(radius*cosf(rads_begin + angle) + center.x), (int)(radius*sinf(rads_begin + angle) + center.y),
                (int)(radius*cosf(angle) + center.x), (int)(radius*sinf(angle) + center.y));
    g->DrawLine((int)(radius*cosf(angle) + center.x), (int)(radius*sinf(angle) + center.y),
                (int)pos.x, (int)pos.y);

  }
  else  if (object->GetShapeType() == object->SEGMENT_SHAPE) {

    SexyVector2 startpos = object->GetSegmentShapeBegin(); 
    SexyVector2 endpos = object->GetSegmentShapeEnd();
    g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);

  }
  else if (object->GetShapeType() == object->POLY_SHAPE) {

    int num = object->GetNumberVertices();

    for (int i = 0; i < num - 1; ++i) {
      SexyVector2 startpos = object->GetVertex(i); 
      SexyVector2 endpos = object->GetVertex(i + 1);
      g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
    }

    SexyVector2 startpos = object->GetVertex(num - 1);
    SexyVector2 endpos = object->GetVertex(0);
    g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
  }

  g->SetColor(Color(0,0,255));
  g->DrawLine((int)pos.x, (int)pos.y, (int)pos.x + 1,(int)pos.y);
}

void Board::BeforePhysicsStep() {
  chassis->ResetForces();
  wheel1->ResetForces();
  wheel2->ResetForces();
  physics->ApplySpringForce(chassis, wheel1, SexyVector2(40,-15), SexyVector2(0,0), 50.0f, 150.0f, 10.0f);
  physics->ApplySpringForce(chassis, wheel2, SexyVector2(-40,-15), SexyVector2(0,0), 50.0f, 150.0f, 10.0f);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void Board::AddedToManager(WidgetManager* theWidgetManager)
{
	// At this point, the Board class has already been added to the
	// widget manager. We should call our parent class' method
	// so that it can be sure to perform any needed tasks, first.
	Widget::AddedToManager(theWidgetManager);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void Board::RemovedFromManager(WidgetManager* theWidgetManager)
{
	// This is called after we've been removed from the widget manager.
	// Again, we should let our base class do anything it needs to, first.
	Widget::RemovedFromManager(theWidgetManager);
}

void Board::KeyDown(KeyCode theKey) {
  if (theKey == KEYCODE_SPACE) {
    points.clear();
    physics->Clear();
    physics->Init();
    InitDemo();
  }
  else if (theKey == KEYCODE_ESCAPE) {
    exit(0);
  }
}

