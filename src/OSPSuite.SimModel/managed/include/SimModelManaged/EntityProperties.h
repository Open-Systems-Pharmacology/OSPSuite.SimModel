#ifndef _EntityProperties_H_
#define _EntityProperties_H_

namespace SimModelNET
{
	//public interface
	public interface class IEntityProperties
    {
		property System::String^ EntityId
		{
			System::String^ get();
		}

		property System::String^ Path
		{
			System::String^ get();
		}

		property System::String^ Name
		{
			System::String^ get();
		}

    };

	ref class EntityProperties : public IEntityProperties
	{
	protected:
		System::String^ _entityId;
		System::String^ _path;
		System::String^ _name;

	internal:
		EntityProperties(System::String^ entityId, System::String^ path, System::String^ name);
	public:
		property System::String^ EntityId
		{
			virtual System::String^ get();
		}

		property System::String^ Path
		{
			virtual System::String^ get();
		}

		property System::String^ Name
		{
			virtual System::String^ get();
		}
	};
}

#endif //_ParameterProperties_H_

