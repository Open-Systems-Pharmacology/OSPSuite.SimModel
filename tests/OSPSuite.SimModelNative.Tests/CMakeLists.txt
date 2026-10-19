cmake_minimum_required (VERSION 3.14)
project (OSPSuite.SimModelNative.Tests)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Locate the SimModelNative shared library built earlier by buildNix.sh. The
# unit tests link against it instead of recompiling every source so the exact
# same binary that ships is exercised. Path is passed in from the script.
if (NOT DEFINED SIMMODEL_NATIVE_DIR)
	 message (FATAL_ERROR "SIMMODEL_NATIVE_DIR must be set to the directory containing libOSPSuite.SimModelNative.${EXT}")
endif()

set (SRC_ROOT ${OSPSuite.SimModelNative.Tests_SOURCE_DIR}/../../src)

# FuncParser is staged next to the shared library by buildNix.sh and found via
# its RPATH ($ORIGIN / @loader_path); add the native dir so the test exe
# resolves the shared library itself.
if (APPLE)
	 set (CMAKE_MACOSX_RPATH 1)
	 list (APPEND CMAKE_BUILD_RPATH ${SIMMODEL_NATIVE_DIR})
elseif (UNIX)
	 list (APPEND CMAKE_BUILD_RPATH ${SIMMODEL_NATIVE_DIR})
endif()

if (APPLE)
	 find_package (LibXml2 REQUIRED)
else()
	 # Headers of the static libxml2 built into SimModelNative
	 set (LIBXML2_INCLUDE_DIR ${SIMMODEL_NATIVE_DIR}/libxml2/include/libxml2)
endif()

# Fetch GoogleTest so the build has no system-level test-framework dependency,
# mirroring the Microsoft.googletest NuGet package used by the Windows vcxproj.
include (FetchContent)
FetchContent_Declare (
	 googletest
	 GIT_REPOSITORY https://github.com/google/googletest.git
	 GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable (googletest)

include_directories (
	 ${SRC_ROOT}/OSPSuite.SimModelNative/include
	 ${SRC_ROOT}/OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/include
	 ${SRC_ROOT}/OSPSuite.SysTool/include
	 ${SRC_ROOT}/OSPSuite.XMLWrapper/include
	 ${SRC_ROOT}/OSPSuite.XMLWrapper
	 ${SRC_ROOT}/../packages/OSPSuite.FuncParser/OSPSuite.FuncParserNative/include
	 ${LIBXML2_INCLUDE_DIR}
)

file (GLOB TEST_SOURCES ${OSPSuite.SimModelNative.Tests_SOURCE_DIR}/src/*.cpp)

add_executable (OSPSuite.SimModelNative.Tests ${TEST_SOURCES})

target_link_libraries (OSPSuite.SimModelNative.Tests
	 ${SIMMODEL_NATIVE_DIR}/libOSPSuite.SimModelNative.${EXT}
	 GTest::gtest
	 GTest::gtest_main
)
