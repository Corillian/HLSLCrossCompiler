CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT (HLSLCrossCompilerProj C CXX)

#IF(DXSDKDIR)
#    INCLUDE_DIRECTORIES($(DXSDKDIR)\\Include)
#    LINK_DIRECTORIES($(DXSDKDIR)\\Lib\\x86)
#ELSE()
#    INCLUDE_DIRECTORIES("C:\\Program Files\\Microsoft DirectX SDK (June 2010)\\Include")
#    LINK_DIRECTORIES("C:\\Program Files\\Microsoft DirectX SDK (June 2010)\\Lib\\x86")
#ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
#	SET(CMAKE_BUILD_TYPE "Debug")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

IF( ${CMAKE_BUILD_TYPE} STREQUAL Debug )
    # CMAKE_C_FLAGS_DEBUG will also be added to CMAKE_C_FLAGS
    MESSAGE( "Debug build" )
    ADD_DEFINITIONS(-DDEBUG -D_DEBUG -D__DEBUG__)
ENDIF()

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../lib" )

INCLUDE(TestBigEndian)

TEST_BIG_ENDIAN(BIG_ENDIAN_SYSTEM)

IF(BIG_ENDIAN_SYSTEM)
    MESSAGE("SETTING Big endian")
    ADD_DEFINITIONS(-D__BIG_ENDIAN__)
ELSE()
    MESSAGE("SETTING Little endian")
    ADD_DEFINITIONS(-D__LITTLE_ENDIAN__)
ENDIF()

IF(MSVC)
	OPTION(USE_MSVC_FAST_FLOATINGPOINT "Use MSVC /fp:fast option" ON)
	IF(USE_MSVC_FAST_FLOATINGPOINT)
		ADD_DEFINITIONS(/fp:fast)
	ENDIF(USE_MSVC_FAST_FLOATINGPOINT)
ENDIF(MSVC)

IF(WIN32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS /WX /W3 /arch:SSE2)
ENDIF()

INCLUDE_DIRECTORIES( ../ ../cbstring/)

file(GLOB CBSTRING_CFILES ../cbstring/*.c)
file(GLOB CBSTRING_HFILES ../cbstring/*.h)
source_group("CBString Files" FILES ${CBSTRING_CFILES} ${CBSTRING_HFILES})

ADD_LIBRARY( libHLSLcc STATIC 
../debug.h
../decode.h
../hlsl_opcode_funcs_glsl.h
../languages.h
../pstdint.h
../reflect.h
../structs.h
../toGLSL.h
../toGLSLDeclaration.h
../toGLSLInstruction.h
../toGLSLOperand.h
../tokens.h
../shaderLimits.h
${CBSTRING_HFILES}
../decode.c
../reflect.c
../toGLSL.c
../toGLSLInstruction.c
../toGLSLOperand.c
../toGLSLDeclaration.c
${CBSTRING_CFILES}
)

ADD_EXECUTABLE( HLSLcc ../toGLSLStandalone.c ../timer.h ../timer.c)

TARGET_LINK_LIBRARIES( HLSLcc libHLSLcc)


# ADD_EXECUTABLE( Reflect ../reflect.cpp )

# force variables that could be defined on the cmdline
# to be written to the cach

SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
"One of None Debug Release RelWithDebInfo MinSizeRel" FORCE )

