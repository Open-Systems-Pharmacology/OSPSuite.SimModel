#ifndef _PInvokeSimulation_H_
#define _PInvokeSimulation_H_

#include "SimModel/Simulation.h"
#include "SimModel/SimulationOptions.h"

namespace SimModelNative
{
   struct SimulationOptionsStructure
   {
      bool ShowProgress;
		double ExecutionTimeLimit;
		bool StopOnWarnings;
		bool AutoReduceTolerances;
		bool WriteLogFile;
		bool CheckForNegativeValues;
      bool ValidateWithXMLSchema;
      bool IdentifyUsedParameters;
      bool KeepXMLNodeAsString;
		bool UseFloatComparisonInUserOutputTimePoints;

      void CopyFrom(const SimulationOptions& options);
   };

   //-------------- C interface for PInvoke -----------------------------------------
   extern "C"
   {
      SIM_EXPORT Simulation* CreateSimulation();
      SIM_EXPORT void DisposeSimulation(Simulation* simulation);

      SIM_EXPORT void LoadSimulationFromXMLFile(Simulation* simulation, const char* fileName, bool& success, char** errorMessage);
      SIM_EXPORT void LoadSimulationFromXMLString(Simulation* simulation, const char* simulationXML, bool& success, char** errorMessage);
      SIM_EXPORT void FinalizeSimulation(Simulation* simulation, bool& success, char** errorMessage);
      SIM_EXPORT long GetSimulationProgress(Simulation* simulation);
      SIM_EXPORT void CancelSimulationRun(Simulation* simulation);
      //SIM_EXPORT char* GetSimModelVersion();
      SIM_EXPORT char* GetObjectPathDelimiter(Simulation* simulation);

      //SIM_EXPORT SimulationOptionsStructure GetSimulationOptions(Simulation* simulation);
      SIM_EXPORT void FillSimulationOptions(Simulation* simulation, SimulationOptionsStructure * options);
      
      SIM_EXPORT void SetSimulationOptions(Simulation* simulation, SimulationOptionsStructure options);

      SIM_EXPORT void RunSimulation(Simulation* simulation, bool& toleranceWasReduced, double& newAbsTol, double& newRelTol, bool& success, char** errorMessage);

      SIM_EXPORT int GetNumberOfTimePoints(Simulation* simulation);

      //The caller must call GetNumberOfTimePoints() first and pass an array where NumberOfTimePoints elements are preallocated 
      SIM_EXPORT void FillTimeValues(Simulation* simulation, double* timepoints, bool& success, char** errorMessage);

      //returns XML string of the simulation (with updated formulas for not fixed parameters/species initial values)
      SIM_EXPORT char* GetSimulationXMLString(Simulation* simulation, bool& success, char** errorMessage);

      //checks if simulation has any contains parameters
      SIM_EXPORT bool ContainsPersistableParameters(Simulation* simulation, bool& success, char** errorMessage);

      SIM_EXPORT int GetXMLVersion(Simulation* simulation);

      SIM_EXPORT void ReleaseSimulationMemory(Simulation* simulation, bool& success, char** errorMessage);

      SIM_EXPORT Observer* GetObserverFrom(Simulation* simulation, const char* entityId, bool& success, char** errorMessage);
      SIM_EXPORT Species* GetSpeciesFrom(Simulation* simulation, const char* entityId, bool& success, char** errorMessage);

      //returns number of all variables (Species, Observer) which are persistable
      SIM_EXPORT int GetNumberOfQuantitiesWithValues(Simulation* simulation, bool& success, char** errorMessage);

      //fills entity ids for all quantities with values.
      //entityIDs array is pre-allocated with <size> elements
      SIM_EXPORT void FillEntityIdsForQuantitiesWithValues(Simulation* simulation, char** entityIds, int size, bool& success, char** errorMessage);

      // -----------------------------------------------------------------------------------------------------------------

      //SIM_EXPORT TObjectList<Species>& SpeciesList();
      //SIM_EXPORT TObjectList<Observer>& Observers();

      //SIM_EXPORT const TObjectVector<SolverWarning>& SolverWarnings() const;
      //SIM_EXPORT OutputSchema& GetOutputSchema();

      ////fill the properties of all simulation observers
      //SIM_EXPORT void FillObserverProperties(std::vector<QuantityInfo>& observerProperties);

      ////fill the properties of all simulation parameters
      //SIM_EXPORT void FillParameterProperties(std::vector<ParameterInfo>& paramProperties);

      ////set variable parameters
      //SIM_EXPORT void SetVariableParameters(std::vector<ParameterInfo>& paramProperties);

      ////fill the properties of all simulation DE variables
      //SIM_EXPORT void FillDEVariableProperties(std::vector<SpeciesInfo>& variableProperties);

      ////set variable DE Variables
      //SIM_EXPORT void SetVariableDEVariables(std::vector<SpeciesInfo>& variableProperties);

      ////sets parameter values into the simulation for the next run
      //SIM_EXPORT void SetParametersValues(std::vector<ParameterInfo>& paramProperties);

      ////sets DE variable properties into the simulation for the next run
      //SIM_EXPORT void SetDEVariablesProperties(std::vector<SpeciesInfo>& variableProperties);

      //SIM_EXPORT bool UseBandLinearSolver();
      //SIM_EXPORT void SetUseBandLinearSolver(bool useBandLinearSolver);
   }

}//.. end "namespace SimModelNative"


#endif //_PInvokeSimulation_H_

