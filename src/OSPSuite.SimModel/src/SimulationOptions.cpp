#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/SimulationOptions.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

SimulationOptions::SimulationOptions(void)
{
	_validateXMLWithSchema = true;
	_showProgress = false;
	_executionTimeLimit = 0.0;    //no limit of simulation execution time
	_stopOnWarnings = true;       //treat warnings of ODE solver as errors
	_autoReduceTolerances = true; //automatically reduce solver tolerances
	_writeLogFile = true;
	_checkForNegativeValues = true;

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
	_checkForNegativeValues = srcOptions.CheckForNegativeValues();
	_keepXMLNodeAsString = srcOptions.KeepXMLNodeAsString();
	_useFloatComparisonInUserOutputTimePoints = srcOptions.UseFloatComparisonInUserOutputTimePoints();
}

void SimulationOptions::SetCheckForNegativeValues(bool performCheck)
{
	_checkForNegativeValues = performCheck;
}

bool SimulationOptions::CheckForNegativeValues(void)
{
	return _checkForNegativeValues;
}

void SimulationOptions::WriteLogFile(bool writeLogFile)
{
	_writeLogFile = writeLogFile;
}

bool SimulationOptions::WriteLogFile(void)
{
	return _writeLogFile;
}

void SimulationOptions::ValidateWithXMLSchema(bool validate)
{
	_validateXMLWithSchema = validate;
}

bool SimulationOptions::ValidateWithXMLSchema(void)
{
	return _validateXMLWithSchema;
}

bool SimulationOptions::ShowProgress()
{
	return _showProgress;
}

void SimulationOptions::SetShowProgress(bool showProgress)
{
	_showProgress = showProgress;
}

double SimulationOptions::ExecutionTimeLimit()
{
	return _executionTimeLimit;
}

void SimulationOptions::SetExecutionTimeLimit(double executionTimeLimit)
{
	_executionTimeLimit = executionTimeLimit;
}

string SimulationOptions::LogFile(void)
{
	return _logFile;
}

void SimulationOptions::SetLogFile(const std::string & logFile)
{
	_logFile = logFile;
}

bool SimulationOptions::StopOnWarnings()
{
	return _stopOnWarnings;
}

void SimulationOptions::SetStopOnWarnings(bool stopOnWarnings)
{
	_stopOnWarnings = stopOnWarnings;
}

bool SimulationOptions::AutoReduceTolerances()
{
	return _autoReduceTolerances;
}

void SimulationOptions::SetAutoReduceTolerances(bool autoReduceTolerances)
{
	_autoReduceTolerances = autoReduceTolerances;
}

bool SimulationOptions::KeepXMLNodeAsString()
{
	return _keepXMLNodeAsString;
}

void SimulationOptions::SetKeepXMLNodeAsString(bool keepXMLNodeAsString)
{
	_keepXMLNodeAsString = keepXMLNodeAsString;
}


bool SimulationOptions::UseFloatComparisonInUserOutputTimePoints()
{
	return _useFloatComparisonInUserOutputTimePoints;
}

void SimulationOptions::SetUseFloatComparisonInUserOutputTimePoints(bool useFloatComparisonInOutputSchema)
{
	_useFloatComparisonInUserOutputTimePoints = useFloatComparisonInOutputSchema;
}


}//.. end "namespace SimModelNative"
