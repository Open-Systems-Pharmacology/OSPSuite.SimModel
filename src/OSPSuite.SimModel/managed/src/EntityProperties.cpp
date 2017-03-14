#include "SimModelManaged/EntityProperties.h"

namespace SimModelNET
{
	EntityProperties::EntityProperties(System::String^ entityId, System::String^ path, System::String^ name)
	{
		_entityId = entityId;
		_path = path;
		_name = name;
	}
    
	System::String^ EntityProperties::EntityId::get()
	{
		return _entityId;
	}

	System::String^ EntityProperties::Path::get()
	{
		return _path;
	}

	System::String^ EntityProperties::Name::get()
	{
		return _name;
	}

}
