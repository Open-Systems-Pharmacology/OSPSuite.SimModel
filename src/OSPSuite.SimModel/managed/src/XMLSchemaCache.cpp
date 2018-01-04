#include "SimModelManaged/Conversions.h"
#include "SimModelManaged/XMLSchemaCache.h"
#include "SimModelManaged/ExceptionHelper.h"
#include "SimModel/GlobalConstants.h"

#ifdef _WINDOWS
#pragma warning( disable : 4279)
#endif

#include "XMLWrapper/XMLCache.h"

namespace SimModelNET
{
	void XMLSchemaCache::InitializeFromFile(System::String^ schemaFile)
	{
		try
		{
			XMLCache * xmlCache = XMLCache::GetInstance();
			xmlCache->SetSchemaNamespace(SimModelNative::XMLConstants::SimModelSchema);
			xmlCache->LoadSchemaFromFile(NETToCPPConversions::MarshalString(schemaFile));

			_schemaFile=schemaFile;
		}
		catch(ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
	}

	void XMLSchemaCache::InitializeFromString(System::String^ schemaString)
	{
		try
		{
			XMLCache * xmlCache = XMLCache::GetInstance();
			xmlCache->LoadSchemaFromXMLString(NETToCPPConversions::MarshalString(schemaString));
		}
		catch(ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
	}

	bool XMLSchemaCache::IsInitialized::get()
	{
		bool initialized = false;

		try
		{
			XMLCache * xmlCache = XMLCache::GetInstance();
			initialized = xmlCache->SchemaInitialized();
		}
		catch(ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}

		return initialized;
	}

	System::String^ XMLSchemaCache::SchemaFile()
	{
		return _schemaFile;
	}
}
