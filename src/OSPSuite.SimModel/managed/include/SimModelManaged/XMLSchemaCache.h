#ifndef _XMLSchemaCache_H_
#define _XMLSchemaCache_H_

namespace SimModelNET
{
	public ref class XMLSchemaCache
	{
	protected:
		static System::String^ _schemaFile;
	internal:
		static System::String^ SchemaFile();
	public:
		static void InitializeFromFile(System::String^ schemaFile);
		static void InitializeFromString(System::String^ schemaString);
		static property bool IsInitialized
		{
			bool get();
		}
	};
}

#endif //_XMLSchemaCache_H_

