#include "XMLWrapper/XMLCache.h"
#include "XMLWrapper/XMLDocument.h"
 
#ifdef _WINDOWS
#include "XMLWrapper/WindowsHelper.h"
#endif


XMLCache * XMLCache::m_Instance;
XMLCache * XMLCache::GetInstance ()
{
	// Implement the XML Cache as a singleton for sake of ease of use.

	// Check if we already have an instance
	if (m_Instance == NULL) {

		// We don't. Create instance
		m_Instance = new XMLCache();
	}

	// Return pointer to instance.
	return m_Instance;
}

XMLCache::XMLCache ()
{
	m_Instance = NULL;
	m_SchemaInitialized = false;
	m_SchemaNamespace = "http://www.pk-sim.com/SimModelSchema";//for backwards compatibility

#ifdef _WINDOWS
	m_Windows_SchemaCache = NULL;
#endif
#ifdef linux
	m_Linux_SchemaCache = NULL;
#endif
}

XMLCache::~XMLCache ()
{
	m_Instance = NULL;

#ifdef _WINDOWS
	if (m_Windows_SchemaCache)
	{
		m_Windows_SchemaCache.Release();
	}
#endif
#ifdef linux
    if (m_Linux_SchemaCache)
    {
      xmlSchemaFree(m_Linux_SchemaCache);
      m_Linux_SchemaCache = NULL;
    }
    xmlSchemaCleanupTypes();
#endif
}

void XMLCache::LoadSchemaFromXMLString (const std::string & sXML)
{
	try
	{
		// Create XML DOM for schema
		XMLDocument pXMLDoc = XMLDocument::FromString(sXML);
		assert(!pXMLDoc.IsNull());

		//Load DOM Schema
		LoadSchemaFromXMLDom(pXMLDoc);
		pXMLDoc.Release();
	}
	catch(ErrorData &)
	{
		throw;
	}
#ifdef _WINDOWS
	catch(_com_error & e)
	{
		throw ErrorData(ErrorData::ED_ERROR,
			            "XMLCache::LoadSchemaFromXMLString ("+SourceFromComError(e)+")",
		                DescriptionFromComError(e));
	}
#endif
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR,"XMLCache::LoadSchemaFromXMLString",
		                 "Unknown Error occured during loading from the XML string");
	}
}

void XMLCache::LoadSchemaFromFile (const std::string & sFileName)
{
	try
	{
		// Create XML DOM for schema
		XMLDocument pXMLDoc = XMLDocument::FromFile(sFileName);
		assert(!pXMLDoc.IsNull());

		//Load DOM Schema
		LoadSchemaFromXMLDom(pXMLDoc);
		pXMLDoc.Release();
	}
	catch(ErrorData &)
	{
		throw;
	}
#ifdef _WINDOWS
	catch(_com_error & e)
	{
		throw ErrorData(ErrorData::ED_ERROR,
			            "XMLCache::LoadSchemaFromFile ("+SourceFromComError(e)+")",
		                DescriptionFromComError(e));
	}
#endif
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR,"XMLCache::LoadSchemaFromFile",
		                 "Unknown Error occured during loading from the XML string");
	}
}

bool XMLCache::SchemaInitialized () const
{
	return m_SchemaInitialized;
}

void XMLCache::LoadSchemaFromXMLDom (XMLDocument pXMLDoc)
{
#ifdef _WINDOWS
    assert(!pXMLDoc.IsNull());

    m_Windows_SchemaCache.CreateInstance(__uuidof(MSXML2::XMLSchemaCache60));
    m_Windows_SchemaCache->add(m_SchemaNamespace.c_str(), pXMLDoc.m_Windows_DocumentPtr.GetInterfacePtr());
    m_SchemaInitialized = true;
#endif
#if linux
    xmlSchemaParserCtxtPtr ctxt;
    ctxt = xmlSchemaNewDocParserCtxt(pXMLDoc.m_Linux_DocumentPtr);
    xmlSchemaSetParserErrors(ctxt,
        (xmlSchemaValidityErrorFunc) fprintf,
        (xmlSchemaValidityWarningFunc) fprintf,
        stderr);
    m_Linux_SchemaCache = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    m_SchemaInitialized = true;

#endif
}

void XMLCache::SetSchemaNamespace(std::string schemaNamespace)
{
	m_SchemaNamespace = schemaNamespace;
}

const XMLCache::LocalSchemaType XMLCache::GetSchemaCache () const
{
#ifdef _WINDOWS
    return m_Windows_SchemaCache;
#endif

#ifdef linux
	// No Schema implementation yet.
	return m_Linux_SchemaCache;
#endif
}

