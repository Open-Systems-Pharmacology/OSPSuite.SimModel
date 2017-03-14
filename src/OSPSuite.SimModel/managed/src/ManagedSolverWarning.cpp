#include "SimModelManaged/SolverWarning.h"

namespace SimModelNET
{
	SolverWarning::SolverWarning(double outputTime, System::String^ warning)
	{
		_outputTime  = outputTime;
		_warning     = warning;
	}

	double SolverWarning::OutputTime::get()
	{
		return _outputTime;
	}

	System::String^ SolverWarning::Warning::get()
	{
		return _warning;
	}

}
