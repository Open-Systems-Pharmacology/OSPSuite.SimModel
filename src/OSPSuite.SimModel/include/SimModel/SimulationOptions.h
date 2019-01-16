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
		bool _checkForNegativeValues;
		bool _keepXMLNodeAsString; //original xml is required only for saving the simulation to XML
		bool _useFloatComparisonInUserOutputTimePoints; //if set to true, float comparison will be used
		                                                //for user output time points.Otherwise: double
		bool _identifyUsedParameters; //if set to false: ALL parameters will be marked as used in ODE variables/observes
		                              //otherwise: only parameters really used will be marked

	public:
		SimulationOptions();

		bool ShowProgress();
		void SetShowProgress(bool showProgress);

		SIM_EXPORT double ExecutionTimeLimit();
		SIM_EXPORT void SetExecutionTimeLimit(double executionTimeLimit);

		SIM_EXPORT bool StopOnWarnings();
		SIM_EXPORT void SetStopOnWarnings(bool stopOnWarnings);

		SIM_EXPORT bool AutoReduceTolerances();
		SIM_EXPORT void SetAutoReduceTolerances(bool autoReduceTolerances);

		//if set to true AND logfile name is not empty, log outputs are created
		SIM_EXPORT void WriteLogFile(bool writeLogFile);
		bool WriteLogFile(void);

		std::string LogFile(void);
		void SetLogFile(const std::string & logFile);

		//at the moment for tests only. If we get problems with negative values check, this
		//property should be set via solver options in SimModel-XML (like UseJacobian etc.)
		void SetCheckForNegativeValues(bool performCheck);
		bool CheckForNegativeValues(void);

		SIM_EXPORT void ValidateWithXMLSchema(bool validate);
		SIM_EXPORT bool ValidateWithXMLSchema(void);

		SIM_EXPORT void IdentifyUsedParameters(bool identifyUsedParameters);
		SIM_EXPORT bool IdentifyUsedParameters(void);

		SIM_EXPORT bool KeepXMLNodeAsString();
		SIM_EXPORT void SetKeepXMLNodeAsString(bool);

		SIM_EXPORT bool UseFloatComparisonInUserOutputTimePoints();
		SIM_EXPORT void SetUseFloatComparisonInUserOutputTimePoints(bool);

		void CopyFrom(SimulationOptions & srcOptions);
	};

}
#endif 