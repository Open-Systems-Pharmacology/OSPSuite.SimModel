#ifndef _ObjectBase_H_
#define _ObjectBase_H_

#include "SimModel/GlobalConstants.h"
#include "SimModel/XMLLoader.h"

#include <string>

namespace SimModelNative
{

class Simulation;

class ObjectBase : 
	public XMLLoader
{	
	protected:
		//object id - unique for all quantities all over the simulation
		long _id;

		//object id as string - used in descriptions etc.
		std::string _idAsString;

		//Entity ID produced by caller
		std::string _entityId;
	public:
		ObjectBase ();
		SIM_EXPORT long GetId(void);
		std::string GetEntityId();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
};

}//.. end "namespace SimModelNative"

#endif //_ObjectBase_H_
