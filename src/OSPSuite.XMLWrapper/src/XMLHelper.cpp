#include <sstream>

#include "ErrorData.h"
#include "XMLWrapper/XMLHelper.h"
#include "XMLWrapper/XMLCache.h"
#include "XMLWrapper/XMLNode.h"
#include "XMLWrapper/XMLDocument.h"
#include "SimModel/MathHelper.h"

// Windows only
#ifdef _WINDOWS
#include "XMLWrapper/WindowsHelper.h"
#pragma warning( disable : 4996)
#else
#include <cstring>
#endif

XMLNode XMLHelper::GetModelNodeFromFile (const std::string & sFileName, const XMLCache * pCache)
{
	// Create XML DOM
	XMLDocument pXMLDoc = XMLDocument::FromFile(sFileName);

#ifdef _WINDOWS
	//Validate schema
	ValidateXMLDomWithSchema(pXMLDoc,pCache);
#endif

 	// Get "<Model>" tag
	XMLNode pModelNode = pXMLDoc.GetRootElement();
	while (!pModelNode.IsNull() && !pModelNode.HasName("Model"))
		pModelNode = pModelNode.GetNextSibling();

	// If this didn't work for some reason...should not because of schema!
	if (pModelNode.IsNull())
	{
	  pXMLDoc.Release();
		throw ErrorData(ErrorData::ED_ERROR, "XMLHelper::GetModelNodeFromFile","Failed to find Model Node in XML File:" + sFileName);
	}

	// Create a deep copy of the model node
	XMLNode modelNodeCopy = pModelNode.Clone(true);
    pXMLDoc.Release();

	return modelNodeCopy;
}

XMLNode XMLHelper::GetModelNodeFromString (const std::string & sModelXML, const XMLCache * pCache)
{
	// Create XML DOM
	XMLDocument pXMLDoc = XMLDocument::FromString(sModelXML);

#ifdef _WINDOWS
	//Validate schema
	ValidateXMLDomWithSchema(pXMLDoc,pCache);
#endif

 	// Get "<Model>" tag
	XMLNode pModelNode = pXMLDoc.GetRootElement();
	while (!pModelNode.IsNull() && !pModelNode.HasName("Model"))
		pModelNode = pModelNode.GetNextSibling();

	// If this didn't work for some reason... should not because of schema!
	if (pModelNode.IsNull())
	{
	    pXMLDoc.Release();
		throw ErrorData(ErrorData::ED_ERROR, "XMLHelper::GetModelNodeFromFile","Failed to find Model Node in XML string");
	}

    XMLNode modelNodeCopy = pModelNode.Clone(true);
    pXMLDoc.Release();
	return modelNodeCopy;
}

std::string XMLHelper::GetParseError (XMLParseErrorPtr pError)
{
#ifdef _WINDOWS
	std::string sErr;
	sErr = "Error code: " + _bstr_t(pError->GeterrorCode()) + "\n";
	try
	{ //This calls might failed
		sErr += std::string("Url: ") + BStrToSTDString(pError->Geturl()) + "\n";
		sErr += std::string("Source Text: ") + BStrToSTDString(pError->GetsrcText()) + "\n";
    }
    catch(...)
    {
    	sErr = sErr;
    }
    sErr += "At line: " +  _bstr_t(pError->Getline()) + "\n";
    sErr += "At line Pos: " +  _bstr_t(pError->Getlinepos()) + "\n";
	sErr += std::string("Reason: ") + BStrToSTDString(pError->Getreason());
	return sErr;
#endif

#ifdef linux
	// !!! Not implemented yet
	return "Unspecified error (XMLHelper::GetParseError() not implemented yet for Linux)";
#endif
}

double XMLHelper::ToDouble (const std::string & sDouble)
{
	// Return value
	std::string s = Trim(sDouble);

	if (!IsNumeric(s))
		throw ErrorData(ErrorData::ED_ERROR, "XMLHelper::ToDouble", "Not Numeric:" + sDouble);

    const char * DecSep = localeconv()->decimal_point;

    if (strcmp(",", DecSep))
        s = XMLHelper::StringReplace(s, ",", DecSep);

    if (strcmp(".", DecSep))
        s = XMLHelper::StringReplace(s, ".", DecSep);

	 double value = 0.;
	 char* end;

    //******************************************************
	 value = strtod(s.c_str(), &end);

	 if (*end != '\0')
		  return SimModelNative::MathHelper::GetNaN();
	 //******************************************************

	// Return value
	return value;
}

int XMLHelper::ToInt (const std::string & sInt)
{
	const std::string s = Trim(sInt);
	if (!IsNumeric(s))
		throw ErrorData(ErrorData::ED_ERROR, "XMLHelper::ToInt", "Not Numeric:" + sInt);

	int value = 0;

	// String stream
	std::istringstream sink(s);

	// Read value from string stream
	sink >> value;

	// Return value
	return value;
}

bool XMLHelper::ToBool (const std::string & sBool)
{
	// Return value
	bool value = false;

	// String stream
	std::istringstream sink(sBool);

	// Read value from string stream
	sink >> value;

	// Return value
	return value;

}

std::string XMLHelper::Trim (const std::string & s)
{
	if(s.length() == 0)
		return s;

	size_t b = s.find_first_not_of(" \t");
	size_t e = s.find_last_not_of(" \t");
	if(b == -1) // No non-spaces
		return "";

	return std::string(s, b, e - b + 1);

}

std::string XMLHelper::ToUpper (const std::string & Source)
{
	const size_t strSize = Source.size();
	char * NewStr = new char[strSize+1]; //+1 for  '\0'
	const char *csource = Source.c_str();
	for(size_t i=0; i<strSize; i++)
		NewStr[i]=toupper(csource[i]);
	NewStr[strSize] = '\0';
	std::string newString = NewStr;
	delete[] NewStr;
	return newString;
}

std::string XMLHelper::ToString (double dValue)
{
	std::ostringstream out;
	out.precision(16);
	out<<dValue;

	return out.str();
}

void XMLHelper::ValidateXMLDomWithSchema (XMLDocument pXMLDoc, const XMLCache * pCache)
{
#ifdef _WINDOWS
	// ===================================================== WINDOWS

	// ********************************************************
	// WE need to use IXMLDOMDocument2 interface with schema
	// ********************************************************
    assert(!pXMLDoc.IsNull());
	assert(pCache!=NULL);
	assert(pCache -> GetSchemaCache() != NULL);

	//Error Parser
	MSXML2::IXMLDOMParseErrorPtr pError = NULL;

    pXMLDoc.m_Windows_DocumentPtr -> schemas = pCache -> GetSchemaCache().GetInterfacePtr();

    //validate XML with schema
    pError = pXMLDoc.m_Windows_DocumentPtr -> validate();
    if(pError->errorCode!= S_OK)
    {
       	std::string sErr = std::string("Validate of XML file \"") + BStrToSTDString(pXMLDoc.m_Windows_DocumentPtr->Geturl());
       	sErr+= "\" failed:\n" + pXMLDoc.GetBaseName() + "\n" + XMLHelper::GetParseError(pError);
       	pError.Release();
		throw ErrorData(ErrorData::ED_ERROR,"XMLHelper::ValidateXMLDomWithSchema",sErr);
    }
    pError.Release();
#endif
#if linux
    assert(!pXMLDoc.IsNull());
    assert(pCache!=NULL);
    assert(pCache -> GetSchemaCache() != NULL);

    xmlSchemaValidCtxtPtr ctxt;
    ctxt = xmlSchemaNewValidCtxt(pCache->GetSchemaCache());
    xmlSchemaSetValidErrors(ctxt,
        (xmlSchemaValidityErrorFunc) fprintf,
        (xmlSchemaValidityWarningFunc) fprintf,
        stderr);
    int ret;
    ret = xmlSchemaValidateDoc(ctxt, pXMLDoc.m_Linux_DocumentPtr);
    if (ret != 0)
    {

    }
    xmlSchemaFreeValidCtxt(ctxt);
#endif
}

std::string XMLHelper::StringReplace (const std::string & source, const char * find, const char * replace, bool CaseSensitive /*= true*/)
{
	size_t iPos=0;
	std::string str=source;

	if(!find)
		return str;

	char * _find = new char[strlen(find)+1];
	strcpy(_find, find);

	if(!CaseSensitive)
	{
		for(size_t i=0; i<strlen(find); i++)
			_find[i] = toupper(_find[i]);
	}

	iPos=str.find_first_of(_find[0]);
	while(iPos != std::string::npos)
	{
		std::string substr = str.substr(iPos, strlen(_find));
		if (!CaseSensitive)
			substr=ToUpper(substr);
		if (substr == _find)
		{
			str.replace(iPos, strlen(_find), replace);
		}
		else
			iPos++;
		iPos=str.find_first_of(_find[0],iPos);
	}

	delete[] _find;
	return str;
}

std::string XMLHelper::StringReplace (const std::string & source, const std::string & find, const std::string & replace, bool CaseSensitive /*= true*/)
{
	return XMLHelper::StringReplace(source,find.c_str(), replace.c_str(), CaseSensitive);
}

bool XMLHelper::IsNumeric (const std::string & aString)
{
    if (aString=="") return false;

    //replace point and comma with current decimal separator USED BY C++ RUNTIME LIBRARY
    //This decimal separator is used by strtod function while converting strings to
    //double numbers
    std::string NewString = aString;
    const char * DecSep = localeconv()->decimal_point;

    if (strcmp(",", DecSep))
        NewString = XMLHelper::StringReplace(NewString, ",", DecSep);

    if (strcmp(".", DecSep))
        NewString = XMLHelper::StringReplace(NewString, ".", DecSep);

    const char *p = NewString.c_str();
    char *pend = 0;

    strtod( p, &pend );

    return (*pend == '\0');
}

