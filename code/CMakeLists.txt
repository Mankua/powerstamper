CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT( PowerStamper )

# 3 d s   m a x

# 3ds Max 9    | MAXSDK9  | Visual Studio 8.0 | 32 Bits | 64 Bits 
# 3ds Max 2008 | MAXSDK10 | Visual Studio 8.0 | 32 Bits | 64 Bits 
# 3ds Max 2009 | MAXSDK11 | Visual Studio 8.0 | 32 Bits | 64 Bits 
# 3ds Max 2010 | MAXSDK12 | Visual Studio 9.0 | 32 Bits | 64 Bits 
# 3ds Max 2011 | MAXSDK13 | Visual Studio 9.0 | 32 Bits | 64 Bits 
                                                      
# Define the Project Name.
#  Projects that are built with the same IDE (ie. Visual Studio 2005) will be
#  added to the same Solution. For this reason all project need to have 
#  unique names.

SET ( POWERSTAMPER_MAX_6  "PowerStamperMax6" )
SET ( POWERSTAMPER_MAX_9  "PowerStamperMax9" )
SET ( POWERSTAMPER_MAX_10 "PowerStamperMax2008" )
SET ( POWERSTAMPER_MAX_11 "PowerStamperMax2008" )
SET ( POWERSTAMPER_MAX_12 "PowerStamperMax2010" )

# Define the header libraries for each SDK

SET ( MAXSDK6_HEADERS_DIR  "c:/Libraries/maxsdk/6/include" )
SET ( MAXSDK9_HEADERS_DIR  "c:/Libraries/maxsdk/9/include" )
SET ( MAXSDK10_HEADERS_DIR "c:/Libraries/maxsdk/10/include" )
SET ( MAXSDK11_HEADERS_DIR "c:/Libraries/maxsdk/11/include" )
SET ( MAXSDK12_HEADERS_DIR "c:/Libraries/maxsdk/12/include" )

# Define the library files for each SDK, and platform. 64 bits lib path will replace
# 32 bits lib path.

# 32 BITS 

SET ( MAXSDK6_LIBRARY_DIR  "c:/Libraries/maxsdk/6/lib" )
SET ( MAXSDK9_LIBRARY_DIR  "c:/Libraries/maxsdk/9/lib" )
SET ( MAXSDK10_LIBRARY_DIR "c:/Libraries/maxsdk/10/lib" )
SET ( MAXSDK11_LIBRARY_DIR "c:/Libraries/maxsdk/11/lib" )
SET ( MAXSDK12_LIBRARY_DIR "c:/Libraries/maxsdk/12/lib" )

SET ( PROGRAM_FILES "c:\\Program Files (x86)" )

IF ( CMAKE_CL_64 ) # 64 BITS

	SET ( MAXSDK9_LIBRARY_DIR  "c:/Libraries/maxsdk/9/x64/lib" )
	SET ( MAXSDK10_LIBRARY_DIR "c:/Libraries/maxsdk/10/x64/lib" )
	SET ( MAXSDK11_LIBRARY_DIR "c:/Libraries/maxsdk/11/x64/lib" )
	SET ( MAXSDK12_LIBRARY_DIR "c:/Libraries/maxsdk/12/x64/lib" )

	SET ( PROGRAM_FILES "c:\\Program Files" )
		
ENDIF ()

# Add the source files.

SET ( SOURCE_FILES
	"Plugin.cpp"
	"Nebbi.cpp"
	"nebbi_render.cpp"
)

# Add the header files.

SET ( HEADER_FILES
	"resource.h"
	"Nebbi.h"
)

# Add the resource and definition file.

SET ( RESOURCE_FILES
	"PowerStamper.rc"
	"Plugin.def"
)

# Define the max libraries.

SET( MAX_LIBS  "mnmath.lib"
			   "poly.lib"
			   "paramblk2.lib"
			   "core.lib"
			   "geom.lib"
			   "mesh.lib"
			   "maxutil.lib"
			   "bmm.lib"
			   "maxscrpt.lib"
			   "comctl32.lib"
)


IF ( MSVC80 )

	#######################################################
	# P L U G I N   F O R   3 D S M A X   9               #
	#######################################################
	
	INCLUDE_DIRECTORIES ( ${MAXSDK9_HEADERS_DIR} )
	LINK_DIRECTORIES ( ${MAXSDK9_LIBRARY_DIR} )

	ADD_LIBRARY ( ${POWERSTAMPER_MAX_9} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} )
	TARGET_LINK_LIBRARIES( ${POWERSTAMPER_MAX_9} ${MAX_LIBS} )

	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_9} PROPERTIES OUTPUT_NAME "Max9/PowStamp" )
	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_9} PROPERTIES SUFFIX ".dlu" )
	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_9} PROPERTIES CLEAN_DIRECT_OUTPUT 1 )

	SET ( LOCAL_WARNING_FLAGS /W3 )
	SET ( LOCAL_RTTI_FLAGS /GR )

	#######################################################
	# P L U G I N   F O R   3 D S M A X   2 0 0 8         #
	#######################################################
	
	INCLUDE_DIRECTORIES ( ${MAXSDK10_HEADERS_DIR} )
	LINK_DIRECTORIES ( ${MAXSDK10_LIBRARY_DIR} )

	ADD_LIBRARY ( ${POWERSTAMPER_MAX_10} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} )
	TARGET_LINK_LIBRARIES( ${POWERSTAMPER_MAX_10} ${MAX_LIBS} )

	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_10} PROPERTIES OUTPUT_NAME "Max2008/PowStamp" )
	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_10} PROPERTIES SUFFIX ".dlu" )
	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_10} PROPERTIES CLEAN_DIRECT_OUTPUT 1 )

	SET ( LOCAL_WARNING_FLAGS /W3 )
	SET ( LOCAL_RTTI_FLAGS /GR )
	
ENDIF ( MSVC80 )

IF ( MSVC90 )

	#######################################################
	# P L U G I N   F O R   3 D S M A X   2 0 1 0         #
	#######################################################
	
	INCLUDE_DIRECTORIES ( ${MAXSDK12_HEADERS_DIR} )
	LINK_DIRECTORIES ( ${MAXSDK12_LIBRARY_DIR} )
	
	ADD_LIBRARY ( ${POWERSTAMPER_MAX_12} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} )
	TARGET_LINK_LIBRARIES( ${POWERSTAMPER_MAX_12} ${MAX_LIBS} )

	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_12} PROPERTIES OUTPUT_NAME "Max2010/PowStamp" )
	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_12} PROPERTIES SUFFIX ".dlu" )
	SET_TARGET_PROPERTIES( ${POWERSTAMPER_MAX_12} PROPERTIES CLEAN_DIRECT_OUTPUT 1 )	

	SET ( LOCAL_WARNING_FLAGS /W3 )
	SET ( LOCAL_RTTI_FLAGS /GR )

	ADD_CUSTOM_COMMAND( TARGET ${POWERSTAMPER_MAX_12} POST_BUILD COMMAND copy $(ConfigurationName)\\Max2010\\PowStamp.dlu "${PROGRAM_FILES}\\Autodesk\\3ds Max 2010\\plugins" )
	
ENDIF ( MSVC90 )