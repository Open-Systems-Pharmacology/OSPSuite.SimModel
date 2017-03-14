#ifndef _ParameterSensitivity_H_
#define _ParameterSensitivity_H_

#include "SimModel/Variable.h"
#include "SimModel/Parameter.h"

namespace SimModelNative
{

	class ParameterSensitivity : public Variable
	{
	protected:
		Parameter * _parameter;

	public:
		ParameterSensitivity(Parameter * parameter);
		Parameter * GetParameter();

		long GetId(void);
		std::string GetEntityId();
	};

}
#endif 