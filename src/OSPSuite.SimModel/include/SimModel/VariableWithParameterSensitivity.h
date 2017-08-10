#ifndef _VariableWithParameterSensitivity_H_
#define _VariableWithParameterSensitivity_H_
 
#include <string>
#include "SimModel/Variable.h"
#include "SimModel/QuantityWithParameterSensitivity.h"

namespace SimModelNative
{

class VariableWithParameterSensitivity :
	public Variable,
	public QuantityWithParameterSensitivity
{
public:
	VariableWithParameterSensitivity(void);
	virtual ~VariableWithParameterSensitivity(void);
};

}//.. end "namespace SimModelNative"

#endif //_VariableWithParameterSensitivity_H_
