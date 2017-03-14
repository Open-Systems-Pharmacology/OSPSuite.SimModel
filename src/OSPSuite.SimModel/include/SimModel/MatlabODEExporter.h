#ifndef _MatlabODEExporter_H_
#define _MatlabODEExporter_H_

namespace SimModelNative
{

class MatlabODEExporter
{	
	public:
		SIM_EXPORT void WriteMatlabCodeFromFile (const std::string & sFileName, const std::string & OutDir, bool FullMode);
		SIM_EXPORT void WriteMatlabCode (const std::string & sSimulationXML, const std::string & OutDir, bool FullMode);
		SIM_EXPORT void WriteMatlabCode (Simulation * sim, const std::string & OutDir, bool FullMode);
	
	private:
		void WriteODEOptionsFile (Simulation * sim, const std::string & ODEOptionsFile);
		void WriteODEInitialValuesFile (Simulation * sim, const std::string & ODEInitValuesFile);
		void WriteODERHSFile (Simulation * sim, const std::string & RHSFunctionFile);
		void WriteODEMainFile (Simulation * sim, const std::string & ODEMainFile);
		void WriteSwitchFile (Simulation * sim, const std::string & switchFile);
		void WriteTableParametersFile (Simulation * sim, const std::string & tableParametersFile);
		void WriteHelperFunctions (const std::string & OutDir);

		std::string GetShortUniqueParameterName (int parameterIdx);
		DoubleQueue GetSwitchTimes (Simulation * sim);
		void MarkQuantitiesUsedBySwitches(Simulation * sim);

		void CheckIfSimulationCanBeExported(Simulation * sim);
};


}//.. end "namespace SimModelNative"


#endif //_MatlabODEExporter_H_

