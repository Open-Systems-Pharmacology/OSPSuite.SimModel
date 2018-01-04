#ifndef _XMLDocument_H_
#define _XMLDocument_H_

#include <string>

#include "XMLWrapper/XMLNode.h"

#ifdef _WINDOWS
#import <msxml6.dll> named_guids
#include <msxml.h>
#endif

#ifdef linux
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#endif

class XMLWRAPPER_EXPORT XMLDocument
{

	public:
		XMLDocument();
		std::string GetBaseName ();
		XMLNode GetRootElement () const;
		const bool IsNull () const;
		void Create();
		void Release ();

		XMLNode CreateNode(const std::string& nodeName);
		XMLNode CreateRootNode(const std::string& rootNodeName);
        XMLNode CreateRootNode(const XMLNode& rootNode);

		static XMLDocument FromString (const std::string & mcrXML);
		const std::string ToString () const;
		static XMLDocument FromFile (const std::string & mcrFilename);
		void ToFile (const std::string & mcrFilename, bool indent = true) const;

#ifdef _WINDOWS

	private:
		MSXML2::IXMLDOMDocument2Ptr m_Windows_DocumentPtr;

#endif

#ifdef linux
		xmlDocPtr m_Linux_DocumentPtr;
#endif

		friend class XMLHelper;
		friend class XMLCache;
};

#endif //_XMLDocument_H_

