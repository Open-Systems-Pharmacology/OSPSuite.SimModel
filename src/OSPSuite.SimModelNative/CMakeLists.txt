cmake_minimum_required (VERSION 3.9)
project (OSPSuite.SimModelNative)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories (
    ${OSPSuite.SimModelNative_SOURCE_DIR}/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/include 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/include 
	/usr/include/libxml2 
	${OSPSuite.SimModelNative_SOURCE_DIR}/../../packages/OSPSuite.FuncParser.Ubuntu18.4.0.0.43/OSPSuite.FuncParserNative/include
)

file (GLOB SOURCES 
        ${OSPSuite.SimModelNative_SOURCE_DIR}/src/*.cpp version.h
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/src/*.cpp
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/src/*.cpp
		${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/src/*.cpp
)

add_library (OSPSuite.SimModelNative SHARED ${SOURCES})

target_link_libraries (OSPSuite.SimModelNative ${OSPSuite.SimModelNative_SOURCE_DIR}/../../${libFuncParser})