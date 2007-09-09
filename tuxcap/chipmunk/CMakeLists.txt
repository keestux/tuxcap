SET(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -std=gnu99")
SET(CMAKE_C_FLAGS_DEBUG "-std=gnu99")

ADD_LIBRARY(chipmunk STATIC 
	chipmunk.c
	cpArbiter.c
	cpArray.c
	cpBB.c
	cpBody.c
	cpCollision.c
	cpHashSet.c
	cpJoint.c
	cpPolyShape.c
	cpShape.c
	cpSpace.c
	cpSpaceHash.c
	cpVect.c
)
