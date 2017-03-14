#include "SimModelComp/SimModelComp_XMLHelper.h"
#include <fstream>
#include <iostream>

#ifdef WIN32
#include <codecvt>
#define uint16_t UINT16
#define strcasecmp _stricmp
#endif

#include "FileSystem.h"
#if linux
#include <sys/stat.h>
#include <string.h>
#endif

#ifdef WIN32
//forward declarations
string GetParseError (MSXML2::IXMLDOMParseErrorPtr pError);
HRESULT STDStringToBStr(string s, BSTR &bstr, UINT codePage = CP_ACP);
#endif

//constructor
SimModelComp_XMLHelper::SimModelComp_XMLHelper()
{
}

SimModelComp_XMLHelper::~SimModelComp_XMLHelper()
{
}


bool SimModelComp_XMLHelper::XMLStringFromFile(const string & FileName,
											   string & XMLString,
											   string & ErrorMsg)
{
	bool RetVal = false;
	const string ErrorSource = "\nin SimModelComp_XMLHelper::XMLStringFromFile";
	XMLDocument pXMLDoc;
	try
	{
		//check if file exists
		if (!FileExists(FileName, ErrorMsg))
			throw ErrorMsg;

		pXMLDoc = XMLDocument::FromFile(FileName);

		// Read XML file
		if (pXMLDoc.IsNull())
			throw "Loading of XML file \"" + FileName + "\" failed.";
		
		XMLString = pXMLDoc.ToString();

		if (!pXMLDoc.IsNull())
			pXMLDoc.Release();

		RetVal = true;
	}
	catch(const string & msg)
	{
		if (!pXMLDoc.IsNull())
			pXMLDoc.Release();
		ErrorMsg = msg + ErrorSource;
	}
	catch(const char * msg)
	{
		if (!pXMLDoc.IsNull())
			pXMLDoc.Release();
		ErrorMsg = msg + ErrorSource;
	}
	catch(...)
	{
		if (!pXMLDoc.IsNull())
			pXMLDoc.Release();
		ErrorMsg = "Unknown Error " + ErrorSource;
	}

	return RetVal;
}

bool SimModelComp_XMLHelper::FileExists(const string & FileName, string & ErrMsg)
{
  bool ok = true;
  ErrMsg = "";

  if (FileSystem::FileOrFolderExists(FileName))
  {
#ifdef WIN32
/*
	DWORD dwFileAttr = GetFileAttributes((LPCTSTR)FileName.c_str());
	if (dwFileAttr == INVALID_FILE_ATTRIBUTES)
	{
	    ErrMsg = "File \"" + FileName + "\" ";

		DWORD dwFileError = GetLastError();

		if (dwFileError == ERROR_FILE_NOT_FOUND)
			ErrMsg += "not found";
		else if (dwFileError == ERROR_PATH_NOT_FOUND)
			ErrMsg += "not found";
		else if (dwFileError == ERROR_ACCESS_DENIED)
			ErrMsg += "- access denied";
		else if (dwFileError == ERROR_TOO_MANY_OPEN_FILES)
			ErrMsg += "could not be opened: too many opened files";
		else
			ErrMsg += "not found";
		ok = false;
	}
	else if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
      ErrMsg = "File \"" + FileName + "\" could not be opened: it's not a file - it's a directory!";
      ok = false;
	}
*/
#endif
#ifdef linux
    FileAttributes fileAttr = FileSystem::GetFileAttributes(FileName);
    if (fileAttr != 0)
    {
      if (S_ISDIR(fileAttr))
      {
        ErrMsg = "File \"" + FileName + "could not be opened: it's not a file - it's a directory!";
        ok = false;
      }
    }
    else
    {
      ErrMsg = "File \"" + FileName + "- access denied";
      ok = false;
    }
#endif
  }
  else
  {
    ErrMsg = "File \"" + FileName + "\" not found.";
    ok = false;
  }
  return ok;
}


XMLDocument SimModelComp_XMLHelper::GetXMLDomFromXMLString (const string & sXMLString)
{
    XMLDocument pXMLDoc;

	const string ERROR_SOURCE = "\nin XMLHelper::GetXMLDomFromXMLString";
	try
	{
      // Read XML file
	  pXMLDoc = XMLDocument::FromString(sXMLString);
	  return pXMLDoc;
	}
	catch(const string & )
	{
		throw;
	}
	catch(...)
	{
		throw "Unknown Error occured during loading from the XML string"+ERROR_SOURCE;
	}
}

bool SimModelComp_XMLHelper::TagHasName (const XMLNode& pNode, const string & sTagName)
{
	if (pNode.IsNull()) return false;
	return strcasecmp(sTagName.c_str(), pNode.GetNodeName().c_str()) == 0;
}

XMLNode SimModelComp_XMLHelper::AddXMLNode (XMLDocument& XMLDoc, const string & sNodeName, XMLNode& ParentNode /*= NULL*/)
{
	XMLNode node = XMLDoc.CreateNode(sNodeName);

	if (!ParentNode.IsNull())
	{
	  ParentNode.AppendChildNode(node);
	}

	return node;
}

void SimModelComp_XMLHelper::AddXMLAttribute (XMLDocument& XMLDoc, XMLNode& oNode, const string & sAttrName, const string & sValue)
{
  oNode.SetAttribute(sAttrName, sValue);
}


string SimModelComp_XMLHelper::GetNodeAttrValue (const XMLNode& pNode, const string & sAttrName, string sDefaultValue /*= ""*/)
{
  return pNode.GetAttribute (sAttrName,sDefaultValue);
}

#ifdef WIN32
std::string to_utf8(const wchar_t* buffer, int len)
{
	int nChars = ::WideCharToMultiByte(
		CP_UTF8,
		0,
		buffer,
		len,
		NULL,
		0,
		NULL,
		NULL);
	if (nChars == 0) return "";

	string newbuffer;
	newbuffer.resize(nChars);
	::WideCharToMultiByte(
		CP_UTF8,
		0,
		buffer,
		len,
		const_cast< char* >(newbuffer.c_str()),
		nChars,
		NULL,
		NULL);

	return newbuffer;
}

std::string to_utf8(const std::wstring& str)
{
	return to_utf8(str.c_str(), (int)str.size());
}
#endif

void SimModelComp_XMLHelper::SaveStringToFile(const std::string & str, const string & FileName)
{
	string ErrorMsgPrefix = "Saving file '" + FileName + "' failed: ";

#ifdef WIN32		
	if (FileSystem::FileOrFolderExists(FileName))
	{
		if (!FileSystem::DeleteFileOrFolder(FileName))
			throw ErrorMsgPrefix+"old file could not be deleted";
	}

	ofstream outf(FileName.c_str()); 

	if (outf.is_open())
    {
		//just writing std::string to file destroys UTF8 encoding
		//implemented workaround via wstring as found here: 
		//http://mariusbancila.ro/blog/2008/10/20/writing-utf-8-files-in-c/

		wstring wstr(str.begin(), str.end());
		string utf8string = to_utf8(wstr);
		outf << utf8string;

		outf.close();
    }
	else
		throw ErrorMsgPrefix+"file could not be opened"+strerror(errno);
#endif
//TODO Linux not encrypted	
}
