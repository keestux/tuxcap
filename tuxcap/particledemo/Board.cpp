/* W.P. van Paassen 2008 */

#include "Board.h"
#include "Graphics.h"
#include "DDImage.h"
#include "SexyAppBase.h"
#include "Color.h"
#include "Physics.h"

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
        physics->Init();
        physics->SetPhysicsListener(this);
        physics->SetSteps(2);
        physics->ResizeStaticHash(50.0f,100);
        physics->ResizeActiveHash(5.0f,2000);

        //left vulcano slope
        PhysicsObject* obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(300,240), SexyVector2(0,480 ), 0.6f,0.6f,1.0f);
        obj->SetCollisionType(2);

        //right vulcano slope
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(340,240), SexyVector2(640, 480), 0.6f,0.6f,1.0f);
        obj->SetCollisionType(2);

        //left wall
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(60,0), SexyVector2(60, 480), 0.9f,0.6f,1.0f);
        obj->SetCollisionType(2);

        //right wall
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(580,0), SexyVector2(580, 480), 0.9f,0.6f,1.0f);
        obj->SetCollisionType(2);

        //left platform
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(290,400), SexyVector2(300, 400), 0.0f,0.0f,1.0f);
        obj->SetCollisionType(3);

        //right platform
        obj = physics->CreateStaticObject();
        obj->AddSegmentShape(SexyVector2(340,400), SexyVector2(350, 400), 0.0f,0.0f,1.0f);
        obj->SetCollisionType(3);

        DDImage* sprite = (DDImage*) gSexyAppBase->GetImage("images/particle.png");

        //creating the lava particle system with physics enabled
        hgeParticleSystem* p = gSexyAppBase->mParticleManager->SpawnPS("images/particle10.psi", sprite, 320,240, false, false, physics);
        
        p->SetCollisionType(4); //This id is used to set the type of the particlesystem which can be used to register collisions between objects of the physics engine, see the
        //example below in which collisions between the lava particles and the platforms are registered and handled in HandleTypedCollision

        //p->SetCollisionGroup(1); //this assigns all particles in this particle system to the same group, meaning they do not collide with each other and with other
        //particle systems with the same group id. To make the particles in a particlesystem collide with each other, set this to 0

        //p->SetScale(2.0f);
        
        ///same particle system but no physics
        //gSexyAppBase->mParticleManager->SpawnPS("images/particle10.psi", sprite, 320,240,  false, false);

        //alternatives to create a particle system, use this if you're gonna  reuse the same particlesystem to avoid loading it.
        //the pro of declaring a hgeParticleSystem is that you can use the system to spawn both physics enabled and physics disabled particle systems from it

        //hgeParticleSystem* system = new hgeParticleSystem("images/particle10.psi",sprite, 0.0f, false, false);

        //creates a particle system without physics
        //gSexyAppBase->mParticleManager->SpawnPS(system, 320,400);
        
        //creates a particle system with physics
        //gSexyAppBase->mParticleManager->SpawnPS(system, 320,400, physics);
        
        //it is also possible to use the ParticlePhysicsSystem class directly but you can only spawn physics enabled particle systems from it

        //ParticlePhysicsSystem* p_system = new ParticlePhysicsSystem("images/particle10.psi",sprite, physics, 0.0f, false, false);

        //creates a particle system with physics
        //gSexyAppBase->mParticleManager->SpawnPS(p_system, 320,400);
        
        //creates a particle system with physics
        //gSexyAppBase->mParticleManager->SpawnPS(p_system, 320,400, physics);
       
        //explosion system which is used to spawn a new particle system from,  this is done whenever the lava hits the platforms, see the HandleTypedCollision function.
        //this system does not have physics enabled
        explosion= new hgeParticleSystem("images/particle6.psi",sprite, 0.0f, false, false);
        explosion->SetParticleScale(2.0f);//scales the particles in the explosion particle system
        explosion->SetScale(2.0f);//scales the explosion particle system itself

        //register collisions between lava and platforms, to be handled inHandleTypedCollision
        physics->RegisterCollisionType(p->GetCollisionType(), 3); 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Board::~Board()
{
  physics->UnregisterCollisionType(4,3); 
  delete physics;
  delete explosion;
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

        gSexyAppBase->mParticleManager->Update(0.01f);

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
	// And now for the good stuff! The Graphics object, "g", is 
	// automatically created and passed to this method by the 
	// WidgetManager and can be thought of as the main screen 
	// bitmap/canvas upon which all drawing will be done. This object
	// is double buffered automatically so you don't need to worry
	// about those details. All you need to do is instruct the object
	// that you would like to draw something to it, and when the
	// WidgetManager gets done letting all widgets draw to the
	// Graphics object, it will then blit everything to the screen
	// at once. 

	// First, let's start by drawing some geometric primitives. By
	// default, the drawing color is black. We will change it later
	// on. The first command clears the screen by drawing a 
	// black rectangle (black due to the default color) that is
	// located at coordinate 0, 0 and is the same size as our 
	// Board widget, which is the same size as the application and
	// thus is the same size of the game window.
	g->FillRect(0, 0, mWidth, mHeight);

        physics->Draw(g);
        
        gSexyAppBase->mParticleManager->Render(g);
}

void Board::KeyDown(KeyCode theKey) {
  if (theKey == KEYCODE_ESCAPE) {
    gSexyAppBase->Shutdown();
  }
}

//draws the lines of the slopes, walls and platforms
void Board::DrawPhysicsObject(PhysicsObject* object, Graphics* g) {  

  g->SetColor(Color(128,128,128)); 
  SexyVector2 pos = object->GetPosition();
  if (object->GetShapeType() == object->SEGMENT_SHAPE) {

    SexyVector2 startpos = object->GetSegmentShapeBegin(); 
    SexyVector2 endpos = object->GetSegmentShapeEnd();
    g->DrawLine((int)startpos.x, (int)startpos.y, (int)endpos.x,(int)endpos.y);
  }
}

/* watch it!, this is being called for every step in which a collision occurres, so in case of physics->SetSteps(3) it might get called 3 times in every Board->Update(). */
void Board::HandleTypedCollision(CollisionObject* col){

  /* Watch it! col->points and col are invalid when this function exits, so if you want to store its data, copy it!
     col->object1 and col->object2 are not invalidated upon return */

  /* collision between platform and lava */
  SexyVector2 p = col->points[0].point;
  gSexyAppBase->mParticleManager->SpawnPS(explosion, p.x,p.y);
}
