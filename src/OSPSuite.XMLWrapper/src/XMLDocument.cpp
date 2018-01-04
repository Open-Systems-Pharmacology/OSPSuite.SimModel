#include <string>
#include "XMLWrapper/XMLDocument.h"
#include "XMLWrapper/XMLNode.h"
#include "XMLWrapper/XMLHelper.h"
#include "ErrorData.h"

#ifdef _WINDOWS
#include "XMLWrapper/WindowsHelper.h"
#endif

#ifdef linux
static const char* gs_xmlEncoding = "UTF-8";
#endif

XMLDocument::XMLDocument()
{
#ifdef _WINDOWS
	m_Windows_DocumentPtr = NULL;
#endif

#ifdef linux
	m_Linux_DocumentPtr = NULL;
#endif

}
std::string XMLDocument::GetBaseName ()
{
	// Return value
	std::string sRet="";

#ifdef _WINDOWS
	// ============================================= WINDOWS
	sRet = "At element: " + _bstr_t(m_Windows_DocumentPtr->GetbaseName());
#endif

#ifdef linux
	// ============================================= LINUX
	//TODO
#endif

	// Return root node
	return sRet;
}

XMLNode XMLDocument::GetRootElement () const
{
	// Return value
	XMLNode ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	ret.m_Windows_NodePtr = m_Windows_DocumentPtr -> firstChild;
#endif

#ifdef linux
	// ============================================= LINUX
	ret.m_Linux_NodePtr = xmlDocGetRootElement(m_Linux_DocumentPtr);
#endif

	// Return root node
	return ret;
}

const bool XMLDocument::IsNull () const
{
#ifdef _WINDOWS
	// ============================================= WINDOWS
	return (m_Windows_DocumentPtr == NULL);
#endif

#ifdef linux
	// ============================================= LINUX
	return (m_Linux_DocumentPtr == NULL);
#endif
}

void XMLDocument::Create()
{
#ifdef _WINDOWS
    // ============================================= WINDOWS
  MSXML2::IXMLDOMDocument2Ptr XMLDoc=NULL;
  HRESULT hr=CoCreateInstance(MSXML2::CLSID_DOMDocument60, NULL, CLSCTX_INPROC_SERVER,
                              MSXML2::IID_IXMLDOMDocument, (void**)&XMLDoc);
  if(FAILED(hr))
    throw "Creating XML Document failed";

  //Load file synchronously
  XMLDoc->async = VARIANT_FALSE;
  m_Windows_DocumentPtr = XMLDoc;
#endif

#ifdef linux
    // ============================================= LINUX
  m_Linux_DocumentPtr = xmlNewDoc(BAD_CAST "1.0");
  if(m_Linux_DocumentPtr == NULL)
    throw "Creating XML Document failed";
#endif
}

XMLNode XMLDocument::CreateNode(const std::string& nodeName)
{
  XMLNode node;
#ifdef _WINDOWS
  MSXML2::IXMLDOMNodePtr nodePtr = m_Windows_DocumentPtr->createNode((short) MSXML2::NODE_ELEMENT, nodeName.c_str(), "");
  node.m_Windows_NodePtr = nodePtr;
#endif
#ifdef linux
  node.m_Linux_NodePtr = xmlNewDocNode(m_Linux_DocumentPtr, NULL, BAD_CAST nodeName.c_str(), NULL);
#endif
  return node;
}

XMLNode XMLDocument::CreateRootNode(const std::string& rootNodeName)
{
  XMLNode rootNode = CreateNode(rootNodeName);
#ifdef _WINDOWS
  throw "Not implemented yet";
#endif
#ifdef linux
  xmlDocSetRootElement(m_Linux_DocumentPtr, rootNode.m_Linux_NodePtr);
#if 0
  // TODO: SaveToFile created duplicate processing instruction when the following lines are executed
  // TODO: Check whether setting the encoding on saving to file is sufficient
  xmlNodePtr pi = xmlNewDocPI(m_Linux_DocumentPtr, BAD_CAST "xml", BAD_CAST "version=\"1.0\" encoding=\"ISO-8859-1\"");
  xmlAddPrevSibling(m_Linux_DocumentPtr->children, pi);
#endif
#endif
  return rootNode;
}

XMLNode XMLDocument::CreateRootNode(const XMLNode& rootNode)
{
#ifdef _WINDOWS
  throw "Not implemented yet";
#endif
#ifdef linux
  xmlDocSetRootElement(m_Linux_DocumentPtr, rootNode.m_Linux_NodePtr);
#endif
  return rootNode;
}

void XMLDocument::Release ()
{
#ifdef _WINDOWS
	// ============================================= WINDOWS
	m_Windows_DocumentPtr.Release();
	m_Windows_DocumentPtr = NULL;
#endif

#ifdef linux
	// ============================================= LINUX
	xmlFreeDoc(m_Linux_DocumentPtr);
	m_Linux_DocumentPtr = NULL;
#endif
}

XMLDocument XMLDocument::FromString (const std::string & mcrXML)
{
	// Return value
	XMLDocument ret;

	try
	{
#ifdef _WINDOWS
		// ========================================================== WINDOWS

		// Create XML DOM for schema
		MSXML2::IXMLDOMDocument2Ptr pXMLDoc;
		HRESULT hr = pXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));

		if (FAILED(hr))
		{
			std::string errMsg = DescriptionFromHResult(hr);
			throw ErrorData(ErrorData::ED_ERROR, "XMLDocument::FromString", "Unable To Create xmldom: "+errMsg);
		}

		if (pXMLDoc == NULL)
			throw ErrorData(ErrorData::ED_ERROR, "XMLDocument::FromString", "Unable To Create xmldom");

		// Load/Save the document synchronously
		pXMLDoc -> async = false;

		//Convert string to bstr
		BSTR bstrXML;
		STDStringToBStr(mcrXML,bstrXML);

		// Read XML file
		if (pXMLDoc -> loadXML(bstrXML)!=VARIANT_TRUE)
		{
			SysFreeString(bstrXML);
			std::string sErr = "Loading of XML String failed:" + XMLHelper::GetParseError(pXMLDoc->parseError);
			throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromString",sErr);
		}

		SysFreeString(bstrXML);

		ret.m_Windows_DocumentPtr = pXMLDoc;
#endif

#ifdef linux
		// ========================================================== LINUX

		// Read document from memory
		ret.m_Linux_DocumentPtr =
			xmlReadMemory(mcrXML.c_str(), mcrXML.size(), "noname.xml", NULL, XML_PARSE_NOBLANKS | XML_PARSE_XINCLUDE);

		// Did that work?
		if (ret.m_Linux_DocumentPtr == NULL)
		{
			const std::string sErr = "Interpreting of XML string failed.";
			throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromString",sErr);
		}
#endif
	}
	catch(ErrorData & ED)
	{
		throw ED;
	}
#ifdef _WINDOWS
	catch(_com_error & e)
	{
		throw ErrorData(ErrorData::ED_ERROR,
			            "XMLDocument::FromString ("+SourceFromComError(e)+")",
		                DescriptionFromComError(e));
	}
#endif
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromString",
		                 "Unknown Error occured during loading from the XML string");
	}

	return ret;
}

const std::string XMLDocument::ToString () const
{
#ifdef _WINDOWS
	// ============================================= WINDOWS
	return (char *) m_Windows_DocumentPtr -> Getxml();
#endif

#ifdef linux
	// ============================================= LINUX
	xmlChar * mem;
	int size;
	xmlDocDumpFormatMemoryEnc(m_Linux_DocumentPtr, &mem, &size, gs_xmlEncoding, 0);

	std::string ret = (char *) mem;
	xmlFree(mem);

	return ret;
#endif
}

XMLDocument XMLDocument::FromFile (const std::string & mcrFilename)
{
	// Return value
	XMLDocument ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	try
	{
		// Create XML DOM
		MSXML2::IXMLDOMDocument2Ptr doc;

		HRESULT hr = doc.CreateInstance(__uuidof(MSXML2::DOMDocument60));

		if (FAILED(hr))
		{
			std::string errMsg = DescriptionFromHResult(hr);
			throw ErrorData(ErrorData::ED_ERROR, "XMLDocument::FromFile", "Unable To Create xmldom: " + errMsg);
		}

		if (doc == NULL)
		{
			throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromFile","Unable to Create xmldom");
		}

		// Load/Save the document synchronously
		doc -> async = VARIANT_FALSE;

		//Convert string to bstr
		BSTR bstrXMLFile;
		STDStringToBStr(mcrFilename, bstrXMLFile);

		// Read XML file
		if (doc -> load(bstrXMLFile) != VARIANT_TRUE)
		{
			SysFreeString(bstrXMLFile);
			std::string sErr =
				"Loading of XML file \"" + mcrFilename + std::string("\" failed: \n") +
				XMLHelper::GetParseError(doc->parseError);
			throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromFile",sErr);
		}

		SysFreeString(bstrXMLFile);
		ret.m_Windows_DocumentPtr = doc;
	}

	catch(ErrorData & ED)
	{
		throw ED;
	}
#ifdef _WINDOWS
	catch(_com_error & e)
	{
		throw ErrorData(ErrorData::ED_ERROR,
			            "XMLDocument::FromFile ("+SourceFromComError(e)+")",
		                DescriptionFromComError(e));
	}
#endif

	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromFile",
			"Unknown Error occured during loading from the XML file'" + mcrFilename + "'");
	}
#endif

#ifdef linux
	// ============================================= LINUX

	// Load XML document
	ret.m_Linux_DocumentPtr = xmlReadFile(mcrFilename.c_str(), NULL, XML_PARSE_NOBLANKS | XML_PARSE_XINCLUDE);

	// Did that work?
	if (ret.m_Linux_DocumentPtr == NULL)
	{
		const std::string sErr =
			"Loading of XML file \"" + mcrFilename + std::string("\" failed.");
		throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::FromFile",sErr);
	}
#endif

	// Return document
	return ret;
}

void XMLDocument::ToFile (const std::string & mcrFilename, bool indent) const
{
#ifdef _WINDOWS
	// ============================================= WINDOWS

	// Convert filename to bstr
	BSTR bstrXMLFile;
	STDStringToBStr(mcrFilename, bstrXMLFile);

	// Write XML file
	if ((m_Windows_DocumentPtr -> load(bstrXMLFile)) != VARIANT_TRUE)
	{
		// Didn't work for some reason.
		SysFreeString(bstrXMLFile);
		std::string sErr = "Saving XML file \"" + mcrFilename + std::string("\" failed.");
		throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::ToFile",sErr);
	}
#endif

#ifdef linux
	// ============================================= LINUX
	int rc = 0;
    // Save XML document
	if (indent)
	{
      int saveKeepBlanks = xmlKeepBlanksDefault(0);
      rc = xmlSaveFormatFileEnc(mcrFilename.c_str(), m_Linux_DocumentPtr, gs_xmlEncoding, 1);
      xmlKeepBlanksDefault(saveKeepBlanks);
	}
	else
	{
      rc = xmlSaveFileEnc(mcrFilename.c_str(), m_Linux_DocumentPtr, gs_xmlEncoding);
    }

    // Did that work?
    if (rc < 0)
    {
        const std::string sErr =
            "Saving of XML file \"" + mcrFilename + std::string("\" failed.");
        throw ErrorData(ErrorData::ED_ERROR,"XMLDocument::ToFile",sErr);
    }
#endif
}
