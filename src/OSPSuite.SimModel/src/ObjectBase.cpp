#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "SimModel/ObjectBase.h"
#include "SimModel/Simulation.h"
#include "XMLWrapper/XMLHelper.h"

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

ObjectBase::ObjectBase ()
{
	_id = INVALID_QUANTITY_ID;
}

long ObjectBase::GetId(void)
{
	return _id;
}

void ObjectBase::LoadFromXMLNode (const XMLNode & pNode)
{
	_id = (long)pNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);
	
	_idAsString = XMLHelper::ToString(_id);

	_entityId = pNode.GetAttribute(XMLConstants::EntityId, "");
}

void ObjectBase::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
}

std::string ObjectBase::GetEntityId()
{
	if (_entityId == "")
		return _idAsString;

	return _entityId;
}

}//.. end "namespace SimModelNative"
