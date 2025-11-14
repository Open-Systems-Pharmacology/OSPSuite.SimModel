#ifndef _SimulationOptions_H_
#define _SimulationOptions_H_

#include "SimModel/GlobalConstants.h"
#include <string>

namespace SimModelNative
{

	class SimulationOptions
	{

	private:
		bool   _showProgress;
		double _executionTimeLimit;
		bool   _stopOnWarnings;
		bool   _autoReduceTolerances;
		bool _validateXMLWithSchema;
		std::string _logFile;
		bool _writeLogFile; //if set to true AND logfile name is not empty, log outputs are created
		bool _keepXMLNodeAsString; //original xml is required only for saving the simulation to XML
		bool _useFloatComparisonInUserOutputTimePoints; //if set to true, float comparison will be used
		                                                //for user output time points.Otherwise: double
		bool _identifyUsedParameters; //if set to false: ALL parameters will be marked as used in ODE variables/observes
		                              //otherwise: only parameters really used will be marked

	public:
		SimulationOptions();

		bool ShowProgress() const;
		void SetShowProgress(bool showProgress);

		SIM_EXPORT double ExecutionTimeLimit() const;
		SIM_EXPORT void SetExecutionTimeLimit(double executionTimeLimit);

		SIM_EXPORT bool StopOnWarnings() const;
		SIM_EXPORT void SetStopOnWarnings(bool stopOnWarnings);

		SIM_EXPORT bool AutoReduceTolerances() const;
		SIM_EXPORT void SetAutoReduceTolerances(bool autoReduceTolerances);

		//if set to true AND logfile name is not empty, log outputs are created
		SIM_EXPORT void WriteLogFile(bool writeLogFile);
		bool WriteLogFile(void) const;

		std::string LogFile(void) const;
		void SetLogFile(const std::string & logFile);

		SIM_EXPORT void ValidateWithXMLSchema(bool validate);
		SIM_EXPORT bool ValidateWithXMLSchema(void) const;

		SIM_EXPORT void IdentifyUsedParameters(bool identifyUsedParameters);
		SIM_EXPORT bool IdentifyUsedParameters(void) const;

		SIM_EXPORT bool KeepXMLNodeAsString() const;
		SIM_EXPORT void SetKeepXMLNodeAsString(bool);

		SIM_EXPORT bool UseFloatComparisonInUserOutputTimePoints() const;
		SIM_EXPORT void SetUseFloatComparisonInUserOutputTimePoints(bool);

		void CopyFrom(SimulationOptions & srcOptions);
	};

}
#endif 