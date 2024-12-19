#ifndef _XMLNode_H_
#define _XMLNode_H_

#include <string>

#ifdef _WINDOWS
#pragma warning( disable : 4251)
#pragma warning( disable : 4192)
#import <msxml6.dll> named_guids
#include <msxml.h>
#define XMLWRAPPER_EXPORT __declspec(dllexport)
#endif

#ifdef linux
#include <libxml/parser.h>
#include <libxml/tree.h>
#define XMLWRAPPER_EXPORT
#endif

#ifdef __APPLE__
#include <libxml/parser.h>
#include <libxml/tree.h>
#define XMLWRAPPER_EXPORT
#endif

class XMLWRAPPER_EXPORT XMLNode
{
	public:
		XMLNode ();

		XMLNode GetFirstChild ();
		const XMLNode GetFirstChild () const;
		XMLNode GetNextSibling ();
		const XMLNode GetNextSibling () const;
		const std::string GetNodeName () const;
		const bool IsNull () const;

		XMLNode Clone(bool recursive) const;
		void FreeNode();

		bool HasAttribute (const std::string & mcrName) const;
		const std::string GetAttribute (const std::string & mcrName, const std::string & mcrDefault = "") const;
		const double GetAttribute (const std::string & mcrName, const double mcDefault) const;
		void SetAttribute (const std::string & mcrName, const std::string & mcrValue);
		void SetAttribute (const std::string & mcrName, const double mcValue);
		const bool HasName (const std::string & mcrName) const;
		XMLNode GetChildNode (const std::string & mcrName);
		const XMLNode GetChildNode (const std::string & mcrName) const;
		XMLNode CreateChildNode (const std::string & mcrName);
		void RemoveChildNode (XMLNode & mrNode);
		void AppendChildNode (XMLNode & mrNode);
		const double GetChildNodeValue (const std::string & mcrName, const double mcDefault) const;
		const std::string GetChildNodeValue (const std::string & mcrName, const std::string & mcrDefault = "") const;
		const std::string GetXML () const;
		const double GetValue (const double mcDefault) const;
		const std::string GetValue () const;
		void SetValue (const std::string & Value);
		void SetValue (double Value);

#ifdef _WINDOWS

	private:
		MSXML2::IXMLDOMNodePtr m_Windows_NodePtr;

#endif

#ifdef linux
		//..end "UTF:----- Linux (Start)"
		xmlNodePtr m_Linux_NodePtr;

#endif

	private:
		const bool operator == (const long mcRhs);
		const bool operator != (const long mcRhs);
		const bool operator == (const XMLNode & mcrRhs);
		const bool operator != (const XMLNode & mcrRhs);

		friend class XMLDocument;
};

#endif //_XMLNode_H_

