#ifndef __SimModelComp_XMLHelper_h_
#define __SimModelComp_XMLHelper_h_

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4251)
#pragma warning( disable : 4279)
#pragma warning( disable : 4005)
#pragma warning( disable : 4996)
#endif

#include <string>
#include <vector>

#include "XMLWrapper/XMLDocument.h"

#ifdef WIN32
#import <msxml6.dll> named_guids
#include <msxml.h>

#endif

using namespace std;

class SimModelComp_XMLHelper
{
public:
	SimModelComp_XMLHelper();
	virtual ~SimModelComp_XMLHelper();

	bool XMLStringFromFile(const string & FileName, string & XMLString,string & ErrorMsg);
	XMLDocument GetXMLDomFromXMLString (const string & sXMLString);
	bool TagHasName (const XMLNode& pNode, const string & sTagName);
	XMLNode AddXMLNode (XMLDocument& XMLDoc, const string & sNodeName, XMLNode& ParentNode);
	void AddXMLAttribute (XMLDocument& XMLDoc, XMLNode& oNode, const string & sAttrName, const string & sValue);
	string GetNodeAttrValue (const XMLNode& pNode, const string & sAttrName, string sDefaultValue = "");
	void SaveStringToFile(const std::string & str, const string & FileName);

private:
	bool FileExists(const string & FileName, string & ErrMsg);

};

#endif
