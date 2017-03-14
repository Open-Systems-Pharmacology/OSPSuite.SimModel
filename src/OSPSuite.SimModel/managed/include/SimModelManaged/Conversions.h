#ifndef _Managed_Conversions_H_
#define _Managed_Conversions_H_

#include <string>
#include <vector>
#include "SimModelManaged/ParameterProperties.h"
#include "SimModelManaged/SpeciesProperties.h"
#include "SimModel/ParameterInfo.h"
#include "SimModel/SpeciesInfo.h"

namespace SimModelNET
{
	using namespace System::Collections::Generic;

	ref class NETToCPPConversions
	{
	public:
		static std::string MarshalString(System::String^ s);
		static void DoubleArrayToVector(array<double>^ doubleArray, std::vector<double> & doubleVec);
		static void StringArrayToVector(array<System::String^>^ stringArray, std::vector<std::string> & stringVec);
		static void StringEnumerableToVector(IEnumerable<System::String^>^ stringList, std::vector<std::string> & stringVec);
	};

	ref class CPPToNETConversions
	{
	public:
		static System::String^ MarshalString(const std::string & s);
		static array<double>^ DoubleVectorToArray(const std::vector<double> & doubleVec);
		static array<double>^ DoubleArrayToArray(const double * doubleCppArray, int size);
		static array<System::String^>^ StringVectorToArray(const std::vector<std::string> & stringVec);
		static IList<System::String^>^ StringVectorToList(const std::vector<std::string> & stringVec);

		static IList<IParameterProperties^>^ ParameterPropertiesListFrom(std::vector<SimModelNative::ParameterInfo> & srcProperties,
			                                                             const std::string & objectPathDelimiter);
		static IList<ISpeciesProperties^>^ SpeciesPropertiesListFrom(std::vector<SimModelNative::SpeciesInfo> & srcProperties,
			                                                         const std::string & objectPathDelimiter);
	};
}//.. end "namespace SimModelNET"

#endif //_Managed_Conversions_H_

