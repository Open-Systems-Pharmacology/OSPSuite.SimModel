#ifndef _QuantityWithParameterSensitivity_H_
#define _QuantityWithParameterSensitivity_H_

#include "SimModel/ParameterSensitivity.h"
#include "SimModel/TObjectList.h"

namespace SimModelNative
{

class QuantityWithParameterSensitivity
{
protected:
	//parameter sensitivities cached by parameter entity id
	TObjectList<ParameterSensitivity> _parameterSensitivities;

public:
	QuantityWithParameterSensitivity(void);
	virtual ~QuantityWithParameterSensitivity(void);

	void InitParameterSensitivities(TObjectList<Parameter> & sensitivityParameter, int numberOfTimePoints, bool isPersistable);

	//set sensitivity values for the <timeStepNumber>
	//sensitivity values come in the same order as sensitivity parameters (per construction)
	void SetSensitivityValues(int timeStepNumber, double* sensitivityValues);

	SIM_EXPORT TObjectList <ParameterSensitivity> & ParameterSensitivities();
};

}
#endif 