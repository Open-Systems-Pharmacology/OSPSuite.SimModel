#include "SimModel/SimulationOptions.h"

namespace SimModelNative
{

using namespace std;

SimulationOptions::SimulationOptions(void)
{
	_validateXMLWithSchema = false; //perform schema validation only if explicitly set by user
	_showProgress = false;
	_executionTimeLimit = 0.0;    //no limit of simulation execution time
	_stopOnWarnings = true;       //treat warnings of ODE solver as errors
	_autoReduceTolerances = true; //automatically reduce solver tolerances
	_writeLogFile = true;
	_identifyUsedParameters = false;

	_keepXMLNodeAsString = false; //not required from PK-Sim/MoBi
	                              //only required from Matlab/R and can be set = true in SimModelComp

	_useFloatComparisonInUserOutputTimePoints = true; //default for PK-Sim/MoBi
}

void SimulationOptions::CopyFrom(SimulationOptions & srcOptions)
{
	_validateXMLWithSchema = srcOptions.ValidateWithXMLSchema();
	_showProgress = srcOptions.ShowProgress();
	_executionTimeLimit = srcOptions.ExecutionTimeLimit();
	_stopOnWarnings = srcOptions.StopOnWarnings();
	_autoReduceTolerances = srcOptions.AutoReduceTolerances();
	_writeLogFile = srcOptions.WriteLogFile();
	_keepXMLNodeAsString = srcOptions.KeepXMLNodeAsString();
	_useFloatComparisonInUserOutputTimePoints = srcOptions.UseFloatComparisonInUserOutputTimePoints();
	_identifyUsedParameters = srcOptions.IdentifyUsedParameters();
}

void SimulationOptions::WriteLogFile(bool writeLogFile)
{
	_writeLogFile = writeLogFile;
}

bool SimulationOptions::WriteLogFile(void) const
{
	return _writeLogFile;
}

void SimulationOptions::ValidateWithXMLSchema(bool validate)
{
	_validateXMLWithSchema = validate;
}

bool SimulationOptions::ValidateWithXMLSchema(void) const
{
	return _validateXMLWithSchema;
}

void SimulationOptions::IdentifyUsedParameters(bool identifyUsedParameters)
{
	_identifyUsedParameters = identifyUsedParameters;
}

bool SimulationOptions::IdentifyUsedParameters(void) const
{
	return _identifyUsedParameters;
}

bool SimulationOptions::ShowProgress() const
{
	return _showProgress;
}

void SimulationOptions::SetShowProgress(bool showProgress)
{
	_showProgress = showProgress;
}

double SimulationOptions::ExecutionTimeLimit() const
{
	return _executionTimeLimit;
}

void SimulationOptions::SetExecutionTimeLimit(double executionTimeLimit)
{
	_executionTimeLimit = executionTimeLimit;
}

string SimulationOptions::LogFile(void) const
{
	return _logFile;
}

void SimulationOptions::SetLogFile(const std::string & logFile)
{
	_logFile = logFile;
}

bool SimulationOptions::StopOnWarnings() const
{
	return _stopOnWarnings;
}

void SimulationOptions::SetStopOnWarnings(bool stopOnWarnings)
{
	_stopOnWarnings = stopOnWarnings;
}

bool SimulationOptions::AutoReduceTolerances() const
{
	return _autoReduceTolerances;
}

void SimulationOptions::SetAutoReduceTolerances(bool autoReduceTolerances)
{
	_autoReduceTolerances = autoReduceTolerances;
}

bool SimulationOptions::KeepXMLNodeAsString() const
{
	return _keepXMLNodeAsString;
}

void SimulationOptions::SetKeepXMLNodeAsString(bool keepXMLNodeAsString)
{
	_keepXMLNodeAsString = keepXMLNodeAsString;
}


bool SimulationOptions::UseFloatComparisonInUserOutputTimePoints() const
{
	return _useFloatComparisonInUserOutputTimePoints;
}

void SimulationOptions::SetUseFloatComparisonInUserOutputTimePoints(bool useFloatComparisonInOutputSchema)
{
	_useFloatComparisonInUserOutputTimePoints = useFloatComparisonInOutputSchema;
}


}//.. end "namespace SimModelNative"
