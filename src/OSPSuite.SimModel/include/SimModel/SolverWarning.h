#ifndef _SolverWarning_H_
#define _SolverWarning_H_

#include "SimModel/Formula.h"
#include <string>

namespace SimModelNative
{

class SolverWarning
{
	protected:
		double _time;
		std::string _message;

	public:
		SolverWarning(double time, std::string message);
		SolverWarning(std::string message);

		SIM_EXPORT double Time() const;
		SIM_EXPORT std::string Message() const;
};

}//.. end "namespace SimModelNative"

#endif //_SolverWarning_H_

