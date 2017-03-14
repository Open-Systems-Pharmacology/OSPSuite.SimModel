#include "SysToolConfig.h"
#include "DynamicLibrary.h"

#ifndef SYSTOOL_WIN32
#include <dlfcn.h>
#endif

using namespace std;

DynamicLibrary::DynamicLibrary()
  : libraryHandle( 0 ), libName( "" ), errorMessage( "" )
{
}

DynamicLibrary::~DynamicLibrary()
{
  Unload();
}

bool
DynamicLibrary::Load( const string& libraryName )
{
  bool ok = false;
  if ( libraryName.length() > 0  && !IsLoaded() )
  {
#ifdef SYSTOOL_WIN32
    libraryHandle = ::LoadLibrary( libraryName.c_str() );
#else
    string name = "lib" + libraryName + ".so";
    libraryHandle = ::dlopen( name.c_str(), RTLD_LAZY | RTLD_GLOBAL );
#endif
    libName = libraryName;
    ok = (libraryHandle != 0);
    if ( !ok )
    {
      SetErrorMessage();
    }
  }
  else
  {
    errorMessage = (IsLoaded()) ?
                   "Another library ("+libName+") is already loaded." :
                   "No library name specified.";
  }
  return ok;
}

bool
DynamicLibrary::Unload()
{
  bool ok = false;
  if ( IsLoaded() )
  {
#ifdef SYSTOOL_WIN32
    BOOL ok_fl;
    ok_fl = ::FreeLibrary( static_cast<HINSTANCE>(libraryHandle) );
    ok = (ok_fl != 0);
    libraryHandle = 0;
#else
    ok = ::dlclose( libraryHandle ) == 0;
    libraryHandle = 0;
#endif
    if ( !ok )
    {
      SetErrorMessage();
    }
  }
  else
  {
    errorMessage = "No library is loaded.";
  }
  return (ok);
}

bool
DynamicLibrary::IsLoaded() const
{
  return (libraryHandle != 0);
}

string
DynamicLibrary::GetLibraryName() const
{
  if ( IsLoaded() )
  {
    return (libName);
  }
  else
  {
    return ("");
  }
}

void*
DynamicLibrary::GetFunctionAddress( const string& functionName )
{
  void* functionAddr = 0;
  if ( IsLoaded() )
  {
#ifdef SYSTOOL_WIN32
    functionAddr = ::GetProcAddress( static_cast<HINSTANCE>(libraryHandle),
                                     functionName.c_str());
#else
    functionAddr = ::dlsym( libraryHandle, functionName.c_str() );
#endif
    if ( functionAddr == 0 )
    {
      SetErrorMessage();
    }
  }
  else
  {
    errorMessage = "No library is loaded.";
  }
  return (functionAddr);
}

string
DynamicLibrary::GetLastErrorMessage()
{
  string errmsg = errorMessage;
  errorMessage = "";
  return (errmsg);
}

void
DynamicLibrary::SetErrorMessage()
{
  char* msgBuffer;
#ifdef SYSTOOL_WIN32
  ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                   (LPTSTR) &msgBuffer, 0, NULL );
  errorMessage = msgBuffer;
  LocalFree( msgBuffer );
#else
  msgBuffer = ::dlerror();
  errorMessage = (msgBuffer != 0) ? msgBuffer : "";
#endif
}

// -- DynamicLibraryFactory

DynamicLibraryFactory* DynamicLibraryFactory::ms_factory = NULL;

DynamicLibraryFactory::DynamicLibraryFactory()
{

}

DynamicLibraryFactory::~DynamicLibraryFactory()
{
  map<string,DynamicLibrary*>::iterator iter;
  for (iter = m_libraryMap.begin(); iter != m_libraryMap.end(); iter++)
  {
    if (iter->second != NULL)
    {
    	DynamicLibrary * library = iter->second;

    	if (library != NULL)
    	{
    		delete library;
    	}
      iter->second = NULL;
    }
  }
}

DynamicLibrary*
DynamicLibraryFactory::GetLibrary( const string& libraryName )
{
  return GetFactory()->FindLibrary(libraryName);
}

DynamicLibrary*
DynamicLibraryFactory::FindLibrary( const string& libraryName )
{
  DynamicLibrary* library = NULL;
  map<string,DynamicLibrary*>::iterator iter = m_libraryMap.find(libraryName);
  if (iter != m_libraryMap.end())
  {
    library = iter->second;
    if (!library->IsLoaded())
    	library->Load(libraryName);
  }
  else
  {
    library = new DynamicLibrary();
    library->Load(libraryName);
    m_libraryMap[libraryName] = library;
  }
  return library;
}

DynamicLibraryFactory*
DynamicLibraryFactory::GetFactory()
{
  if (ms_factory == NULL)
  {
    static DynamicLibraryFactory factory;
    ms_factory = &factory;
  }
  return ms_factory;
}

  map<string,DynamicLibrary*> m_libraryMap;
