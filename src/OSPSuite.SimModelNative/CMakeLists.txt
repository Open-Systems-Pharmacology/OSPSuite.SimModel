cmake_minimum_required (VERSION 3.18)
project (OSPSuite.SimModelNative)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(RPATH ".")
    list(APPEND CMAKE_BUILD_RPATH ${RPATH})
elseif (UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_RPATH "$ORIGIN")
endif()

if (APPLE)
    find_package(LibXml2 REQUIRED)
else()
    # Link libxml2 statically: its soname differs between distributions (libxml2.so.2, libxml2.so.16)
    include(ExternalProject)
    if (POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    set(LIBXML2_PREFIX ${CMAKE_BINARY_DIR}/libxml2)
    set(LIBXML2_INCLUDE_DIR ${LIBXML2_PREFIX}/include/libxml2)
    set(LIBXML2_LIBRARIES ${LIBXML2_PREFIX}/lib/libxml2.a)
    ExternalProject_Add(libxml2
        URL https://download.gnome.org/sources/libxml2/2.15/libxml2-2.15.4.tar.xz
        URL_HASH SHA256=98087fd181d9070724f3fbc65c7377db03038eb92bd882374daff44940138821
        INSTALL_DIR ${LIBXML2_PREFIX}
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_INSTALL_LIBDIR=lib
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DBUILD_SHARED_LIBS=OFF
            -DLIBXML2_WITH_ICU=OFF
            -DLIBXML2_WITH_ZLIB=OFF
            -DLIBXML2_WITH_PROGRAMS=OFF
            -DLIBXML2_WITH_TESTS=OFF
        BUILD_BYPRODUCTS ${LIBXML2_LIBRARIES}
    )
endif()

include_directories (
    ${OSPSuite.SimModelNative_SOURCE_DIR}/include
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SimModelSolverBase/src/OSPSuite.SimModelSolverBase/include
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.SysTool/include
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../OSPSuite.XMLWrapper/include
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../../packages/OSPSuite.FuncParser/OSPSuite.FuncParserNative/include
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
    ${OSPSuite.SimModelNative_SOURCE_DIR}/../../packages/OSPSuite.FuncParser/runtimes/${RID}/native/libOSPSuite.FuncParserNative.${EXT}
    ${LIBXML2_LIBRARIES}
)

if (NOT APPLE)
    add_dependencies(OSPSuite.SimModelNative libxml2)
    # Don't export libxml2's symbols, so they can't clash with another libxml2 loaded in the same process
    set_target_properties(OSPSuite.SimModelNative PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,ALL")
endif()
