//1 Header File: Dynamic Library Management
//
//1.1 Overview
//
//The module ~DynamicLibrary~ provides support for dynamic linking, which allows
//a process to link libraries at run-time. Dynamically loaded libraries are very
//similar to ordinary shared libraries; they are usually built as standard shared
//libraries. The main difference is that the libraries aren't automatically
//loaded at program link time or start-up; instead, there is an application
//programming interface (API) for opening a library, looking up symbols, handling
//errors, and closing the library. Often it is a very useful feature for
//applications to be able to load and execute code from an external source at the
//runtime. Plugins or modules are good examples.
//
//Unix-like operating systems, i.e. Linux and Solaris, provide this facility
//by offering the *dl* functions (~dlopen~, ~dlsym~, ~dlerror~ and ~dlclose~)
//as a mechanism for loading libraries at runtime. The Win32 API provides the
//functions ~LoadLibrary~, ~GetProcAddr~ and ~FreeLibrary~ for managing dynamic
//link libraries (DLL).
//
//Unfortunately there are considerable differences in the implementation
//and functionality of the dynamic loading mechanism on these platforms.
//While Unix-like systems usually perform a complete linking step resolving
//unresolved function and data references, Windows resolves only references
//from the loaded DLL to other DLLs. To make things worse, this is only done,
//when the DLL was created using appropriate ~import libraries~. Otherwise
//it's up to the DLL itself to resolve references. One has to keep these
//deficiencies in mind when writing portable programs.
//
//In most cases an application loads a dynamic library, tries to locate a
//function by using its known name and then to execute the code of the
//function to perform the requested task. Using a C function interface
//locating the address of a function is quite straight forward since the
//names seen in the C source code and seen by the linker are essentially
//the same. Using C++ things get more complicated since due to the name
//mangling the C++ compiler performs to make calling functions type-safe
//and to allow overloading of functions.
//For example the function name ~init~ might look like
//~init\_int23\@myclass\@qbe\_ev\_abc~ or something similar strange. So
//finding the function isn't an easy task, but there is a way out of the
//problem when there exists at least *one* C call protected from name
//mangling with ~extern C~ in the dynamic library that returns a pointer
//to a class instance. Clever as it is this ~trick~ solves only part of
//the problem since the calling program wont actually have any knowledge
//about the object, unless there is a common parent class from which the
//object class in the dynamic library is inherited. Although one is limited
//to calling those methods on the returned objects that had definitions in
//the base class, it still allows a great deal of flexibility through the
//use of virtual methods.
//
//Implementing C++ classes in a DLL usually only works when used within a
//C++ application built with the same compiler due to name mangling and heap
//memory management. Beware of mixing run time libraries since one might end
//up with heap corruption; application and DLL should both use the same debug
//versus non debug run time libraries.
//
//1.2 Interface methods
//
//This module offers the following routines to manipulate dynamic libraries:
//
//[23]    Creation/Removal      & Library loading & Information retrieval \\
//        [--------]
//        DynamicLibrary        & Load     & GetFunctionAddress  \\
//        [tilde]DynamicLibrary & Unload   & GetLibraryName      \\
//                              & IsLoaded & GetLastErrorMessage \\
//
//1.3 Class "DynamicLibrary"[1]
//
//The class ~DynamicLibrary~ implements a portable wrapper to the operating
//system specific mechanisms to load libraries at runtime.

#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

#include "SysToolConfig.h"

#include <map>

using namespace std;

class SYSTOOL_EXPORT DynamicLibrary
{
public:

	//initializes the ~DynamicLibrary~ object.
	//Once the object is constructed, use the method ~Load~
	//to dynamically link to a dynamic library while the process is running.
	DynamicLibrary();

	//destroys an instance of ~DynamicLibrary~
	//The method ~Unload~ is call implicitly.
	virtual ~DynamicLibrary();

	//loads a dynamic library ~libraryName~ into a process while it is
	//running (dynamic linking).
	//The method returns ~true~ on success, otherwise ~false~.
	//On failure use method ~GetLastErrorMessage~ to get an error message.
	bool Load( const string& libraryName );

	//unloads a dynamic library from a process while it is running.
	//The method returns ~true~ on success, otherwise ~false~.
	//On failure use method ~GetLastErrorMessage~ to get an error message.
	bool Unload();

	//returns true if a dynamic library is currently loaded into the process.
	bool IsLoaded() const;

	//returns the name of the currently loaded dynamic library.
	//An empty string is returned when no library is loaded.
	string GetLibraryName() const;

	//finds the function named ~functionName~ and returns a function pointer to it.
	//If the function cannot be found, a null pointer is returned.
	void* GetFunctionAddress( const string& functionName );

	//returns the error message text of the last failed class method.
	//An empty string is returned when no error occurred.
	//The internal message buffer is emptied.
	string GetLastErrorMessage();

 protected:
#ifdef SYSTOOL_WIN32
  HANDLE libraryHandle; // Handle of library
#else
  void* libraryHandle;
#endif
  string libName;       // Name of currently loaded library
  string errorMessage;  // Error message text
 private:

	//is used to create an error message when one of dynamic library system calls failed.
	//The method ~GetLastErrorMessage~ returns this message to the user on request.

	void SetErrorMessage();

  // Do not use the following functions.
  DynamicLibrary( const DynamicLibrary& other );
  int operator==( const DynamicLibrary& other ) const;
  DynamicLibrary& operator=( const DynamicLibrary& other);
};

class SYSTOOL_EXPORT DynamicLibraryFactory
{
 public:
   static DynamicLibrary* GetLibrary( const string& libraryName );
 private:
  DynamicLibraryFactory();
  ~DynamicLibraryFactory();

  DynamicLibrary* FindLibrary( const string& libraryName );

  static DynamicLibraryFactory* GetFactory();

  map<string,DynamicLibrary*> m_libraryMap;

  static DynamicLibraryFactory* ms_factory;
};

#endif // DYNAMIC_LIBRARY_H

