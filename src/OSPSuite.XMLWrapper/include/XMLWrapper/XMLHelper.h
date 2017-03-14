#ifndef _XMLHelper_H_
#define _XMLHelper_H_


#ifdef WIN32

	#ifdef ReleaseX64
	#import <msxml6.dll> named_guids
	#else
	#import <msxml4.dll> named_guids
	#include <msxml.h>
	#endif

#endif

#include <string>

// Windows only
#ifdef WIN32
// Disable warning about truncation of type name to 255 characters
#pragma warning(disable:4786)
#endif

#include "XMLWrapper/XMLDocument.h"
#include "XMLWrapper/XMLNode.h"

class XMLCache;

class XMLWRAPPER_EXPORT XMLHelper
{
	public:
		static XMLNode GetModelNodeFromFile (const std::string & sFileName, const XMLCache * pCache);
		static XMLNode GetModelNodeFromString (const std::string & sModelXML, const XMLCache * pCache);

#ifdef WIN32
		typedef MSXML2::IXMLDOMParseErrorPtr XMLParseErrorPtr;
#endif

#ifdef linux

		// Not used
		typedef void * XMLParseErrorPtr;

#endif

		static std::string GetParseError (XMLParseErrorPtr pError);
		static double ToDouble (const std::string & sDouble);

	private:
		static int ToInt (const std::string & sInt);
		static bool ToBool (const std::string & sBool);

	public:
		static std::string Trim (const std::string & s);
		static std::string ToUpper (const std::string & Source);
		static std::string ToString (double dValue);
		static void ValidateXMLDomWithSchema (XMLDocument pXMLDoc, const XMLCache * pCache);
		static std::string StringReplace (const std::string & source, const char * find, const char * replace, bool CaseSensitive = true);
		static std::string StringReplace (const std::string & source, const std::string & find, const std::string & replace, bool CaseSensitive = true);
		static bool IsNumeric (const std::string & aString);
};

#endif //_XMLHelper_H_

