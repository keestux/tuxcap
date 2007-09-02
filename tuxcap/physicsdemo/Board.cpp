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
        current = 0;

        physics = new Physics();
        physics->SetPhysicsListener(this);
        physics->Init();      

        InitDemo7();
        current = 6;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Board::~Board()
{
  delete physics;
}

void Board::InitDemo1() {

  physics->SetSteps(2);
  physics->ResizeStaticHash(20.0f,999);

        PhysicsObject* obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(0,475), SexyVector2(625, 475), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);

        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(635,0), SexyVector2(635, 475), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);

        int num = 4;
        SexyVector2 verts[] = {
          SexyVector2(-15,-15),
          SexyVector2(-15, 15),
          SexyVector2( 15, 15),
          SexyVector2( 15,-15),
        };

        for(int i=0; i<47; i++){
                int j = i + 1;

                SexyVector2  a(i*10, i*10);
                SexyVector2 b(j*10, i*10);
                SexyVector2  c(j*10, j*10);
                
                PhysicsObject* obj = physics->CreateStaticObject();
                obj->AddSegmentShape(a, b, 0.0f);
                obj->SetElasticity(1.0f); 
                obj->SetFriction(1.0f);

                obj = physics->CreateStaticObject();
                obj->AddSegmentShape(b, c, 0.0f);
                obj->SetElasticity(1.0f); 
                obj->SetFriction(1.0f);
        }

        obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
        obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
        obj->SetPosition(SexyVector2(40, 0));
        obj->SetElasticity(0.0f); 
        obj->SetFriction(1.5f);
}


void Board::InitDemo2() {

  physics->SetIterations(20);
  physics->SetSteps(1);
  physics->ResizeStaticHash(40.0f,1000);
  physics->ResizeActiveHash(40.0f,1000);

        PhysicsObject* obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(0,470), SexyVector2(640, 470), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);

        float radius = 15.0f;
        obj = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(10.0f, 0.0f, radius, SexyVector2(0.0f,0.0f)));
        obj->SetPosition(SexyVector2(320,  455));
        obj->AddCircleShape(radius, SexyVector2(0,0));
        obj->SetElasticity(0.0f); 
        obj->SetFriction(0.9f);

        int num = 4;
        SexyVector2 verts[] = {
                SexyVector2(-15,-15),
                SexyVector2(-15, 15),
                SexyVector2( 15, 15),
                SexyVector2( 15,-15),
        };

        int i,j;
        for(i=0; i<14; i++){
                for(j=0; j<=i; j++){
                  PhysicsObject* obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                  obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                  obj->SetPosition(SexyVector2(300 + j*32 - i*16,  i*32));
                  obj->SetElasticity(0.0f); 
                  obj->SetFriction(0.4f);
                }
        }
}

void Board::InitDemo3() {

  physics->SetIterations(5);
  physics->SetSteps(1);
  physics->ResizeStaticHash(40.0f,999);
  physics->ResizeActiveHash(30.0f,2999);

  int num = 5;
  SexyVector2 verts[num];
        int i;
        for(i=0; i<num; i++){
                float angle = -2*M_PI*i/((float) num);
                verts[i] = SexyVector2(10*cos(angle), 10*sin(angle));
        }
        
        SexyVector2 tris[] = {
                SexyVector2(-15,-15),
                SexyVector2(  0, 10),
                SexyVector2( 15,-15),
        };

        int j;
        for(i=0; i<9; i++){
                for(j=0; j<6; j++){
                        float stagger = (j%2)*40;
                        SexyVector2 offset = SexyVector2(640 - i*80 + stagger, 480 - j*70);
                        PhysicsObject* obj = physics->CreateStaticObject();
                        obj->AddPolyShape(3, tris, offset);
                        obj->SetElasticity(1.0f); 
                        obj->SetFriction(1.0f);
                }
        }

        for(i=0; i<300; i++){

          PhysicsObject* obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
          cpFloat x = rand()/(cpFloat)RAND_MAX*640;
          obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
          obj->SetPosition(SexyVector2(x, -110));
          obj->SetElasticity(0.0f); 
          obj->SetFriction(0.4f);
        }
}

void Board::InitDemo4() {

  physics->SetSteps(3);
  physics->ResizeStaticHash(200.0f, 99);
  physics->ResizeActiveHash(30.0f, 999);
  physics->SetGravity(SexyVector2(0,200));

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
        obj->AddSegmentShape(a, b, 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);
        obj->SetAngularVelocity(-0.4f);
        object1 = obj;

        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(b, c, 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);
        obj->SetAngularVelocity(-0.4f);
        object2 = obj;
        
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(c, d, 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);
        obj->SetAngularVelocity(-0.4f);
        object3 = obj;
        
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(d, a, 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);
        obj->SetAngularVelocity(-0.4f);
        object4 = obj;
        
        int i,j;
        for(i=0; i<3; i++){
                for(j=0; j<7; j++){

                  PhysicsObject* obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                  obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                  obj->SetPosition(SexyVector2(i*60 + 170, j*30 + 90));
                  obj->SetElasticity(0.0f); 
                  obj->SetFriction(0.7f);
                }
        }
}


void Board::InitDemo5() {

  physics->SetSteps(2);
  physics->SetIterations(20);
  physics->ResizeStaticHash(40.0f, 99);
  physics->ResizeActiveHash(40.0f, 2999);
  physics->SetGravity(SexyVector2(0,300));

  int num = 4;
  SexyVector2 verts[] = {
    SexyVector2(-3,-20),
    SexyVector2(-3, 20),
    SexyVector2( 3, 20),
    SexyVector2( 3,-20)
  };

  PhysicsObject* obj = physics->CreateStaticObject();
  obj->AddSegmentShape(SexyVector2(-200,480), SexyVector2(840, 480), 0.0f);
  obj->SetElasticity(1.0f); 
  obj->SetFriction(1.0f);

  float u = 0.6;
        
  int n = 9;
  int i, j;

  for(i=1; i<=n; i++){
    SexyVector2 offset = SexyVector2(320 + -i*60/2.0f, i*52);                
                for(j=0; j<i; j++){

                  obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                  obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                  obj->SetPosition(SexyVector2(j*60, -8) + offset);
                  obj->SetElasticity(0.0f); 
                  obj->SetFriction(u);

                  obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                  obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                  obj->SetPosition(SexyVector2(j*60,15) + offset);
                  obj->SetAngle(M_PI/2.0f);
                  obj->SetElasticity(0.0f); 
                  obj->SetFriction(u);

                  if(j == (i - 1)) continue;

                  obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                  obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                  obj->SetPosition(SexyVector2(j*60 + 30,21) + offset);
                  obj->SetAngle(M_PI/2.0f);
                  obj->SetElasticity(0.0f); 
                  obj->SetFriction(u);

                }

                obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                obj->SetPosition(SexyVector2(-17, 48) + offset);
                obj->SetElasticity(0.0f); 
                obj->SetFriction(u);

                obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
                obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
                obj->SetPosition(SexyVector2((i - 1) * 60 + 17, 48) + offset);
                obj->SetElasticity(0.0f); 
                obj->SetFriction(u);

        }

  obj->SetAngularVelocity(-1);
  obj->SetVelocity(SexyVector2(20,0));
}

void Board::InitDemo6() {

  physics->SetSteps(3);
  physics->ResizeStaticHash(50.0f,999);
  physics->ResizeActiveHash(50.0f,999);
  physics->SetGravity(SexyVector2(0,300));

        PhysicsObject* obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(0,475), SexyVector2(625, 475), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);

        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(635,0), SexyVector2(635, 475), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);

        int num = 4;
        SexyVector2 verts[] = {
                SexyVector2(-7,-15),
                SexyVector2(-7, 15),
                SexyVector2( 7, 15),
                SexyVector2( 7,-15),
        };

        for(int i=0; i<47; i++){
                int j = i + 1;

                SexyVector2  a(i*10, i*10);
                SexyVector2 b(j*10, i*10);
                SexyVector2  c(j*10, j*10);
                
                PhysicsObject* obj = physics->CreateStaticObject();
                obj->AddSegmentShape(a, b, 0.0f);
                obj->SetElasticity(1.0f); 
                obj->SetFriction(1.0f);

                obj = physics->CreateStaticObject();
                obj->AddSegmentShape(b, c, 0.0f);
                obj->SetElasticity(1.0f); 
                obj->SetFriction(1.0f);
        }

        float moment = physics->ComputeMomentForPoly(1.0, num, verts, SexyVector2(0,-15));
        moment += physics->ComputeMomentForCircle(1.0, 0.0, 25.0, SexyVector2(0,15));

        obj = physics->CreateObject(1.0f, moment);
        obj->AddPolyShape(num, verts, SexyVector2(0.0f,-15.0f));
        obj->SetPosition(SexyVector2(40, 0));
        obj->SetElasticity(0.0f); 
        obj->SetFriction(1.5f);
        obj->SetAngularVelocity(-1.0f);          

        obj->AddCircleShape(25.0, SexyVector2(0,15));
        obj->SetElasticity(0.9f, 1); 
        obj->SetFriction(1.5f, 1);
}

void Board::InitDemo7() {

  physics->SetSteps(3);
  physics->ResizeStaticHash(50.0f,999);
  physics->ResizeActiveHash(50.0f,999);
  physics->SetGravity(SexyVector2(0,300));

  PhysicsObject* obj = physics->CreateStaticObject();
  obj->AddSegmentShape(SexyVector2(0,470), SexyVector2(640, 470), 0.0f);
  obj->SetElasticity(1.0f); 
  obj->SetFriction(1.0f);


        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(0,325), SexyVector2(250, 490), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);

        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(300,495), SexyVector2(660, 375), 0.0f);
        obj->SetElasticity(1.0f); 
        obj->SetFriction(1.0f);


  PhysicsObject* static_obj = physics->CreateStaticObject();
  PhysicsObject* obj1 = MakeBox(SexyVector2(220,200));
  PhysicsObject* obj2 = MakeBox(SexyVector2(260,200));
  PhysicsObject* obj3 = MakeBox(SexyVector2(300,200));
  PhysicsObject* obj4 = MakeBox(SexyVector2(340,200));

  physics->CreatePivotJoint(static_obj, obj1, SexyVector2(200,200));
  physics->CreatePivotJoint(obj1, obj2, SexyVector2(240,200));
  physics->CreatePivotJoint(obj2, obj3, SexyVector2(280,200));
  physics->CreatePivotJoint(obj3, obj4, SexyVector2(320,200));

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
  chassis->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
  chassis->SetPosition(SexyVector2(60, 250));
  chassis->SetElasticity(0.0f); 
  chassis->SetFriction(1.0f);

  float radius = 15;
  float wheel_mass = 0.3;
  SexyVector2 offset = SexyVector2(radius + 30, 25);

  wheel1 = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(wheel_mass, 0.0f, radius, SexyVector2(0.0f,0.0f)));
  wheel1->SetPosition(chassis->GetPosition() + offset);
  wheel1->AddCircleShape(radius, SexyVector2(0,0));
  wheel1->SetVelocity(chassis->GetVelocity());
  wheel1->SetElasticity(0.0f); 
  wheel1->SetFriction(2.5f);

  physics->CreatePinJoint(chassis, wheel1, SexyVector2(0,0), SexyVector2(0,0));

  wheel2 = physics->CreateObject(10.0f, physics->ComputeMomentForCircle(wheel_mass, 0.0f, radius, SexyVector2(0.0f,0.0f)));
  wheel2->SetPosition(chassis->GetPosition() + SexyVector2(-offset.x, offset.y));
  wheel2->AddCircleShape(radius, SexyVector2(0,0));
  wheel2->SetVelocity(chassis->GetVelocity());
  wheel2->SetElasticity(0.0f); 
  wheel2->SetFriction(2.5f);

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
        obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f));
        obj->SetPosition(position);
        obj->SetElasticity(0.0f); 
        obj->SetFriction(1.0f);

        return obj;
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

        std::vector<std::vector<CollisionPoint> >::const_iterator it = points.begin();
        while (it != points.end()) {
          std::vector<CollisionPoint>::const_iterator inner_it = (*it).begin();
          while (inner_it != (*it).end()) {
            SexyVector2 pos = (*inner_it).point;
            g->DrawLine((int)pos.x - 2, (int)pos.y, (int)pos.x + 2, (int)pos.y);
            g->DrawLine((int)pos.x, (int)pos.y - 2, (int)pos.x, (int)pos.y + 2);
            ++inner_it;
          }
          ++it;
        }
        points.clear();
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

void Board::HandleCollision(CollisionObject* col) {
  //save collision points
  points.push_back(col->points);
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

void Board::BeforePhysicsStep() {
  if (current == 6) {
          chassis->ResetForces();
          wheel1->ResetForces();
          wheel2->ResetForces();
          physics->ApplySpringForce(chassis, wheel1, SexyVector2(40,-15), SexyVector2(0,0), 50.0f, 150.0f, 10.0f);
          physics->ApplySpringForce(chassis, wheel2, SexyVector2(-40,-15), SexyVector2(0,0), 50.0f, 150.0f, 10.0f);
  }
}

void Board::AfterPhysicsStep(){
        if (current == 2) {

          std::vector<PhysicsObject*> objects = physics->GetPhysicsObjects();
          std::vector<PhysicsObject*>::iterator it = objects.begin();
          
          while (it != objects.end()) {
            if((*it)->GetPosition().y > 520 || (*it)->GetPosition().x > 680 || (*it)->GetPosition().x < -40){
                cpFloat x = rand()/(cpFloat)RAND_MAX*640;
                (*it)->SetPosition(SexyVector2(x, -110));
            }
            ++it;
          }
        }
        else if (current == 3) {
          object1->UpdatePosition();
          object2->UpdatePosition();
          object3->UpdatePosition();
          object4->UpdatePosition();
        }
}

void Board::KeyDown(KeyCode theKey) {
  if (theKey == KEYCODE_SPACE) {
    points.clear();
    physics->Clear();
    physics->Init();

    ++current %= 7;
    if (current == 0)
      InitDemo1();
    else if (current == 1)
      InitDemo2();
    else if (current == 2)
      InitDemo3();
    else if (current == 3)
      InitDemo4();
    else if (current == 4)
      InitDemo5();
    else if (current == 5)
      InitDemo6();
    else if (current == 6)
      InitDemo7();
  }
}


