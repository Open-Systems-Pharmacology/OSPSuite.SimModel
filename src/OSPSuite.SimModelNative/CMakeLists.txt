cmake_minimum_required (VERSION 3.9)
project (OSPSuite.SimModelNative)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(RPATH ".")
    list(APPEND CMAKE_BUILD_RPATH ${RPATH})
endif()

find_package(LibXml2 REQUIRED)

include_directories (
    ${OSPSuite.SimModelNative_SOURCE_DIR}/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../../packages/OSPSuite.FuncParser.Ubuntu22/OSPSuite.FuncParserNative/include
	${LIBXML2_INCLUDE_DIR}
)

file (GLOB SOURCES 
        ${OSPSuite.SimModelNative_SOURCE_DIR}/src/*.cpp version.h
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/src/*.cpp
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/src/*.cpp
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/src/*.cpp
)

add_library (OSPSuite.SimModelNative SHARED ${SOURCES})

target_link_libraries (OSPSuite.SimModelNative 
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../../${FuncParserDir}libOSPSuite.FuncParserNative.${EXT}
	${LIBXML2_LIBRARIES}
)
