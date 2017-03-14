#include "SimModelManaged/SpeciesProperties.h"
#include "SimModelManaged/Conversions.h"

namespace SimModelNET
{
	using namespace std;

	SpeciesProperties::SpeciesProperties(SimModelNative::SpeciesInfo & speciesInfo, const string & objectPathDelimiter)
		: AdjustableEntityProperties(speciesInfo.GetId(), CPPToNETConversions::MarshalString(speciesInfo.GetEntityId()), 
			CPPToNETConversions::MarshalString(speciesInfo.PathWithoutRoot(objectPathDelimiter)), CPPToNETConversions::MarshalString(speciesInfo.GetFullName()), 
			CPPToNETConversions::MarshalString(speciesInfo.GetDescription()), CPPToNETConversions::MarshalString(speciesInfo.GetUnit()), 
			speciesInfo.IsFormula(), CPPToNETConversions::MarshalString(speciesInfo.GetFormulaEquation()))
	{
		_scaleFactor = speciesInfo.GetScaleFactor();
	}

	double SpeciesProperties::ScaleFactor::get()
	{
		return _scaleFactor;
	}

	void SpeciesProperties::ScaleFactor::set(double scaleFactor)
	{
		_scaleFactor = scaleFactor;
	}

}
