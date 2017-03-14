//1 Header File: File System Management
//
//1.1 Overview
//
//The *File System Management* provides several services for handling files and
//folders (directories) in an operating system independent manner. There are
//functions for inspecting and manipulating the folder (directory) tree. No
//functions for file access are provided.
//
//1.3 Interface methods
//
//The class ~FileSystem~ provides the following methods:
//
//[23]    Folder Management & Files and Folders & File management \\
//        [--------]
//        GetCurrentFolder & FileOrFolderExists & CopyFile          \\
//        SetCurrentFolder & RenameFileOrFolder & GetFileAttributes \\
//        CreateFolder     & DeleteFileOrFolder & SetFileAttributes \\
//        EraseFolder      & FileSearch         &      \\
//                         & AppendSlash        &      \\
//
//1.4 Imports, Constants, Types
//

#ifndef FILESYSTEM_H
#define FILESYSTEM_H 1

#include "SysToolConfig.h"
#include <string>
#include <vector>

using namespace std;

//type for the file attributes for a specific file.
// *NOTE*: The values of the file attributes are operating system dependent.
//One has to keep this in mind when implementing portable applications.
#ifdef SYSTOOL_WIN32
typedef DWORD FileAttributes;
#else
typedef uint32_t FileAttributes;
#endif

//type for a collection of filenames found by a File Search.
typedef vector<string> FilenameList;

//type of user-supplied functions for filename filtering. The
//function arguments are:
//  * ~absolutePath~ -- Directory where file resides.
//  * ~fileName~ -- Filename without directory.
//  * ~attribs~ -- File attributes.
typedef bool (*FileSearchCallbackFunc)
          ( const string& absolutePath,
            const string& fileName,
            FileAttributes attribs );

//1.3 Class "FileSystem"[1]
//
//This class implements all functions for the file system management as static
//members. Since the constructor is private the class cannot be instantiated.
class SYSTOOL_EXPORT FileSystem
{
 public:

	//returns the current folder (directory).
	static string GetCurrentFolder();

	//sets the current folder (directory) to ~folder~.
	//The function returns ~true~, if the current folder could be set.
	static bool SetCurrentFolder( const string& folder );

	//creates the folder (directory) located at ~folder~.
	//The function returns ~true~, if the folder could be created.
	static bool CreateFolder( const string& folder );

	//deletes the file or folder (directory) specified in ~fileName~.
	//The function returns ~true~, if the file or folder could be deleted.
	//
	//The function fails if the file is protected by file attributes or
	//if the folder to be removed contains one or more files.
	static bool DeleteFileOrFolder( const string& fileName );

	//removes the folder (directory) specified in ~folder~.
	//The function returns ~true~, if the remove operation succeeded.
	//
	//This function makes every attempt to delete the folder (directory), such as
	//removing file protection attributes and files contained within folders (directories).

	//~maxLevels~ controls how many levels of subfolders the
	//function will traverse in order to remove a folder (directory).

	// *NOTE*: When ~EraseFolder~ deletes multiple files, files will be deleted until
	//an error occurs. Any files that were successfully deleted before the error
	//occurred will not be restored. This situation typically occurs if
	//the user does not have permission to remove a file.
	static bool EraseFolder( const string& folder,
                           uint16_t maxLevels = 16 );

	//renames (moves) a file or folder (directory) from ~currentName~ to ~newName~.
	//The function returns ~true~, if the copy operation succeeded.
	static bool RenameFileOrFolder( const string& currentName,
                                  const string& newName );

	//copies a file from ~source~ to ~dest~.
	//The function returns ~true~, if the copy operation succeeded.
	//
	// *NOTE*: On Unix systems this function may be used to copy folders (directories)
	//as well. Keep in mind that this property is not portable.
	static bool CopyFile( const string& source,
                                const string& dest );

	//checks for the existence of the file indicated by ~fileName~.
	//The function returns ~true~, if the file exists.
	static bool FileOrFolderExists( const string& fileName );

	//returns the file attributes for the file ~fileName~.
	//In case of an error the function returns 0.
	static FileAttributes GetFileAttributes( const string&
                                             fileName );

	//sets the file attributes for a file to the values specified in ~attribs~ .
	//The function returns ~true~, if the attributes could be set.
	static bool SetFileAttributes( const string& fileName,
                                 FileAttributes attribs );

	//returns a list of filenames which meet the search criteria in ~filenameList~.
	//~folder~ indicates the folder (directory) where the search begins.
	//
	//If ~searchName~ is specified, the list will only contain the
	//files matching this name. Wildcard searches are currently *not* supported,
	//but the callback function ~fileSearchCallback~ can be used to filter the
	//filenames.
	//
	//~maxLevels~ controls how many levels of subdirectories will be searched.
	//
	//~includeFolders~ specifies whether subfolder names are to be included in the
	//list of filenames.
	//
	//If ~fullPath~ is true, the complete pathname of each file will be returned.
	static bool FileSearch( const string& folder,
                          FilenameList& filenameList,
                          const string* searchName = 0,
                          uint16_t maxLevels = 1,
                          bool includeFolders = true,
                          bool fullPath = true,
                          FileSearchCallbackFunc
                            fileSearchCallback = 0 );

	//searches the file ~fileName~ on the path and returns ~true~ if the file was
	//found, otherweise ~false~. If the file was found the complete pathname of
	//the file is returned in ~foundFile~.
	static bool SearchPath( const string& fileName, string& foundFile );

	//appends the proper slash character to a pathname.
	//This character will either be a forward or backward
	//slash, depending on the operating system used.
	static void AppendSlash( string& pathName );

 protected:
 private:
#ifdef SYSTOOL_WIN32

	 //removes file protection attributes from a file, so that
	 //it may be modified or deleted.
	 //
	 // *NOTE*: This function is available in Windows only.
	static void UnprotectFile( const string& fileName );

#endif

//The following functions are not implemented and must never be used.

  FileSystem() {};
  ~FileSystem() {};
  FileSystem( const FileSystem& other );
  FileSystem& operator=( const FileSystem& other );
  int operator==( const FileSystem& other ) const;
};

#endif // FILESYSTEM_H

