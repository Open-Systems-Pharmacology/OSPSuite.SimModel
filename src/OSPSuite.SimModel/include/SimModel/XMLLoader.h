#ifndef _XMLLoader_H_
#define _XMLLoader_H_

#ifdef WIN32
#pragma warning( disable : 4251)
#pragma warning( disable : 4279)
#endif

#include "XMLWrapper/XMLNode.h"

namespace SimModelNative
{

class Simulation;

class XMLLoader
{
	public:
	virtual ~XMLLoader()
	{
	}

	virtual void LoadFromXMLNode (const XMLNode & pNode) = 0;
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim) = 0;
};

}//.. end "namespace SimModelNative"

#endif //_XMLLoader_H_

