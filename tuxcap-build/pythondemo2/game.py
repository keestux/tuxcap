# normal imports
import random, math

###########################################
# Application initialization data
###########################################
appIni = {	"mCompanyName"		: "Farbs",
		"mFullCompanyName"	: "Farbs",
		"mProdName"		: "Petal Hero Prototype",
		"mProductVersion"	: "1.0",
		"mTitle"			: "TuxCap: Petal Hero (Prototype) v1.0",
		"mRegKey"			: "TuxCap\\Pythondemo2",
		"mWidth"			: 800,
		"mHeight"			: 600,
		"mAutoEnable3D"	: 0,
                "mTest3D"		: 1,
		"mVSyncUpdates"	: 1,
                "mWindowIconBMP": "unicron_baby.bmp",
                "mWaitForVSync"      : 1}

doExit = 0 # flag specifying whether or not the game should continue to run (not a hook)
res = None
PC = None
PCR = None

hero = None
planes = []
playerShots = []
enemyShots = []
explosions = []
bestTime = 0
gameTime = 0
firstRun = 1
font = None
squadCount = 0
level = 0

canvasImage = None
backgroundImage = None
skyImage = None

mousex = 0
mousey = 0

nextPlaneTime = 0.0
time = 0.0

PLANEMODE_DELAY		= 0
PLANEMODE_ACTIVE	= 1
PLANEMODE_DONE		= 2

def vToA( x, y, a ):
	"""calculate angle from a 2D vector and default value"""
	if y == 0:
		# dangerous special case (avoids div by 0 in dx/dy)
		if x > 0:
			return math.pi * 0.5
		elif x < 0:
			return -math.pi * 0.5
		else:
			# don't change angle... previous is probably best
			return a
	else:
		# safe to use atan technique
		a = math.atan( x / y )
		if y < 0:
			# inverted
			a += math.pi
		return a

class Hero:
	"""Player 'plane class"""
	image = None
	
	def __init__( self, x, y ):
		self.x = x
		self.y = y
		self.left = 0
		self.right = 0
		self.up = 0
		self.down = 0
		self.radius = PCR.imageWidth( self.image ) * 0.5
		self.lives = 3
		self.lifeTimer = 0
		self.a = 0
		self.dx = 0
		self.dy = 0
	
	def fire( self ):
		global playerShots
		if len( playerShots ) < 6:
			playerShots.append( Shot( self.x + math.cos( self.a ) * 10.0, self.y - math.sin( self.a ) * 10.0, math.sin( self.a ) * 10.0 + self.dx, math.cos( self.a ) * 10.0 + self.dy, 0 ) )
			playerShots.append( Shot( self.x - math.cos( self.a ) * 10.0, self.y + math.sin( self.a ) * 10.0, math.sin( self.a ) * 10.0 + self.dx, math.cos( self.a ) * 10.0 + self.dy, 0 ) )
			# recoil
			#self.dx -= math.sin( self.a ) * 2
			#self.dy -= math.cos( self.a ) * 2
	
	def hit( self, x, y ):
		"""Accept a hit, return whether still alive or not"""
		# ignore if recently hit
		if self.lifeTimer < 0:
			return 1
		
		# test for lives remaining
		if self.lives != 0:
			# damage
			self.lives -= 1
			self.lifeTimer = -100
			# bounce
			n = normScale( x - self.x, y - self.y )
			s = -5.0
			self.dx += n * s * ( x - self.x )
			self.dy += n * s * ( y - self.y )
			return 1
		else:
			# death
			return 0
			
	def update( self, delta ):
		# move
		# calc angle
		self.a = vToA( mousex - self.x, mousey - self.y, self.a )
		
		# drag
		d = pow( 0.98, delta )
		self.dx *= d
		self.dy *= d
		
		# thrust
		s = 0.1
		self.dx += math.sin( self.a ) * s
		self.dy += math.cos( self.a ) * s
		
		# move
		self.x += self.dx * delta
		self.y += self.dy * delta
	
		# bound the 'plane
		if self.x < self.radius:
			self.x = self.radius
		if self.x > 800 - self.radius:
			self.x = 800 - self.radius
		if self.y < self.radius:
			self.y = self.radius
		if self.y > 600 - self.radius:
			self.y = 600 - self.radius
		
		# extra life check
		if self.lives < 3:
			self.lifeTimer += delta
			if self.lifeTimer > 500:
				self.lives += 1
				self.lifeTimer -= 500

	def draw( self ):
		# draw the 'plane sprite
		if self.lifeTimer < 0:
			# damaged
			if ( self.lifeTimer % 5.0 ) < 2.5:
				PC.drawImageRot( self.damageImage, self.x - self.radius, self.y - self.radius, math.pi + self.a )
		else:
			PC.drawImageRot( self.image, self.x - self.radius, self.y - self.radius, math.pi + self.a )
		
		# draw cursor
		PC.setColour( 0, 0, 0, 255 )
		PC.fillRect( mousex - 1, mousey - 1, 3, 3 )
		
class Plane:
	"""Enemy aeroplane class"""
	image = None
	
	def __init__( self, xi, yi, xa, ya, xb, yb, dxi, dyi, dxa, dya, dxb, dyb, pathTime, delay, fireTime, type ):
		self.type = type
		self.radius = PCR.imageWidth( self.image ) * 0.5
		areaWidth = 800 + self.radius * 2
		areaHeight = 600 + self.radius * 2
		
		self.xi = xi * areaWidth - self.radius
		self.yi = yi * areaHeight - self.radius
		self.xa = xa * areaWidth - self.radius
		self.ya = ya * areaHeight - self.radius
		self.xb = xb * areaWidth - self.radius
		self.yb = yb * areaHeight - self.radius
		self.dxi = dxi * areaWidth - self.radius
		self.dyi = dyi * areaHeight - self.radius
		self.dxa = dxa * areaWidth - self.radius
		self.dya = dya * areaHeight - self.radius
		self.dxb = dxb * areaWidth - self.radius
		self.dyb = dyb * areaHeight - self.radius
		self.pathTime = pathTime
		self.time = -delay
		self.fireTime = fireTime
		self.x = self.xi
		self.y = self.yi
		self.a = 0
		self.dx = self.dxi
		self.dy = self.dyi
		
		if delay == 0:
			self.mode = PLANEMODE_ACTIVE
		else:
			self.mode = PLANEMODE_DELAY

	def update( self, delta ):
		# increment timer
		self.time += delta
		
		# move if active
		if self.time >= 0 and self.mode != PLANEMODE_DONE:
			# set to active
			self.mode = PLANEMODE_ACTIVE

			# determine path count, time alpha and blend weight
			t = float( self.time ) / self.pathTime
			a = t%1
			c = int( t )
			b = -math.cos( a * math.pi ) * 0.5 + 0.5

			# record old position
			ox = self.x
			oy = self.y
			
			# set new position based on current path and time in it
			if c == 0:
				# initial path from i to a
				self.x = ( self.xi + a*self.dxi ) * (1-b) + ( self.xa + (a-1)*self.dxa ) * b
				self.y = ( self.yi + a*self.dyi ) * (1-b) + ( self.ya + (a-1)*self.dya ) * b
			elif c % 2 == 1:
				# path from a to b
				self.x = ( self.xa + a*self.dxa ) * (1-b) + ( self.xb + (a-1)*self.dxb ) * b
				self.y = ( self.ya + a*self.dya ) * (1-b) + ( self.yb + (a-1)*self.dyb ) * b
			else:
				# path from b back to a
				self.x = ( self.xb + a*self.dxb ) * (1-b) + ( self.xa + (a-1)*self.dxa ) * b
				self.y = ( self.yb + a*self.dyb ) * (1-b) + ( self.ya + (a-1)*self.dya ) * b
			
			# record estimated velocity ( I'm too lazy for calculus )
			self.dx = ( self.x - ox ) / delta
			self.dy = ( self.y - oy ) / delta
			
			# fire if appropriate
			global enemyShots
			if self.fireTime != 0 and int( ( self.time - delta + self.fireTime / 2.0 ) / self.fireTime ) != int( ( self.time + self.fireTime / 2.0 ) / self.fireTime ):
				n = normScale( self.dx, self.dy )
				if n != 0:
					enemyShots.append( Shot( self.x, self.y, self.dx * n * 4, self.dy * n * 4, 0 ) )
			
			# record current angle
			self.a = dToA( self.dx, self.dy )

	def draw( self ):
		# draw the 'plane sprite if it's active
		if self.mode == PLANEMODE_ACTIVE:
			PC.setColourize( 1 )
			levelAlpha = min( gameTime / 6000.0, 1 )
			PC.setColour( ( 50 + int( 205 * levelAlpha ) )/ 2 , 50, ( 255 - int( 205 * levelAlpha ) ) /2, 255 )
			PC.drawImageRot( self.image, self.x - self.radius, self.y - self.radius, self.a )
			PC.setColourize( 0 )
		
class Shot:
	"""Bullet class"""
	image = None
	
	def __init__( self, x, y, dx, dy, type ):
		self.x = x
		self.y = y
		self.dx = dx
		self.dy = dy
		if dx == 0 and dy == 0:
			self.dy = 1
		self.type = type
		self.radius = PCR.imageWidth( self.image ) * 0.5
		self.done = 0

	def update( self, delta ):
		# move
		self.x += self.dx * delta
		self.y += self.dy * delta
		if self.x < 0 - self.radius or self.x > 800 + self.radius or self.y < 0 - self.radius or self.y > 600 + self.radius:
			self.done = 1

	def draw( self ):
		PC.setColourize( 1 )
		PC.setColour( 0, 0, 0, 255 )
		PC.drawImageF( self.image, self.x - self.radius, self.y - self.radius )
		PC.setColourize( 0 )
	
class Explosion:
	"""Dodgy particle class"""
	image = None
	
	def __init__( self, x, y ):
		self.x = x
		self.y = y
		self.radius = PCR.imageWidth( self.image ) * 0.5
		self.done = 0
		self.lifespan = 100
		self.age = 0

	def update( self, delta ):
		# age
		self.age += delta
		if self.age > self.lifespan:
			self.done = 1

	def draw( self ):
		PC.setColourize( 1 )
		PC.setColour( 255, 255, 255, int( 255 * pow( 1 - self.age / self.lifespan, 1 ) ) )
		s = 150
		PC.drawImageScaled( self.image, self.x - s / 2, self.y - s / 2, s, s )
		PC.setColourize( 0 )

class SquadType:
	"""Basic variables defining a squad type"""
	def __init__( self, difficulty ):
		# assumes random number generator is already seeded and ready for use
		d = difficulty + 1 # shorthand
		
		# set base values
		self.squadSize = int( 1 + random.random() * difficulty )
		self.checkpointTime = 800 / difficulty * ( 1 + random.random() )
		self.spawnTime = 800 / difficulty * ( 1 + random.random() )
		self.fireTime = 400 / difficulty * ( 1 + random.random() )
		
		# choose one value to make more extreme
		switch = int( random.random() * 4 )
		if switch == 0:
			self.squadSize = difficulty * 4
		if switch == 1:
			self.checkpointTime = 100.0 / difficulty
		if switch == 2:
			self.spawnTime = 200 - 150 / difficulty
		if switch == 3:
			self.fireTime = 100 / difficulty
		
		# other values
		self.mirror = int( random.random() * 2 )
		if self.mirror:
			self.spawnTime *= 2
		switch = random.random() * 10
		if switch < 5:
			self.spawnLoc = 0 # front
		elif switch < 8:
			self.spawnLoc = 1 # sides
		else:
			self.spawnLoc = 2 # behind
	
def loadBase():
	# import res module
	import PycapRes
	global PCR
	PCR = PycapRes
	
	# load images
	global canvasImage, backgroundImage, skyImage, explosionImage, tune
	Hero.image = PCR.loadImage( "..\\images\\hero" )
	Hero.damageImage = PCR.loadImage( "..\\images\\heroDamage" )
	Plane.image = PCR.loadImage( "..\\images\\dragonfly" )
	Shot.image = PCR.loadImage( "..\\images\\smallShot" )
	canvasImage = PCR.loadImage( "..\\images\\bigcanvas" )
	backgroundImage = PCR.loadImage( "..\\images\\backgroundbig" )
	skyImage = PCR.loadImage( "..\\images\\sky" )
	Explosion.image = PCR.loadImage( "..\\images\\explosion" )
        tune = PCR.loadTune("..\\music\\m.Mid")
	
	# load font
	global font
	font = PCR.loadFont( "..\\fonts\\Andy28Bold.txt" )


def init():
	# load the pycap module
	import Pycap
	global PC
	PC = Pycap

        global KEYDOWN , KEYESC, KEYLEFT, KEYRIGHT, KEYUP, KEYSHIFT
 
        KEYDOWN = PC.getKeyCode("DOWN")
        KEYUP = PC.getKeyCode("UP")
        KEYLEFT = PC.getKeyCode("LEFT")
        KEYRIGHT = PC.getKeyCode("RIGHT")
        KEYESC = PC.getKeyCode("ESCAPE")
        KEYSHIFT = PC.getKeyCode("RSHIFT")
	
	# hide the mouse
	PC.showMouse( 0 )
	
	# add the player 'plane
	global hero
	hero = Hero( 400, 300 )
	
	# get ready to spawn some stuff
	global nextPlaneTime
	global time
	global squadCount
	nextPlaneTime = 300.0
	time = 0
	squadCount = 0
	
	# set initial random seed
	random.seed(level+345345)
	
	global planes
	global playerShots
	global enemyShots
	planes = []
	playerShots = []
	enemyShots = []
	
	# load the savegame file & current high score
	# skip loading if we're re-initializing
	global bestTime
	global firstRun
	if firstRun:
		try:
			saveFile = open( "lib.py", "r" )
			bestTime = float( saveFile.read() )
			saveFile.close()
		except:
			pass
		firstRun = 0

	# set current game timer
	global gameTime
	gameTime = 0

        PC.playTune(tune)

def fini():
	# attempt to write best time to file
	try:
		saveFile = open( "lib.py", "w" )
		saveFile.write( str( bestTime ) )
		saveFile.close()
	except:
		pass

def keydown( key ):
	if key == KEYESC:
		global doExit
		doExit = 1
	elif key == KEYLEFT:
		hero.left = 1
	elif key == KEYRIGHT:
		hero.right = 1
	elif key == KEYUP:
		hero.up = 1
	elif key == KEYDOWN:
		hero.down = 1
	elif key == KEYSHIFT:
		hero.fire()

def keyup( key ):
	if key == KEYLEFT:
		hero.left = 0
	elif key == KEYRIGHT:
		hero.right = 0
	elif key == KEYUP:
		hero.up = 0
	elif key == KEYDOWN:
		hero.down = 0
def mouseDown( x, y, i ):
	hero.fire()
	global mousex, mousey
	mousex = x
	mousey = y
def mouseMove( x, y ):
	global mousex, mousey
	mousex = x
	mousey = y
def exitGame():
	return doExit;
	
def spawnSquadron( time ):
	# generate squad control data
	# number to spawn
	i = int( math.pow( random.random(), 6 ) * 9 + 1 )
	# mirror this?
	m = int( random.random() + 0.2 )
	# time stagger
	if int( random.random() + 0.5 ):
		timeStagger = random.random() * 30.0 + 10.0
	else:
		timeStagger = 0
	# time between checkpoints
	airTime = random.random() * 500.0 + 150.0
	# entry points, including optional stagger
	entry = random.random()
	if int( random.random() + 0.5 ):
		entryAlt = random.random()
	else:
		entryAlt = entry
	# checkpoint speeds, including optional stagger
	dxi = random.random() * random.random() * 4.0 - 2.0
	if int( random.random() + 0.2 ):
		dxia = random.random() * random.random() * 4.0 - 2.0
	else:
		dxia = dxi
	dyi = random.random() * random.random() * 4.0 - 2.0
	if int( random.random() + 0.2 ):
		dyia = random.random() * random.random() * 4.0 - 2.0
	else:
		dyia = dyi
	dxa = random.random() * random.random() * 2.0 - 1.0
	if int( random.random() + 0.2 ):
		dxaa = random.random() * random.random() * 2.0 - 1.0
	else:
		dxaa = dxa
	dya = random.random() * random.random() * 2.0 - 1.0
	if int( random.random() + 0.2 ):
		dyaa = random.random() * random.random() * 2.0 - 1.0
	else:
		dyaa = dya
	dxb = random.random() * random.random() * 2.0 - 1.0
	if int( random.random() + 0.2 ):
		dxba = random.random() * random.random() * 2.0 - 1.0
	else:
		dxba = dxb
	dyb = random.random() * random.random() * 2.0 - 1.0
	if int( random.random() + 0.2 ):
		dyba = random.random() * random.random() * 2.0 - 1.0
	else:
		dyba = dyb
		
	# pick an entry side & set up for it
	side = random.random()
	if side < 0.2:
		# left
		dxi = abs( dxi )
		dxia = abs( dxia )
		xi = 0
		xia = 0
		yi = entry
		yia = entryAlt
	elif side < 0.4:
		# right
		dxi = -abs( dxi )
		dxia = -abs( dxia )
		xi = 1
		xia = 1
		yi = entry
		yia = entryAlt
	elif side < 0.95:
		# top
		dyi = abs( dyi )
		dyia = abs( dyia )
		xi = entry
		xia = entryAlt
		yi = 0
		yia = 0
	else:
		# bottom
		dyi = -abs( dyi )
		dyia = -abs( dyia )
		xi = entry
		xia = entryAlt
		yi = 1
		yia = 1
	
	# pick loop path points & optionally stagger
	xa = random.random() * 0.4 + 0.3
	if random.random() < 0.5:
		xaa = random.random() *  0.4 + 0.3
	else:
		xaa = xa
	ya = random.random() *  0.4 + 0.1
	if random.random() < 0.5:
		yaa = random.random() *  0.4 + 0.1
	else:
		yaa = xa
	xb = random.random() *  0.4 + 0.3
	if random.random() < 0.5:
		xba = random.random() *  0.4 + 0.3
	else:
		xba = xb
	yb = random.random() *  0.4 + 0.1
	if random.random() < 0.5:
		yba = random.random() *  0.4 + 0.1
	else:
		yba = xb
	
	# rate of fire, if any
	if random.random() < 0.5:
		fireRate = random.random() * 200 + 100
	else:
		fireRate = 0

	# test some overrides
	fireRate = 100 / ( 0.5 + level * 1.0 )

	# program planes
	global planes
	delay = 0
	for p in range( i ):
		a = float( p ) / i
		na = 1 - a
		# add a plane
		planes.append( Plane( 	a * xi + na * xia,
								a * yi + na * yia,
								a * xa + na * xaa,
								a * ya + na * yaa,
								a * xb + na * xba,
								a * yb + na * yba,
								a * dxi + na * dxia,
								a * dyi + na * dyia,
								a * dxa + na * dxaa,
								a * dya + na * dyaa,
								a * dxb + na * dxba,
								a * dyb + na * dyba,
								airTime,
								delay,
								fireRate,
								0 ) )
		if m:
			# add mirrored plane
			planes.append( Plane( 	1 - a * xi - na * xia,
									a * yi + na * yia,
									1 - a * xa - na * xaa,
									a * ya + na * yaa,
									1 - a * xb - na * xba,
									a * yb + na * yba,
									-( a * dxi + na * dxia ),
									a * dyi + na * dyia,
									-( a * dxa + na * dxaa ),
									a * dya + na * dyaa,
									-( a * dxb + na * dxba ),
									a * dyb + na * dyba,
									airTime,
									delay,
									fireRate,
									0 ) )
		delay += timeStagger

def dToA( dx, dy ):
	"""Translates a vector to an angle"""
	# check dangerous special case
	if dy == 0:
		if dx > 0:
			return math.pi / 2
		else:
			return 3 * math.pi / 2
	
	# safe cases
	a = math.atan( dx / dy )
	if dy > 0:
		a += math.pi
	
	# return output rolled into 0..2pi
	return a % ( 2 * math.pi )
	
def hitTest( a, b ):
	"""Circular collision test. Assumes x, y, and radius members on both objects."""
	r = a.radius + b.radius
	x = abs( a.x - b.x )
	y = abs( a.y - b.y )
	if x <= r and y <= r and x*x + y*y <= r*r:
		return 1
	return 0

def normScale( x, y ):
	"""Scale to apply to an xy vector to normalize it, or 0 if it's a 0 vector"""
	if x == 0 and y == 0:
		return 0
	else:
		return 1.0 / pow( x*x + y*y, 0.5 )

def update( delta ):
	# update timers
	global gameTime
	global bestTime
	global level
	global explosions
	gameTime += delta
	if gameTime > bestTime:
		bestTime = gameTime
	
	# spawn new enemy planes
	global nextPlaneTime
	global time
	time += delta
	nextPlaneTime -= delta
	if gameTime < 6000:
		if nextPlaneTime <= 0:
			nextPlaneTime = random.random() * random.random() * random.random() * 1600 / ( gameTime * 0.0001 + 1 ) + 100
			global squadCount
			squadCount += 1
			if squadCount % 10 == 0:
				nextPlaneTime += 500.0
			spawnSquadron( time )
	
	# tick the player 'plane
	hero.update( delta )
	
	# update explosions, removing those that're done
	doneList = []
	for e in explosions:
		e.update( delta )
		if e.done:
			doneList.append( e )
	for e in doneList:
		explosions.remove( e )
	
	# update bullets, removing those that're done
	doneList = []
	for s in playerShots:
		s.update( delta * 0.5 )
		if s.done:
			doneList.append( s )
	for s in doneList:
		playerShots.remove( s )
		
	doneList = []
	for s in enemyShots:
		s.update( delta * 0.5 )
		if s.done:
			doneList.append( s )
	for s in doneList:
		enemyShots.remove( s )
		
	# update enemy 'planes
	doneList = []
	for p in planes:
		p.update( delta * 0.5 )
		if p.mode == PLANEMODE_DONE:
			doneList.append( p )
	
	# remove dead or finished planes
	for d in doneList:
		planes.remove( d )
		
	# test for player-shot collisions
	global doExit
	for s in enemyShots:
		if hitTest( s, hero ):
			# hit the hero plane
			if not hero.hit( s.x, s.y ):
				# player death
				init()
			else:
				# particle
				explosions.append( Explosion( hero.x, hero.y ) )
	
	# test for player-plane collisions
	for p in planes:
		if hitTest( p, hero ):
			# hit the hero plane
			if not hero.hit( p.x, p.y ):
				# player death
				init()
				# break out early
				PC.markDirty()
				return
			else:
				# particle
				explosions.append( Explosion( hero.x, hero.y ) )

	
	# test for playerShot-plane collisions
	deadPlanes = []
	deadShots = []
	for s in playerShots:
		for p in planes:
			if hitTest( s, p ):
				# remove
				if deadPlanes.count( p ) == 0:
					deadPlanes.append( p )
				if deadShots.count( s ) == 0:
					deadShots.append( s )
				# particle
				explosions.append( Explosion( p.x, p.y ) )
	for p in deadPlanes:
		planes.remove( p )
	for s in deadShots:
		playerShots.remove( s )
	
	# call the draw function
	PC.markDirty()
	
	# test for level win
	if gameTime >= 6000 and len( planes ) == 0:
		level += 1
		# next level
		init()
		# break out early
		PC.markDirty()
		return


def draw():
	global time
	# levelAlpha: distance through the level
	levelAlpha = min( gameTime / 6000.0, 1 )
	
	# draw the background
	# clear
	PC.setColour( 255, 255, 255, 255 )
	PC.fillRect( 0, 0, 800, 600 )
	
	# prep for layers
	PC.setColour( 50 + int( 205 * levelAlpha ) , 100, 255 - int( 205 * levelAlpha ), 255 )
	PC.setColourize( 1 )

	# draw sky
	scroll = time % 800
	PC.drawImageF( skyImage, scroll - 800, 0 )
        if PC.getIs3DAccelerated():
                PC.drawImageF( skyImage, scroll, 0 )
        else:
                PC.drawImageF( skyImage, scroll - 2, 0 )

	# foreground
	PC.drawImage( backgroundImage, 0, 0 )
	PC.setColourize( 0 )

	# explosions
	for e in explosions:
		e.draw()

	# draw shots
	for s in enemyShots:
		s.draw()
	for s in playerShots:
		s.draw()

	# draw enemy 'planes
	for p in planes:
		p.draw()
		
	# draw the player 'plane
	hero.draw()
	
	# draw the HUD elements
	# time
	PC.setColour( 255, 255, 255, 255 )
	time = max( 0, 6000 - gameTime )
	string = "Time Left: " + timeString( time )
	PC.setFont( font )
	PC.drawString( string, 25, PCR.fontAscent( font ) )

	# current level
	string = "Level: " + str( level )
	PC.drawString( string, 775 - PCR.stringWidth( string, font ), PCR.fontAscent( font ) )
	
	# lives remaining
	if hero.lives > 0:
		string = "Lives: "
		width = PCR.stringWidth( string, font )
		x = 400 - width * 0.5 - hero.radius * 4
		PC.drawString( string, x, 575 )
		
		# draw life counter
		for i in range( hero.lives ):
			PC.drawImageF( hero.image, x + width + hero.radius * ( 2 * i ), 575 - hero.radius * 2 )
			
	else:
		# flashing warning
		if ( gameTime % 10.0 ) < 5.0:
			string = "NO LIVES LEFT!"
			PC.drawString( string, 400 - PCR.stringWidth( string, font ) * 0.5, 575 )
			
	# draw the canvas overlay
	PC.setColourize( 1 )
	PC.setColour( 0, 0, 0, 255 )
	PC.drawImage( canvasImage, 0, 0 )
	PC.setColourize( 0 )
	
def timeString( time ):
	minutes = int( time * 0.01 ) / 60
	seconds = int( time * 0.01 ) - 60 * minutes
	minutesString = str( minutes )
	secondsString = str( seconds )
	# add seconds padding if necessary
	if len( secondsString ) == 1:
		secondsString = "0" + str( seconds )
	return minutesString + ":" + secondsString
