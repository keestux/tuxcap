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

  physics->SetIterations(5);
  physics->SetSteps(1);
  physics->ResizeStaticHash(40.0f,999);
  physics->ResizeActiveHash(30.0f,2999);

  int num = 5;
  SexyVector2 verts[5];
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
                        obj->AddPolyShape(3, tris, offset, 1.0f, 1.0f);
                }
        }

        for(i=0; i<300; i++){
          PhysicsObject* obj = physics->CreateObject(1.0f, physics->ComputeMomentForPoly(1.0f, num, verts, SexyVector2(0.0f,0.0f)));
          cpFloat x = rand()/(cpFloat)RAND_MAX*640;
          obj->AddPolyShape(num, verts, SexyVector2(0.0f,0.0f), 0.0f,0.4f);
          obj->SetPosition(SexyVector2(x, -110));
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

  physics->Draw(g);
}

void Board::DrawPhysicsObject(PhysicsObject* object, Graphics* g) {
  g->SetColor(Color(128,128,128)); 

  if (object->GetShapeType() == object->SEGMENT_SHAPE) {

    SexyVector2 startpos = object->GetSegmentShapeBegin(); 
    SexyVector2 endpos = object->GetSegmentShapeEnd();
    g->DrawLineAA((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);

  }
  else if (object->GetShapeType() == object->POLY_SHAPE) {

    int num = object->GetNumberVertices();

    for (int i = 0; i < num - 1; ++i) {
      SexyVector2 startpos = object->GetVertex(i); 
      SexyVector2 endpos = object->GetVertex(i + 1);
      g->DrawLineAA((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
    }

    SexyVector2 startpos = object->GetVertex(num - 1);
    SexyVector2 endpos = object->GetVertex(0);
    g->DrawLineAA((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
  }

  g->SetColor(Color(0,0,255));
  SexyVector2 pos = object->GetPosition();
  g->DrawLineAA((int)pos.x, (int)pos.y, (int)pos.x + 1,(int)pos.y);
}

void Board::AfterPhysicsStep(){
 
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
    physics->Clear();
    physics->Init();
    InitDemo();
  }
}

