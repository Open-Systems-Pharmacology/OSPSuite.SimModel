//1 Header File: SysTool Configuration
//
//1.1 Overview
//
//The compilation of SysTool has to take into account differences in the
//architecture of the underlying operating system. Usually the compiler defines
//symbols which can be used to identify the operating system SysTool is built for.
//
//Depending on the actual operating system several constants are defined 
//of the c
//In this header file the operating system is identified and depending on the
//operating system several constants are defined and system dependent
//
//1.1 Imports, Types


#ifndef SYSTOOL_CONFIG_H
#define SYSTOOL_CONFIG_H

#define SYSTOOL_VERSION            "1.1.1"
#define SYSTOOL_VERSION_MAJOR      1
#define SYSTOOL_VERSION_MINOR      1
#define SYSTOOL_VERSION_REVISION   1

//Define the preprocessor symbol *SYSTOOL_LITTLE_ENDIAN* if your machine has a little
//endian byte order architecture. Otherwise ~\#undef~ this symbol.
#define SYSTOOL_LITTLE_ENDIAN

//---- Detect the platform

//Windows
#if (defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) \
  || defined(__MINGW32__)) && !defined(__CYGWIN__)
#  define SYSTOOL_WIN32
   // Define Windows version for WINVER and _WIN32_WINNT
   // 0x0400 = Windows NT 4.0
   // 0x0500 = Windows 2000 (NT based)
#  define WINVER       0x0501
#  define _WIN32_WINNT 0x0501
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>


//When creating or using shared libraries (i.e. DLLs = Dynamic Link Libraries)
//on Windows platforms it is necessary to specify for C++ classes whether they
//are imported from a DLL (when using a DLL) or exported from a DLL (when creating a DLL).
//
//Define SYSTOOL_USE_DLL if compiling modules which want to use a DLL.
//Define SYSTOOL_CREATE_DLL if compiling this component as a DLL.
//
//If none of these symbols is defined it is assumed that a static library is to be built.
#  if defined(SYSTOOL_USE_DLL)
#    define SYSTOOL_EXPORT __declspec(dllimport)
#    define SYSTOOL_TEMPLATE extern
#  elif defined(SYSTOOL_CREATE_DLL)
#    define SYSTOOL_EXPORT __declspec(dllexport)
#    define SYSTOOL_TEMPLATE
#  else
#    define SYSTOOL_EXPORT
#    define SYSTOOL_TEMPLATE
#  endif

//Linux

#elif defined(__linux__)
#  define SYSTOOL_LINUX
//Creating shared libraries requires no special measures.
#  define SYSTOOL_EXPORT

//Solaris
#elif defined(unix) && defined(sun)
#  define SYSTOOL_SOLARIS
//Creating shared libraries requires no special measures.
#  define SYSTOOL_EXPORT
#else
#  error Could not identify the operating system
#endif

//The following symbol must be defined in order to use the latest POSIX APIs.
#ifdef __GNUC__
#  define _GNU_SOURCE 1
#endif

//Define separator character for pathnames an PATH environment variable
#ifdef SYSTOOL_WIN32
#  define PATH_SLASH "\\"
#  define PATH_SLASHCHAR '\\'
#  define PATH_SEP ";"
#  define PATH_SEPCHAR ';'
#else
#  define PATH_SLASH "/"
#  define PATH_SLASHCHAR '/'
#  define PATH_SEP ":"
#  define PATH_SEPCHAR ':'
#endif

//Number of elements in a static array.
#define nelems(x) (sizeof((x))/sizeof(*(x)))

//Size of a structure member.
#define sizeofm(struct_t,member) \
  ((size_t)(sizeof(((struct_t *)0)->member)))

//Conversion between timeval and timespec
#define CONVERT_TIMEVAL_TO_TIMESPEC(tv,ts) \
{ \
  (ts)->tv_sec=(tv)->tv_sec; \
  (ts)->tv_nsec=(tv)->tv_usec*1000; \
}
#define CONVERT_TIMESPEC_TO_TIMEVAL(tv,ts) \
{ \
  (tv)->tv_sec=(ts)->tv_sec; \
  (tv)->tv_usec=(ts)->tv_nsec/1000; \
}

//Default includes:
//  * stdint.h -- defines standard integer types
//    (alternatively ~inttypes.h~ could be used where ~stdint.h~ is not available).
//  * string -- defines the C++ string data type.
#ifndef SYSTOOL_SOLARIS
#ifdef _MSC_VER
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
#pragma warning(disable: 4786)
#include "stdint.h"
#else
#include <stdint.h>
#endif
#else
#include <inttypes.h>
#endif
#include <string>

#endif // SYSTOOL_CONFIG_H

