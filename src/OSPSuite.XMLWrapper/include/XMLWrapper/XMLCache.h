#ifndef _XMLCache_H_
#define _XMLCache_H_

#include "ErrorData.h"
#include "XMLWrapper/XMLHelper.h"
#include <cmath>
#include <assert.h>

#if defined(linux) || defined (__APPLE__)
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#endif


class XMLCache
{
	private:
		bool m_SchemaInitialized;
		static XMLCache * m_Instance;
		std::string m_SchemaNamespace;

	public:
		XMLWRAPPER_EXPORT static XMLCache * GetInstance ();
		XMLWRAPPER_EXPORT XMLCache ();
		XMLWRAPPER_EXPORT virtual ~XMLCache ();
		XMLWRAPPER_EXPORT void LoadSchemaFromXMLString (const std::string & sXML);
		XMLWRAPPER_EXPORT void LoadSchemaFromFile (const std::string & sFileName);
		XMLWRAPPER_EXPORT bool SchemaInitialized () const;
		XMLWRAPPER_EXPORT void SetSchemaNamespace(std::string schemaNamespace);

#ifdef _WINDOWS
		typedef MSXML2::IXMLDOMSchemaCollectionPtr LocalSchemaType ;

	private:
		MSXML2::IXMLDOMSchemaCollectionPtr m_Windows_SchemaCache;

#endif
		void LoadSchemaFromXMLDom (XMLDocument pXMLDoc);


#if defined(linux) || defined (__APPLE__)
		typedef xmlSchemaPtr LocalSchemaType;

	private:
	    xmlSchemaPtr m_Linux_SchemaCache;

#endif

	public:
		const LocalSchemaType GetSchemaCache () const;
};

#endif //_XMLCache_H_

