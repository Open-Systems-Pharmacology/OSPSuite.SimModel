#include "SimModelManaged/Conversions.h"

using namespace System::Runtime::InteropServices;

namespace SimModelNET
{
	//---- NET to C++ types conversions
	//

	//Method to marshal a CLR compatible System::String to a string
	//(alternative: s. ms-help://MS.VSCC.v90/MS.MSDNQTR.v90.en/dv_vcmgdlib/html/24a1afee-51c0-497c-948c-f77fe43635c8.htm)
	std::string NETToCPPConversions::MarshalString(System::String^ s)
	{
  	   const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	   std::string os = chars;
       Marshal::FreeHGlobal( System::IntPtr((void*)chars));
	   return os;
	}

	void NETToCPPConversions::DoubleArrayToVector(array<double>^ doubleArray, std::vector<double> & doubleVec)
	{
		doubleVec.clear();		

		for(int i=doubleArray->GetLowerBound(0);i<=doubleArray->GetUpperBound(0);i++)
		{
			double d = doubleArray[i];
			doubleVec.push_back(d);
		}
	}

	void NETToCPPConversions::StringArrayToVector(array<System::String^>^ stringArray, std::vector<std::string> & stringVec)
	{
		stringVec.clear();

		for(int i=stringArray->GetLowerBound(0);i<stringArray->GetUpperBound(0);i++)
			stringVec.push_back(MarshalString(stringArray[i]));
	}

	void NETToCPPConversions::StringEnumerableToVector(IEnumerable<System::String^>^ stringList, std::vector<std::string> & stringVec)
	{
		stringVec.clear();
		
		for each(System::String^ s in stringList)
			stringVec.push_back(MarshalString(s));
	}

	//---- C++ to NET types conversions
	//

	System::String^ CPPToNETConversions::MarshalString(const std::string & s)
	{
		return gcnew System::String(s.c_str());
	}

	array<double>^ CPPToNETConversions::DoubleVectorToArray(const std::vector<double> & doubleVec)
	{
		array<double>^ doubleArray = gcnew array<double>((unsigned int)doubleVec.size());

		for(unsigned int i=0; i<doubleVec.size(); i++)
			doubleArray[i] = doubleVec[i];

		return doubleArray;
	}

	array<double>^ CPPToNETConversions::DoubleArrayToArray(const double * doubleCppArray, int size)
	{
		if (size<=0)
			throw gcnew System::ArgumentException(gcnew System::String("arry size must be positive"));

		array<double>^ doubleArray = gcnew array<double>(size);

		for(int i=0; i<size; i++)
			doubleArray[i] = doubleCppArray[i];

		return doubleArray;
	}

	array<System::String^>^ CPPToNETConversions::StringVectorToArray(const std::vector<std::string> & stringVec)
	{
		array<System::String^>^ stringArray = gcnew array<System::String^>((unsigned int)stringVec.size());;

		for(unsigned int i=0; i<stringVec.size(); i++)
			stringArray[i] = MarshalString(stringVec[i]);

		return stringArray;
	}

	IList<System::String^>^ CPPToNETConversions::StringVectorToList(const std::vector<std::string> & stringVec)
	{
		IList<System::String^>^ stringList = gcnew List<System::String^>();;

		for(unsigned int i=0; i<stringVec.size(); i++)
			stringList->Add(MarshalString(stringVec[i]));

		return stringList;
	}

	using namespace SimModelNative;

	IList<IParameterProperties^>^ CPPToNETConversions::ParameterPropertiesListFrom(std::vector<ParameterInfo> & srcProperties,
		                                                                           const std::string & objectPathDelimiter)
	{
		IList<IParameterProperties^>^ paramProperties = gcnew List<IParameterProperties^>();

		for (size_t i=0; i<srcProperties.size(); i++)
			paramProperties->Add(gcnew ParameterProperties(srcProperties[i], objectPathDelimiter));

		return paramProperties;
	}

	IList<ISpeciesProperties^>^ CPPToNETConversions::SpeciesPropertiesListFrom(std::vector<SpeciesInfo> & srcProperties,
		                                                                       const std::string & objectPathDelimiter)
	{
		IList<ISpeciesProperties^>^ speciesProperties = gcnew List<ISpeciesProperties^>();

		for (size_t i=0; i<srcProperties.size(); i++)
			speciesProperties->Add(gcnew SpeciesProperties(srcProperties[i], objectPathDelimiter));

		return speciesProperties;
	}

}