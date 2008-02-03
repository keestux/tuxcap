# Template pycap game v1.0
# Written by Farbs

appIni = {	"mCompanyName"		: "CompanyNameGoesHere",
		"mFullCompanyName"	: "CompanyNameGoesHere",
		"mProdName"		: "Pycap Template Game",
		"mProductVersion"	: "1.0",
		"mTitle"			: "Pycap Template Game v1.0",
		"mRegKey"			: "PycapTest",
		"mWidth"			: 800,
		"mHeight"			: 600,
		"mAutoEnable3D"	: 0,
		"mVSyncUpdates"	: 1 }

import Pycap as PC
PCR = None

def loadBase():
	import PycapRes
	global PCR
	PCR = PycapRes

def init():
	pass

def update( delta ):
	PC.markDirty()

def draw():
	pass
