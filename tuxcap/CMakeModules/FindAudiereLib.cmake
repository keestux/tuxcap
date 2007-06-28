# 
# Adjusted cmake script for Magick++ by Jan Woetzel 10/2004 
# --------------------------------


# find the libraries main include header file
FIND_PATH(AUDIERELIB_INCLUDE_DIR audiere.h
  "${AUDIERELIB_DIR}/include"
  "$ENV{AUDIERELIB_DIR}/include"
  "${AUDIERELIB_HOME}/include"
  "$ENV{AUDIERELIB_HOME}/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/include"
  /usr/local/include
  /usr/include
  /opt/net/gcc41/ImageMagick/include
  /opt/net/gcc33/ImageMagick/include
  )
#MESSAGE("DBG AUDIERELIB_INCLUDE_DIR=${AUDIERELIB_INCLUDE_DIR}")


# set directories to search for libraries: 
SET(AUDIERELIB_POSSIBLE_LIBRARY_PATHS
  "${AUDIERELIB_DIR}/lib"
  "$ENV{AUDIERELIB_DIR}/lib"
  "${AUDIERELIB_HOME}/lib"
  "$ENV{AUDIERELIB_HOME}/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;LibPath]/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;LibPath]"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/lib"  
  /usr/local/lib
  /usr/lib
  /opt/net/gcc41/ImageMagick/lib  
  /opt/net/gcc33/ImageMagick/lib  
  )
#MESSAGE("DBG AUDIERELIB_POSSIBLE_LIBRARY_PATHS=${AUDIERELIB_POSSIBLE_LIBRARY_PATHS}")

FIND_LIBRARY(AUDIERELIB_LIBRARY
  NAMES audiere CORE_RL_audiere_ CORE_DB_audiere_ 
  PATHS ${AUDIERELIB_POSSIBLE_LIBRARY_PATHS} )
#MESSAGE("DBG AUDIERELIB_LIBRARY=${AUDIERELIB_LIBRARY}")

# --------------------------------
# decide if we found all we require: 
IF(NOT AUDIERELIB_LIBRARY)
  MESSAGE(SEND_ERROR "FindAudiereLib.cmake could not find AUDIERELIB_LIBRARY")
ENDIF(NOT AUDIERELIB_LIBRARY)

# get the path(=directory) of the main library:
GET_FILENAME_COMPONENT(AUDIERELIB_LINK_DIRECTORIES ${AUDIERELIB_LIBRARY} PATH)

IF (AUDIERELIB_INCLUDE_DIR AND AUDIERELIB_LIBRARY)
  # OK:
  SET(AUDIERELIB_FOUND TRUE)
  SET(AUDIERELIB_LIBRARIES
    ${AUDIERELIB__LIBRARY}
    )
ELSE (AUDIERELIB_INCLUDE_DIR AND AUDIERELIB_LIBRARY)
  MESSAGE(SEND_ERROR "FindAudiere could not find Audiere library  or header(s)")
ENDIF (AUDIERELIB_INCLUDE_DIR AND AUDIERELIB_LIBRARY)


MARK_AS_ADVANCED(
  AUDIERELIB_INCLUDE_DIR
  AUDIERELIB_LIBRARIES
  AUDIERELIB_LIBRARY
  AUDIERELIB_WAND_LIBRARY
  AUDIERELIB_INCLUDE_DIR
  )

