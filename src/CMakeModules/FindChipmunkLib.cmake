# 
# Adjusted cmake script for Magick++ by Jan Woetzel 10/2004 
# --------------------------------

# find the libraries main include header file
FIND_PATH(CHIPMUNKLIB_INCLUDE_DIR chipmunk.h
  "${CHIPMUNKLIB_DIR}/include"
  "$ENV{CHIPMUNKLIB_DIR}/include"
  "${CHIPMUNKLIB_HOME}/include"
  "$ENV{CHIPMUNKLIB_HOME}/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Chipmunk\\Current;BinPath]/include"
  /usr/local/include
  /usr/local/include/chipmunk
  /usr/include
  /opt/net/gcc41/Chipmunk/include
  /opt/net/gcc33/Chipmunk/include
  )
#MESSAGE("DBG CHIPMUNKLIB_INCLUDE_DIR=${CHIPMUNKLIB_INCLUDE_DIR}")


# set directories to search for libraries: 
SET(CHIPMUNKLIB_POSSIBLE_LIBRARY_PATHS
  "${CHIPMUNKLIB_DIR}/lib"
  "$ENV{CHIPMUNKLIB_DIR}/lib"
  "${CHIPMUNKLIB_HOME}/lib"
  "$ENV{CHIPMUNKLIB_HOME}/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Chipmunk\\Current;LibPath]/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Chipmunk\\Current;LibPath]"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Chipmunk\\Current;BinPath]/lib"  
  /usr/local/lib
  /usr/local/lib/chipmunk
  /usr/lib
  /opt/net/gcc41/Chipmunk/lib  
  /opt/net/gcc33/Chipmunk/lib  
  )
#MESSAGE("DBG CHIPMUNKLIB_POSSIBLE_LIBRARY_PATHS=${CHIPMUNKLIB_POSSIBLE_LIBRARY_PATHS}")

FIND_LIBRARY(CHIPMUNKLIB_LIBRARY
  NAMES chipmunk CORE_RL_chipmunk_ CORE_DB_chipmunk_ 
  PATHS ${CHIPMUNKLIB_POSSIBLE_LIBRARY_PATHS} )
#MESSAGE("DBG CHIPMUNKLIB_LIBRARY=${CHIPMUNKLIB_LIBRARY}")

# --------------------------------
# decide if we found all we require: 
IF(NOT CHIPMUNKLIB_LIBRARY)
  MESSAGE(SEND_ERROR "FindChipmunkLib.cmake could not find CHIPMUNKLIB_LIBRARY")
ENDIF(NOT CHIPMUNKLIB_LIBRARY)

# get the path(=directory) of the main library:
GET_FILENAME_COMPONENT(CHIPMUNKLIB_LINK_DIRECTORIES ${CHIPMUNKLIB_LIBRARY} PATH)

IF (CHIPMUNKLIB_INCLUDE_DIR AND CHIPMUNKLIB_LIBRARY)
  # OK:
  SET(CHIPMUNKLIB_FOUND TRUE)
  SET(CHIPMUNKLIB_LIBRARIES
    ${CHIPMUNKLIB__LIBRARY}
    )
ELSE (CHIPMUNKLIB_INCLUDE_DIR AND CHIPMUNKLIB_LIBRARY)
  MESSAGE(SEND_ERROR "FindChipmunkLib.cmake  could not find Chipmunk library  or header(s)")
ENDIF (CHIPMUNKLIB_INCLUDE_DIR AND CHIPMUNKLIB_LIBRARY)


MARK_AS_ADVANCED(
  CHIPMUNKLIB_INCLUDE_DIR
  CHIPMUNKLIB_LIBRARIES
  CHIPMUNKLIB_LIBRARY
  CHIPMUNKLIB_WAND_LIBRARY
  CHIPMUNKLIB_INCLUDE_DIR
  )

