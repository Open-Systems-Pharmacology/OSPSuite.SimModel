cmake_minimum_required (VERSION 3.9)
project (OSPSuite.SimModelNative)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(LibXml2 REQUIRED)

include_directories (
    ${OSPSuite.SimModelNative_SOURCE_DIR}/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/include 
	${LIBXML2_INCLUDE_DIR}
	${OSPSuite.SimModelNative_SOURCE_DIR}/../../packages/OSPSuite.FuncParser.Ubuntu18.${FuncParserVersion}/OSPSuite.FuncParserNative/include
)

file (GLOB SOURCES 
        ${OSPSuite.SimModelNative_SOURCE_DIR}/src/*.cpp version.h
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/src/*.cpp
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/src/*.cpp
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/src/*.cpp
)

add_library (OSPSuite.SimModelNative SHARED ${SOURCES})

target_link_libraries (OSPSuite.SimModelNative 
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../../packages/OSPSuite.FuncParser.Ubuntu18.${FuncParserVersion}/OSPSuite.FuncParserNative/bin/native/x64/Release/libOSPSuite.FuncParserNative.so
	${LIBXML2_LIBRARIES}
)