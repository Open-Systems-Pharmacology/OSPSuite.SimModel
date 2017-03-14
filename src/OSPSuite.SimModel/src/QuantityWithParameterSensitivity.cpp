#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/QuantityWithParameterSensitivity.h"

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

QuantityWithParameterSensitivity::QuantityWithParameterSensitivity(void)
{}

QuantityWithParameterSensitivity::~QuantityWithParameterSensitivity(void)
{
	_parameterSensitivities.clear();
}

void QuantityWithParameterSensitivity::InitParameterSensitivities(TObjectList<Parameter>& sensitivityParameter, int numberOfTimePoints, bool isPersistable)
{
	_parameterSensitivities.clear();

	if (!isPersistable)
		return;

	for (int i = 0; i < sensitivityParameter.size(); i++)
	{
		ParameterSensitivity * parameterSensitivity = new ParameterSensitivity(sensitivityParameter[i]);
		parameterSensitivity->RedimValues(numberOfTimePoints);

		//set initial sensitivity to zero
		//TODO this is WRONG if InitialFormula depends on parameter!
		//In that case derivative by the parameter must be calculated
		parameterSensitivity->SetValue(0, 0.0);

		_parameterSensitivities.Add(parameterSensitivity);
	}
}

void QuantityWithParameterSensitivity::SetSensitivityValues(int timeStepNumber, double* sensitivityValues)
{
	//sensitivity values come in the same order as sensitivity parameters per construction
	for (int i = 0; i < _parameterSensitivities.size(); i++)
		_parameterSensitivities[i]->SetValue(timeStepNumber, sensitivityValues[i]);
}

TObjectList<ParameterSensitivity> & QuantityWithParameterSensitivity::ParameterSensitivities()
{
	return _parameterSensitivities;
}
}//.. end "namespace SimModelNative"
