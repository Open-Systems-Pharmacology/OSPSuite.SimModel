#include "SysToolConfig.h"

#ifdef SYSTOOL_WIN32
#  include <io.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <cstring>
#endif

#include "FileSystem.h"

using namespace std;

#ifdef SYSTOOL_WIN32

//~attrMask~ is used to remove file attributes from a file, so that the file may be deleted.
const FileAttributes attrMask = ( FILE_ATTRIBUTE_HIDDEN   |
                                  FILE_ATTRIBUTE_READONLY |
                                  FILE_ATTRIBUTE_SYSTEM );
#endif

//~currentFolder~ and ~parentFolder~ are filenames that should never be added to file lists for a folder.
const static char* currentFolder = ".";
const static char* parentFolder  = "..";

string
FileSystem::GetCurrentFolder()
{
  char str[500];
  string folder = "";
#ifdef SYSTOOL_WIN32
  int count = ::GetCurrentDirectory( 497, str );
  if ( count > 0 && count < 498 )
  {
    folder = string( str );
  }
#else
  char* buf = ::getcwd( str, 499 );
  if ( buf == str )
  {
    folder = buf;
  }
#endif
  return (folder);
}

bool
FileSystem::SetCurrentFolder( const string& folder )
{
#ifdef SYSTOOL_WIN32
  return (::SetCurrentDirectory( folder.c_str() ) != 0);
#else
  return (chdir( folder.c_str() ) ==0);
#endif
}

bool
FileSystem::CreateFolder( const string& folder )
{
#ifdef SYSTOOL_WIN32
  return (::CreateDirectory( folder.c_str(), NULL ) != 0);
#else
  return (::mkdir( folder.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) == 0);
#endif
}

bool
FileSystem::DeleteFileOrFolder( const string& fileName )
{
  FileAttributes fileAttribs = GetFileAttributes( fileName );
#ifdef SYSTOOL_WIN32
  int isFolder = (fileAttribs & FILE_ATTRIBUTE_DIRECTORY);
#else
  int isFolder = S_ISDIR( fileAttribs );
#endif
   // Is this a folder (directory) or file?
  if ( isFolder )
  {
    // Remove the directory.
#ifdef SYSTOOL_WIN32
    return (::RemoveDirectory( fileName.c_str() ) != 0);
#else
    return (::rmdir( fileName.c_str() ) == 0);
#endif
  }
  else
  {
    // Delete the file
#ifdef SYSTOOL_WIN32
    return (::DeleteFile( fileName.c_str() ) != 0);
#else
    return (::unlink( fileName.c_str()) == 0);
#endif
  }
}

bool
FileSystem::EraseFolder( const string& folder, uint16_t maxLevels )
{
#ifdef SYSTOOL_WIN32
  // Determine file type
  FileAttributes fileAttribs = GetFileAttributes( folder );
  if ( fileAttribs & FILE_ATTRIBUTE_DIRECTORY )
  {
    // Remove the folder (directory):
    // (1) Get the list of filenames within the folder (directory).
    FilenameList filenameList;
    FileSearch( folder, filenameList, 0, maxLevels );

    // (2) Remove all files (and subfolders) in the folder
    vector<string>::const_iterator iter = filenameList.begin();
    while ( iter != filenameList.end() )
    {
      // Remove any file protection attributes.
      vector<string>::const_reference filename = *iter;
      UnprotectFile( filename );
      DeleteFileOrFolder( filename );
      iter++;
    }
  }
  // Remove any file protection attributes.
  UnprotectFile( folder );
  // Remove the requested folder.
  return (DeleteFileOrFolder( folder ));
#else
  string command = "rm -rf " + folder;
  int syserr = ::system( command.c_str() );
  return (syserr != 127 && syserr != -1);
#endif
}

bool
FileSystem::RenameFileOrFolder( const string& currentName,
                                const string& newName )
{
#ifdef SYSTOOL_WIN32
  return (::MoveFile( currentName.c_str(), newName.c_str() ) != 0);
#else
  string command = "mv " + currentName + " " + newName;
  int syserr = ::system( command.c_str() );
  return (syserr != 127 && syserr != -1);
#endif
}

bool
FileSystem::CopyFile( const string& source, const string& dest )
{
#ifdef SYSTOOL_WIN32
  return (::CopyFile( source.c_str(), dest.c_str(), FALSE ) != 0);
#else
  string command = "cp " + source + " " + dest;
  int syserr = ::system( command.c_str() );
  return (syserr != 127 && syserr != -1);
#endif
}

bool
FileSystem::FileOrFolderExists( const string& fileName )
{
  return (access( fileName.c_str(), 0 ) != -1);
}

FileAttributes
FileSystem::GetFileAttributes( const string& fileName )
{
  FileAttributes attribs = 0;
#ifdef SYSTOOL_WIN32
  attribs = ::GetFileAttributes( fileName.c_str() );
  if ( attribs == reinterpret_cast<FileAttributes>(INVALID_HANDLE_VALUE) )
    attribs = 0;
#else
  struct stat filestatus;
  int rc = ::lstat( fileName.c_str(), &filestatus );
  if ( rc == 0 )
    attribs = filestatus.st_mode;
  else
    attribs = 0;
#endif
  return (attribs);
}

bool
FileSystem::SetFileAttributes( const string& fileName, FileAttributes attribs )
{
#ifdef SYSTOOL_WIN32
  return (::SetFileAttributes( fileName.c_str(), attribs ) != 0);
#else
  return (::chmod( fileName.c_str(), attribs ) != -1);
#endif
}

bool
FileSystem::FileSearch( const string& folder,
                        FilenameList& filenameList,
                        const string* searchName,
                        uint16_t maxLevels,
                        bool includeFolders,
                        bool fullPath,
                        FileSearchCallbackFunc fileSearchCallback )
{
  // Save the current folder and change to folder where the search starts.
  string oldFolder = GetCurrentFolder();
  SetCurrentFolder( folder );

  // Get absolute pathname of the current folder.
  string absoluteFolder = GetCurrentFolder();
  if ( maxLevels ) maxLevels--;

#ifdef SYSTOOL_WIN32
  WIN32_FIND_DATA findData = { 0 };
  HANDLE findHandle = ::FindFirstFile( "*", &findData );

  if ( findHandle == INVALID_HANDLE_VALUE )
  {
    SetCurrentFolder( oldFolder );
    return false;
  }

  // Add file entries to FileList.
  do
  {
    // Skip the entries for the current folder and the parent folder
    if ( strcmp( findData.cFileName, currentFolder ) &&
         strcmp( findData.cFileName, parentFolder ))
    {
      string pathName = absoluteFolder;
      AppendSlash( pathName );
      pathName += findData.cFileName;

	//The names of files in subfolders are inserted into the list before
	//the names of subfolders are added. This ensures that folders are empty
	//when the entries in the list are used in ascending index sequence to delete each file.

      int isFolder = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
      if ( maxLevels && isFolder )
      {
        if ( !FileSearch( pathName, filenameList, searchName,
                          maxLevels, includeFolders, fullPath,
                          fileSearchCallback ) )
        {
          SetCurrentFolder( oldFolder );
          return false;
        }
      }

      // Subfolders will only be added to the list if requested

      if ( !isFolder || (isFolder && includeFolders) )
      {
        if ( !searchName ||
            (searchName && *searchName == string( findData.cFileName )) )
        {
          if ( !fileSearchCallback ||
              (fileSearchCallback &&
               fileSearchCallback( absoluteFolder, findData.cFileName,
                                   findData.dwFileAttributes )) )
          {
            if ( fullPath )
            {
              filenameList.push_back( pathName );
            }
            else
            {
              filenameList.push_back( findData.cFileName );
            }
          }
        }
      }
    }
    if ( !::FindNextFile( findHandle, &findData ) )
    {
      DWORD nexterr = ::GetLastError();
      if ( nexterr == ERROR_NO_MORE_FILES )
      {
        // Last entry was found, get out of the loop
        break;
      }
      else
      {
        SetCurrentFolder( oldFolder );
        return false;
      }
    }
  }
  while ( true );

  if ( !::FindClose( findHandle ) )
  {
    SetCurrentFolder( oldFolder );
    return false;
  }
#else
  DIR* dir = ::opendir( absoluteFolder.c_str() );
  if ( dir )
  {
    dirent* dirEntry = 0;
    while ( (dirEntry = ::readdir( dir )) != 0 )
    {
      if ( strcmp( dirEntry->d_name, currentFolder ) == 0 ) continue;
      if ( strcmp( dirEntry->d_name, parentFolder ) == 0 )  continue;
      string pathName = absoluteFolder;
      AppendSlash( pathName );
      pathName += dirEntry->d_name;
      FileAttributes fileAttribs = GetFileAttributes( pathName );

	//The names of files in subfolders are inserted into the list before
	//the names of subfolders are added. This ensures that folders are empty
	//when the entries in the list are used in ascending index sequence to delete each file.

      int isFolder = S_ISDIR( fileAttribs );
      if ( maxLevels && isFolder )
      {
        FileSearch( pathName, filenameList, searchName,
                    maxLevels, includeFolders, fullPath,
                    fileSearchCallback );
      }

      // Subfolders will only be added to the list if requested

      if ( S_ISLNK( fileAttribs ) == 0 && 
          (!isFolder || (isFolder && includeFolders)) )
      {
        if ( !searchName ||
            (searchName && *searchName == string(dirEntry->d_name)) )
        {
          if ( !fileSearchCallback ||
              (fileSearchCallback &&
               fileSearchCallback( absoluteFolder, dirEntry->d_name, fileAttribs )) )
          {
            if ( fullPath )
            {
              filenameList.push_back( pathName );
            }
            else
            {
              filenameList.push_back( dirEntry->d_name );
            }
          }
        }
      }
    }
    ::closedir( dir );
  }
#endif
  // Restore current directory.
  SetCurrentFolder( oldFolder );
  return true;
}

bool
FileSystem::SearchPath( const string& fileName, string& foundFile )
{
  bool ok = false;
#ifdef SYSTOOL_WIN32
  char buffer[MAX_PATH];
  char* filepart;
  if ( ::SearchPath( NULL, fileName.c_str(), NULL, MAX_PATH, buffer, &filepart ) == 0 )
  {
    if ( ::SearchPath( NULL, fileName.c_str(), ".exe", MAX_PATH, buffer, &filepart ) != 0 )
    {
      foundFile = buffer;
      ok = true;
    }
  }
  else
  {
    foundFile = buffer;
    ok = true;
  }
#else
  if ( fileName[0] == PATH_SLASH[0] )
  {
    // file name is fully qualified
    foundFile = fileName;
    ok = FileOrFolderExists( fileName );
  }
  else
  {
    // 1. Search in current directory
    string cwd = GetCurrentFolder();
    AppendSlash( cwd );
    foundFile = cwd + fileName;
    ok = FileOrFolderExists( foundFile );
    if ( !ok )
    {
      // 2. Search path if file name was not partially qualified
      if ( fileName.find( PATH_SLASH ) != std::string::npos )
      {
        char* envPath = getenv( "PATH" );
        if ( envPath != 0 )
        {
          string path;
          string pathRest = envPath;
          string::size_type delim;
          while ( pathRest.length() > 0 )
          {
            delim = pathRest.find( ":" );
            if ( delim != string::npos )
            {
              path = pathRest.substr( 0, delim-1 );
              pathRest = pathRest.substr( delim+1 );
            }
            else
            {
              path = pathRest;
              pathRest = "";
            }
            AppendSlash( path );
            foundFile = path + fileName;
            ok = FileOrFolderExists( foundFile );
            if ( ok ) break;
          }
        }
      }
    }
  }
#endif
  if ( !ok )
  {
    foundFile = "";
  }
  return (ok);
}

void
FileSystem::AppendSlash( string& pathName )
{
  if ( pathName.length() )
  {
    size_t idx = pathName.length()-1;
    if ( pathName[idx] != PATH_SLASH[0] )
    {
      pathName += PATH_SLASH;
    }
  }
}

#ifdef SYSTOOL_WIN32
void
FileSystem::UnprotectFile( const string& fileName )
{
  FileAttributes fileAttribs = GetFileAttributes( fileName );
  if ( fileAttribs & attrMask )
  {
    // File has attributes that must be cleared before it may be deleted.
    fileAttribs &= ~attrMask;
    SetFileAttributes( fileName, fileAttribs );
  }
}
#endif

