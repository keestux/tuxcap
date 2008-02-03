# Simple demo game showing the basics of pycap

# This dict provides the basic information required to initialize the application.
# Filling it out is a great way to pretend that you're making progress on a new project.
appIni = {	"mCompanyName"		: "CompanyNameGoesHere",
		"mFullCompanyName"	: "CompanyNameGoesHere",
		"mProdName"		: "Game Name Goes Here",
		"mProductVersion"	: "1.0",
		"mTitle"			: "Window Title Goes Here",
		"mRegKey"			: "RegistryEntryKey",
		"mWidth"			: 800,
		"mHeight"			: 600,
		"mAutoEnable3D"	: 0,
		"mVSyncUpdates"	: 1 }

# The Pycap module (imported here as "PC") gives us access to drawing functions, sound functions, and fun stuff like that.
import Pycap as PC

# The PycapRes module (which we'll import as "PCR" a bit later) gives us access to resource management functions.
# It's used to load images and music and fonts and sounds, and to figure out stuff like image widths and font heights.
PCR = None

# The current state of some of the keyboard keys
leftDown = 0
rightDown = 0
upDown = 0
downDown = 0

# the key codes used by some of the keys in this demo
# if you want to find out the codes for some other keys, just add "print key" to the keydown function, run the game, and hit the key a few times. Then go read out.txt.
KEYLEFT		= 37
KEYRIGHT		= 39
KEYUP		= 38
KEYDOWN		= 40
KEYESC		= 27

# Whether the game should exit now. In this game we set this to true when the escape key is pressed.
doExit = 0

# These are some game objects we'll create in init()
player = None
things = []

# This class represents the player avatar. The engine doesn't require it, or even know about it. I've just added it to make this demo a little more interesting.
class PlayerGuy:
	# This function is called whenever a playerguy is constructed.
	def __init__( self, x, y ):
		# store the x and y parameters as our location
		self.x = x
		self.y = y
	
	# we'll call this from the main update function
	def update( self, delta ):
		# calculate x and y speed from input and scaling factor 1.5 (increase to move faster)
		dx = ( rightDown - leftDown ) * 1.5
		dy = ( downDown - upDown ) *1.5
		
		# apply speed to position for this timestep
		self.x += dx * delta
		self.y += dy * delta
	
	# we'll call this from the main draw function
	def draw( self ):
		# call the draw function. This version takes floating point arguments so the object moves smoothly, rather than jumping one pixel at a time
		# we offset by the width and the height so that the image is drawn with self.x and self.y at its middle.
		PC.drawImageF( self.image, self.x - self.width * 0.5, self.y - self.height * 0.5 )

# This class represents an entity other than the player. It's pretty bare.
class Thing:
	def __init__( self, x, y, a ):
		self.x = x
		self.y = y
		self.a = a # current angle

	def update( self, delta ):
		# spin!
		self.a += delta * 0.03
	
	def draw( self ):
		# to make this a little more interesting we'll tint it and spin it around
		PC.setColourize( 1 )	# enable tinting
		PC.setColour( 255, 45, 45, 127 ) # tint red and semi transparent
		PC.drawImageRotF( self.image, self.x - self.width * 0.5, self.y - self.height * 0.5, self.a )	# draw rotated image
		PC.setColourize( 0 )	# disable tinting

# This function is called first, and is a handy place to load the games resources
def loadBase():
	# PycapRes is ready to be imported now, so we do it here
	import PycapRes
	global PCR
	PCR = PycapRes

	# load an image
	# this image is built from two image files, one for RGB, and another with an underscore for alpha. They can be jpg, png, gif, bmp, whatever. Just make sure they have the same dimensions.
	PlayerGuy.image = PCR.loadImage( "..\\images\\ImageSample" )
	# grab the width and height of the image. We could do this elsewhere, but here'll do.
	PlayerGuy.width = PCR.imageWidth( PlayerGuy.image )
	PlayerGuy.height = PCR.imageHeight( PlayerGuy.image )
	
	# copy the image for use with the Thing class too. I can't be arsed creating more sample data :)
	Thing.image = PlayerGuy.image
	Thing.width = PlayerGuy.width
	Thing.height = PlayerGuy.height

# This is called once on startup, after loadBase and before update or draw
# It's a pretty good place to initialize game data.
def init():
	# create some game objects
	
	# player
	global player
	player = PlayerGuy( 100, 400 )	# create player at location 100, 100
	
	# things
	global things
	for i in range( 10 ):
		things.append( Thing( i * 50, i * 50, i * 0.2 ) ) # create a line of "Thing" objects, one for each i in range 0..9
	
# This is called regularly, and is where you're expected to update game data.
# The parameter "delta" is the amount of time that's passed since this was last called, in centiseconds. I think that's what the unit's called anyway. There's 100 of them every second.
# This means you'll need to use some very basic calculus to ensure things don't speed up or slow down with the framerate.
def update( delta ):
	
	# Tell the engine that we need to call the draw function
	PC.markDirty()

	# update the player
	player.update( delta )
	
	# update the things
	for t in things:
		t.update( delta )

# This is called when the engine wants to redraw the screen.
# All drawing functions must be called from within this function, or the engine will get shirty.
def draw():
	# clear the screen
	PC.setColour( 255, 255, 255, 255 )
	PC.fillRect( 0, 0, 800, 600 )
	
	# draw the player
	player.draw()
	
	# draw the things
	for t in things:
		t.draw()

# This is called every time a key is pressed, with a number corresponding to the key that was pressed.
# To find a key's code, add a "print key" line at the start of this function, run the game, press the key a bunch of times, then quit and read out.txt.
def keyDown( key ):
	# if arrow keys have changed, store them in our global variables
	if key == KEYLEFT:
		global leftDown
		leftDown = 1
	elif key == KEYRIGHT:
		global rightDown
		rightDown = 1
	elif key == KEYUP:
		global upDown
		upDown = 1
	elif key == KEYDOWN:
		global downDown
		downDown = 1
	elif key == KEYESC:
		global doExit
		doExit = 1

# This is just like keydown, but it's called when a key is released.
def keyUp( key ):
	# if arrow keys have changed, store them in our global variables
	if key == KEYLEFT:
		global leftDown
		leftDown = 0
	elif key == KEYRIGHT:
		global rightDown
		rightDown = 0
	elif key == KEYUP:
		global upDown
		upDown = 0
	elif key == KEYDOWN:
		global downDown
		downDown = 0

# This is called by the game every frame to ask if the application should exit. Return 1 here to close the game.
def exitGame():
	return doExit
