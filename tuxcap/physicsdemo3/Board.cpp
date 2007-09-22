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
  physics->ResizeStaticHash(200.0f, 99);
  physics->ResizeActiveHash(30.0f, 999);
  physics->SetGravity(SexyVector2(0,400));

  int num = 4;
  SexyVector2 verts[] = {
    SexyVector2(-30,-15),
    SexyVector2(-30, 15),
    SexyVector2( 30, 15),
    SexyVector2( 30,-15)
  };
        
  SexyVector2 a = SexyVector2(-200,-200);
  SexyVector2 b = SexyVector2(-200,200);
  SexyVector2 c = SexyVector2(200, 200);
  SexyVector2 d = SexyVector2(200, -200);

  PhysicsObject* obj = physics->CreateStaticObject();
  obj->AddSegmentShape(a, b, 0.0f,1.0f,1.0f);
  obj->SetAngularVelocity(-0.4f);
  object1 = obj;

  obj = physics->CreateStaticObject();
  obj->AddSegmentShape(b, c, 0.0f,1.0f,1.0f);
  obj->SetAngularVelocity(-0.4f);
  object2 = obj;
        
  obj = physics->CreateStaticObject();
  obj->AddSegmentShape(c, d, 0.0f,1.0f,1.0f);
  obj->SetAngularVelocity(-0.4f);
  object3 = obj;
        
  obj = physics->CreateStaticObject();
  obj->AddSegmentShape(d, a, 0.0f,1.0f,1.0f);
  obj->SetAngularVelocity(-0.4f);
  object4 = obj;
        
  int i,j;
  for(i=0; i<3; i++){
    for(j=0; j<7; j++){
      PhysicsObject* obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
      obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f),0.0f,0.7f);
      obj->SetPosition(SexyVector2(i*60 -150, j*30 -150));
    }
  }
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

  //draw saved collision points

  g->SetColor(Color(255,0,0)); 
  std::vector<SexyVector2>::const_iterator it = points.begin();
  while (it != points.end()) {
    SexyVector2 pos = (*it) + SexyVector2(320,240);
      g->DrawLine((int)pos.x - 2, (int)pos.y, (int)pos.x + 2, (int)pos.y);
      g->DrawLine((int)pos.x, (int)pos.y - 2, (int)pos.x, (int)pos.y + 2);
    ++it;
  }
  points.clear();
}

void Board::DrawPhysicsObject(PhysicsObject* object, Graphics* g) {
  g->SetColor(Color(128,128,128)); 
  
  if (object->GetShapeType() == object->SEGMENT_SHAPE) {

    SexyVector2 startpos = object->GetSegmentShapeBegin() + SexyVector2(320,240);
    SexyVector2 endpos = object->GetSegmentShapeEnd() + SexyVector2(320,240);
    g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);

  }
  else if (object->GetShapeType() == object->POLY_SHAPE) {

    int num = object->GetNumberVertices();

    for (int i = 0; i < num - 1; ++i) {
      SexyVector2 startpos = object->GetVertex(i) + SexyVector2(320,240); 
      SexyVector2 endpos = object->GetVertex(i + 1) + SexyVector2(320,240);
      g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
    }

    SexyVector2 startpos = object->GetVertex(num - 1) + SexyVector2(320,240);
    SexyVector2 endpos = object->GetVertex(0) + SexyVector2(320,240);
    g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
  }
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

/* watch it!, this is being called for every step in which a collision occurres, so in case of physics->SetSteps(3) it might get called 3 times by every Board->Update(). */
void Board::HandleCollision(CollisionObject* col) {

  /* Watch it! col->points and col are invalid when this function exits, so if you want to store its data, copy it!
     col->object1 and col->object2 are not invalidated upon return */

  for (int i = 0; i < col->num_points; ++i) {
    points.push_back(SexyVector2(col->points[i].point.x, col->points[i].point.y));
  }
}

void Board::AfterPhysicsStep(){
  object1->UpdatePosition();
  object2->UpdatePosition();
  object3->UpdatePosition();
  object4->UpdatePosition();
}

void Board::KeyDown(KeyCode theKey) {
  if (theKey == KEYCODE_SPACE) {
    points.clear();
    physics->Clear();
    physics->Init();
    InitDemo();
  }
}

