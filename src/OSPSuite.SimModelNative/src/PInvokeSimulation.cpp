#include "SimModel/PInvokeHelper.h"
#include "SimModel/PInvokeSimulation.h"

namespace SimModelNative
{
   using namespace std;

   void SimulationOptionsStructure::CopyFrom(const SimulationOptions& options)
   {
      ShowProgress = options.ShowProgress();
      ExecutionTimeLimit = options.ExecutionTimeLimit();
      StopOnWarnings = options.StopOnWarnings();
      AutoReduceTolerances = options.AutoReduceTolerances();
      WriteLogFile = options.WriteLogFile();
      CheckForNegativeValues = options.CheckForNegativeValues();
      ValidateWithXMLSchema = options.ValidateWithXMLSchema();
      IdentifyUsedParameters = options.IdentifyUsedParameters();
      KeepXMLNodeAsString = options.KeepXMLNodeAsString();
      UseFloatComparisonInUserOutputTimePoints = options.UseFloatComparisonInUserOutputTimePoints();
   }

   Simulation* CreateSimulation()
   {
      return new Simulation();
   }

   void DisposeSimulation(Simulation* simulation)
   {
      delete simulation;
   }

   void LoadSimulationFromXMLFile(Simulation* simulation, const char* fileName, bool& success, char** errorMessage)
   {
      try
      {
         simulation->LoadFromXMLFile(fileName);
         success = true;
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("LoadSimulationFromXMLFile");
         success = false;
      }
   }

   void LoadSimulationFromXMLString(Simulation* simulation, const char* simulationXML, bool& success, char** errorMessage)
   {
      try
      {
         simulation->LoadFromXMLString(simulationXML);
         success = true;
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("LoadSimulationFromXMLString");
         success = false;
      }
   }

   void FinalizeSimulation(Simulation* simulation, bool& success, char** errorMessage)
   {
      try
      {
         simulation->Finalize();
         success = true;
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("LoadSimulationFromXMLString");
         success = false;
      }
   }

   long GetSimulationProgress(Simulation* simulation)
   {
      return simulation->GetProgress();
   }

   void CancelSimulationRun(Simulation* simulation)
   {
      simulation->Cancel();
   }

   //char* GetSimModelVersion();

   char* GetObjectPathDelimiter(Simulation* simulation)
   {
      return MarshalString(simulation->GetObjectPathDelimiter());
   }

   SimulationOptionsStructure GetSimulationOptions(Simulation* simulation)
   {
      SimulationOptionsStructure options{};
      options.CopyFrom(simulation->Options());

      return options;
   }

   void SetSimulationOptions(Simulation* simulation, SimulationOptionsStructure options)
   {
      auto simulationOptions = simulation->Options();

      simulationOptions.SetShowProgress(options.ShowProgress);
      simulationOptions.SetExecutionTimeLimit(options.ExecutionTimeLimit);
      simulationOptions.SetStopOnWarnings(options.StopOnWarnings);
      simulationOptions.SetAutoReduceTolerances(options.AutoReduceTolerances);
      simulationOptions.WriteLogFile(options.WriteLogFile);
      simulationOptions.SetCheckForNegativeValues(options.CheckForNegativeValues);
      simulationOptions.ValidateWithXMLSchema(options.ValidateWithXMLSchema);
      simulationOptions.IdentifyUsedParameters(options.IdentifyUsedParameters);
      simulationOptions.SetKeepXMLNodeAsString(options.KeepXMLNodeAsString);
      simulationOptions.SetUseFloatComparisonInUserOutputTimePoints(options.UseFloatComparisonInUserOutputTimePoints);
   }

   void RunSimulation(Simulation* simulation, bool& toleranceWasReduced, double& newAbsTol, double& newRelTol, bool& success, char** errorMessage)
   {
      try
      {
         simulation->RunSimulation(toleranceWasReduced, newAbsTol, newRelTol);
         success = true;
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("RunSimulation");
         success = false;
      }
   }

   int GetNumberOfTimePoints(Simulation* simulation)
   {
      return simulation->GetNumberOfTimePoints();
   }

   //The caller must call GetNumberOfTimePoints() first and pass an array where NumberOfTimePoints elements are preallocated 
   void FillTimeValues(Simulation* simulation, double* timepoints, bool& success, char** errorMessage)
   {
      try
      {
         double* timeValues = simulation->GetTimeValues();
         memcpy(timepoints, timeValues, simulation->GetNumberOfTimePoints() * sizeof(double));

         success = true;
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("FillTimeValues");
         success = false;
      }
   }

   //returns XML string of the simulation (with updated formulas for not fixed parameters/species initial values)
   char* GetSimulationXMLString(Simulation* simulation, bool& success, char** errorMessage)
   {
      success = true;

      try
      {
         return MarshalString(simulation->GetObjectPathDelimiter());
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
         return MarshalString("");
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("GetSimulationXMLString");
         success = false;
         return MarshalString("");
      }
   }

   //checks if simulation has any contains parameters
   bool ContainsPersistableParameters(Simulation* simulation, bool& success, char** errorMessage)
   {
      success = true;

      try
      {
         return simulation->ContainsPersistableParameters();
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
         return false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("GetSimulationXMLString");
         success = false;
         return false;
      }
   }

   int GetXMLVersion(Simulation* simulation)
   {
      return simulation->GetXMLVersion();
   }

   void ReleaseSimulationMemory(Simulation* simulation, bool& success, char** errorMessage)
   {
      
      try
      {
         simulation->ReleaseMemory();

         success = true;
      }
      catch (ErrorData& ED)
      {
         *errorMessage = ErrorMessageFrom(ED);
         success = false;
      }
      catch (...)
      {
         *errorMessage = ErrorMessageFromUnknown("FillTimeValues");
         success = false;
      }
   }
}//.. end "namespace FuncParserNative"
