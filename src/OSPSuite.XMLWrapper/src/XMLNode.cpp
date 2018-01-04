#include "ErrorData.h"
#include "XMLWrapper/XMLNode.h"
#include "XMLWrapper/XMLHelper.h"

#ifdef _WINDOWS
#include "XMLWrapper/WindowsHelper.h"
#endif

XMLNode::XMLNode ()
{
	// Initialize as NULL pointer

#ifdef _WINDOWS
	// ============================================= WINDOWS
	m_Windows_NodePtr = NULL;
#endif

#ifdef linux
	// ============================================= LINUX
	m_Linux_NodePtr = NULL;
#endif
}

XMLNode XMLNode::GetFirstChild ()
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetFirstChild",
        "Trying to access empty XML node.");
  }
	// Return value
	XMLNode ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	ret.m_Windows_NodePtr = m_Windows_NodePtr -> firstChild;
#endif

#ifdef linux
	// ============================================= LINUX
	ret.m_Linux_NodePtr = m_Linux_NodePtr -> children;
#endif

	// Return value
	return ret;
}

const XMLNode XMLNode::GetFirstChild () const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetFirstChild",
        "Trying to access empty XML node.");
  }
	// Return value
	XMLNode ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	ret.m_Windows_NodePtr = m_Windows_NodePtr -> firstChild;
#endif

#ifdef linux
	// ============================================= LINUX
	ret.m_Linux_NodePtr = m_Linux_NodePtr -> children;
#endif

	// Return value
	return ret;
}

XMLNode XMLNode::GetNextSibling ()
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetNextSibling",
        "Trying to access empty XML node.");
  }
	// Return value
	XMLNode ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	ret.m_Windows_NodePtr = m_Windows_NodePtr -> nextSibling;
#endif

#ifdef linux
	// ============================================= LINUX
	ret.m_Linux_NodePtr = m_Linux_NodePtr -> next;
#endif

	// Return value
	return ret;
}

const XMLNode XMLNode::GetNextSibling () const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetNextSibling",
        "Trying to access empty XML node.");
  }
	// Return value
	XMLNode ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	ret.m_Windows_NodePtr = m_Windows_NodePtr -> nextSibling;
#endif

#ifdef linux
	// ============================================= LINUX
	ret.m_Linux_NodePtr = m_Linux_NodePtr -> next;
#endif

	// Return value
	return ret;
}

const std::string XMLNode::GetNodeName () const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetNodeName",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
	// ============================================= WINDOWS
	return ((char *) m_Windows_NodePtr -> nodeName);
#endif

#ifdef linux
	// ============================================= LINUX
	return ((char *) m_Linux_NodePtr -> name);
#endif
}

const bool XMLNode::IsNull () const
{
#ifdef _WINDOWS
	// ============================================= WINDOWS
	return (m_Windows_NodePtr == NULL);
#endif

#ifdef linux
	// ============================================= LINUX
	return (m_Linux_NodePtr == NULL);
#endif
}

XMLNode XMLNode::Clone(bool recursive) const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::Clone",
        "Trying to access empty XML node.");
  }
  XMLNode nodeCopy;
#ifdef _WINDOWS
  // ============================================= WINDOWS
  // TODO: is a deep copy necessary?
  // For now just copy the pointer
  nodeCopy.m_Windows_NodePtr = m_Windows_NodePtr;
#endif

#ifdef linux
    // ============================================= LINUX
    int extended = (recursive) ? 1 : 2;
    nodeCopy.m_Linux_NodePtr = xmlCopyNode(m_Linux_NodePtr, extended);
#endif

    return nodeCopy;
}

void XMLNode::FreeNode()
{
  if (!IsNull())
  {
#ifdef linux
    // ============================================= LINUX
    xmlFreeNode(m_Linux_NodePtr);
    m_Linux_NodePtr = NULL;
#endif
  }
}

bool XMLNode::HasAttribute (const std::string & mcrName) const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::HasAttribute",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
    // ============================================= WINDOWS
    // Get attribute pointer
    const MSXML2::IXMLDOMAttributePtr attribute =
        m_Windows_NodePtr -> Getattributes() -> getNamedItem(mcrName.c_str());

    // Check if attribute could be found
    return (attribute != NULL);
#endif

#ifdef linux
    // ============================================= LINUX

    // Check if attribute exists
    return xmlHasProp(m_Linux_NodePtr, BAD_CAST mcrName.c_str());
#endif
}

const std::string XMLNode::GetAttribute (const std::string & mcrName, const std::string & mcrDefault /*= ""*/) const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetAttribute",
        "Trying to access empty XML node.");
  }
	// Return value
	std::string ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	// Get attribute pointer
	const MSXML2::IXMLDOMAttributePtr attribute =
		m_Windows_NodePtr -> Getattributes() -> getNamedItem(mcrName.c_str());

	// Check if attribute could be found
	if (attribute)
	{
		// Get content
		ret = BStrToSTDString(attribute -> nodeValue.bstrVal);
	} else
	{
		// Set default
		ret = mcrDefault;
	}
#endif

#ifdef linux
	// ============================================= LINUX

	// Check if attribute exists
	if (xmlHasProp(m_Linux_NodePtr, BAD_CAST mcrName.c_str()))
	{
		char * attr = (char *) xmlGetProp(m_Linux_NodePtr, BAD_CAST mcrName.c_str());
		ret = attr;
		xmlFree(attr);
	} else
	{
		ret = mcrDefault;
	}
#endif

	// Return value
	return ret;
}

const double XMLNode::GetAttribute (const std::string & mcrName, const double mcDefault) const
{
	// Get attribute value as string
	const std::string value = GetAttribute(mcrName, "");

	// Check for default value
	if (value == "")
	{
		return mcDefault;
	}

	// Convert to double
	return XMLHelper::ToDouble(value);
}

void XMLNode::SetAttribute (const std::string & mcrName, const std::string & mcrValue)
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::SetAttribute",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
	// ============================================= WINDOWS

	// Get document
	MSXML2::IXMLDOMDocument * document;
	m_Windows_NodePtr -> get_ownerDocument(&document);

	// Did that work?
	if (document == NULL)
	{
		// Nope. This is an error.
		throw ErrorData(ErrorData::ED_ERROR, "XMLNode::SetAttribute",
			"Cannot get DOM document pointer");
	}

	// Create attribute
	MSXML2::IXMLDOMAttributePtr attr = document -> createAttribute(mcrName.c_str());

	// Set attribute value
	attr -> nodeTypedValue = mcrValue.c_str();
	m_Windows_NodePtr -> attributes -> setNamedItem(attr);
#endif

#ifdef linux
	// ============================================= LINUX
    // Check if attribute exists
    if (xmlHasProp(m_Linux_NodePtr, BAD_CAST mcrName.c_str()))
    {
      xmlSetProp(m_Linux_NodePtr, BAD_CAST mcrName.c_str(), BAD_CAST mcrValue.c_str());
    }
    else
    {
      xmlNewProp(m_Linux_NodePtr, BAD_CAST mcrName.c_str(), BAD_CAST mcrValue.c_str());
    }
#endif
}

void XMLNode::SetAttribute (const std::string & mcrName, const double mcValue)
{
	// Independent of platform
	SetAttribute(mcrName, XMLHelper::ToString(mcValue));
}

const bool XMLNode::HasName (const std::string & mcrName) const
{
	// Valid for all platforms
	return (GetNodeName() == mcrName);
}

XMLNode XMLNode::GetChildNode (const std::string & mcrName)
{
#ifdef _WINDOWS
	// ============================================= WINDOWS
	// Check if node exists
	if (m_Windows_NodePtr == NULL)
	{
		// It does not.
		return XMLNode();
	}
#endif

#ifdef linux
	// ============================================= LINUX
	// Check if node exists
	if (m_Linux_NodePtr == NULL)
	{
		// It does not.
		return XMLNode();
	}
#endif

	// Loop all children for first match
	for (XMLNode child = GetFirstChild();
		!child.IsNull();
		child = child.GetNextSibling())
	{
		// Check if child has the correct name
		if (child.HasName(mcrName))
		{
			return child;
		}
	}

	// Not found
	return XMLNode();
}

const XMLNode XMLNode::GetChildNode (const std::string & mcrName) const
{
#ifdef _WINDOWS
	// ============================================= WINDOWS
	// Check if node exists
	if (m_Windows_NodePtr == NULL)
	{
		// It does not.
		return XMLNode();
	}
#endif

#ifdef linux
	// ============================================= LINUX
	// Check if node exists
	if (m_Linux_NodePtr == NULL)
	{
		// It does not.
		return XMLNode();
	}
#endif

	// Loop all children for first match
	for (XMLNode child = GetFirstChild();
		!child.IsNull();
		child = child.GetNextSibling())
	{
		// Check if child has the correct name
		if (child.HasName(mcrName))
		{
			return child;
		}
	}

	// Not found
	return XMLNode();
}

XMLNode XMLNode::CreateChildNode (const std::string & mcrName)
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::CreateChildNode",
        "Trying to access empty XML node.");
  }
	// Return value
	XMLNode ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS

	// Get document
	MSXML2::IXMLDOMDocument * document;
	m_Windows_NodePtr -> get_ownerDocument(&document);

	const long NODE_ELEMENT = 1;

	ret.m_Windows_NodePtr =
		document -> createNode(NODE_ELEMENT, mcrName.c_str(),
			m_Windows_NodePtr -> namespaceURI);

	// Append child node
	m_Windows_NodePtr -> appendChild(ret.m_Windows_NodePtr);
#endif

#ifdef linux
	// ============================================= LINUX
	ret.m_Linux_NodePtr = xmlNewChild(m_Linux_NodePtr, NULL, BAD_CAST mcrName.c_str(), BAD_CAST "");
#endif

	// Return child node
	return ret;
}

void XMLNode::RemoveChildNode (XMLNode & mrNode)
{
  if (mrNode.IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::SetValue",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
	// ============================================= WINDOWS
	m_Windows_NodePtr -> removeChild(mrNode.m_Windows_NodePtr);
#endif

#ifdef linux
	// ============================================= LINUX
	xmlUnlinkNode(mrNode.m_Linux_NodePtr);
#endif
}

void XMLNode::AppendChildNode (XMLNode & mrNode)
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::AppendChildNode",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
	// ============================================= WINDOWS
	m_Windows_NodePtr -> appendChild(mrNode.m_Windows_NodePtr);
#endif

#ifdef linux
	// ============================================= LINUX
	xmlAddChild(m_Linux_NodePtr, mrNode.m_Linux_NodePtr);
#endif
}

const double XMLNode::GetChildNodeValue (const std::string & mcrName, const double mcDefault) const
{
	// Try to get child node
	XMLNode node = GetChildNode(mcrName);

	// Did that work?
	if (node.IsNull())
	{
		// No. Use default value
		return mcDefault;
	}

	// Return child's value
	return node.GetValue(mcDefault);
}

const std::string XMLNode::GetChildNodeValue (const std::string & mcrName, const std::string & mcrDefault /*= ""*/) const
{
	// Try to get child node
	XMLNode node = GetChildNode(mcrName);

	// Did that work?
	if (node.IsNull())
	{
		// No. Use default value
		return mcrDefault;
	}

	// Return child's value
	std::string ret = node.GetValue();

	// Node is empty, return default value
	if (ret=="")
		ret = mcrDefault;

	return ret;
}

const std::string XMLNode::GetXML () const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetXML",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
	// ============================================= WINDOWS
	return (char *) m_Windows_NodePtr -> Getxml();
#endif

#ifdef linux
	// ============================================= LINUX

	// Create buffer
	xmlBufferPtr buffer = xmlBufferCreate();

	// Dump node to buffer
	xmlNodeDump(buffer, m_Linux_NodePtr -> doc, m_Linux_NodePtr, 0, 0);

	// Get content
	const std::string ret = (const char *) buffer -> content;

	// Free buffer
	xmlBufferFree(buffer);

	// Return content
	return ret;
#endif
}

const double XMLNode::GetValue (const double mcDefault) const
{
	// Return value
	double ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	try
	{
		ret = XMLHelper::ToDouble(GetValue());
	}
	catch(...)
	{
		ret = mcDefault;
	}
#endif

#ifdef linux
	// ============================================= LINUX
	const std::string value = GetValue();
	if (value == "")
	{
		ret = mcDefault;
	} else
	{
		ret = XMLHelper::ToDouble(value);
	}
#endif

	// Return value
	return ret;
}

const std::string XMLNode::GetValue () const
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::GetValue",
        "Trying to access empty XML node.");
  }

  // Return value
  std::string ret;

#ifdef _WINDOWS
	// ============================================= WINDOWS
	HRESULT hr;
	BSTR itemtext = NULL;

	// Get text and flag
	hr = m_Windows_NodePtr -> get_text(&itemtext);

	// Did we succeed?
	if (FAILED(hr))
	{
		// No. Empty text and fail.
		ret = "";
		SysFreeString(itemtext);
	} else
	{
		// Otherwise, return text
		ret = BStrToSTDString(itemtext);
		SysFreeString(itemtext);
	}
#endif

#ifdef linux
	// ============================================= LINUX

	// Loop children
	for (xmlNodePtr child = m_Linux_NodePtr -> children;
		 child != NULL;
		 child = child -> next)
	{
		// Check if current child is a text node
		if (child -> type != XML_TEXT_NODE)
		{
			// No. Do nothing
			continue;
		}

		// Otherwise, store its content
		ret = ret + (char *) child -> content;

		// Next child
	}
#endif

	// Remove leading/trailing whitespace
	ret = XMLHelper::Trim(ret);

	// Return correct value
	return ret;
}

void XMLNode::SetValue (const std::string & Value)
{
  if (IsNull())
  {
    throw ErrorData(ErrorData::ED_ERROR, "XMLNode::SetValue",
        "Trying to access empty XML node.");
  }
#ifdef _WINDOWS
	// ============================================= WINDOWS
	m_Windows_NodePtr -> text = Value.c_str();
#endif

#ifdef linux
	// ============================================= LINUX
	//TODO: Check
	// The following code sets only the node text content
	// while the Windows version sets also descendants (?)
	xmlChar* value = xmlEncodeSpecialChars(m_Linux_NodePtr->doc, (const xmlChar*) Value.c_str());
	xmlNodeSetContent(m_Linux_NodePtr, value);
	xmlFree(value);
#endif
}

void XMLNode::SetValue (double Value)
{
	// Independent of platform
	SetValue(XMLHelper::ToString(Value));
}

const bool XMLNode::operator == (const long mcRhs)
{
	// Never to be called anyway
	return false;
}

const bool XMLNode::operator != (const long mcRhs)
{
	// Never to be called anyway
	return false;
}

const bool XMLNode::operator == (const XMLNode & mcrRhs)
{
	// Never to be called anyway
	return false;
}

const bool XMLNode::operator != (const XMLNode & mcrRhs)
{
	// Never to be called anyway
	return false;
}
