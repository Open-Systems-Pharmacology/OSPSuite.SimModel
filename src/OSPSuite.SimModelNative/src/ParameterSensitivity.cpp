#include "SimModel/ParameterSensitivity.h"

namespace SimModelNative
{

	ParameterSensitivity::ParameterSensitivity(Parameter * parameter)
	{
		_parameter = parameter;
	}

	Parameter * ParameterSensitivity::GetParameter()
	{
		return _parameter;
	}

	long ParameterSensitivity::GetId(void)
	{
		return _parameter->GetId();
	}

	std::string ParameterSensitivity::GetEntityId()
	{
		return _parameter->GetEntityId();
	}
}