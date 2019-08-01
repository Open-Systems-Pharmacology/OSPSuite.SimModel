using System;
using System.Runtime.InteropServices;

namespace OSPSuite.SimModel
{
   //[StructLayout(LayoutKind.Sequential)]
   internal struct SimulationOptionsStructure
   {
      [MarshalAs(UnmanagedType.I1)]
      public bool ShowProgress;
      
      public double ExecutionTimeLimit;
      [MarshalAs(UnmanagedType.I1)]
      public bool StopOnWarnings;

      [MarshalAs(UnmanagedType.I1)]
      public bool AutoReduceTolerances;

      [MarshalAs(UnmanagedType.I1)]
      public bool WriteLogFile;

      [MarshalAs(UnmanagedType.I1)]
      public bool CheckForNegativeValues;

      [MarshalAs(UnmanagedType.I1)]
      public bool ValidateWithXMLSchema;

      [MarshalAs(UnmanagedType.I1)]
      public bool IdentifyUsedParameters;

      [MarshalAs(UnmanagedType.I1)]
      public bool KeepXMLNodeAsString;

      [MarshalAs(UnmanagedType.I1)]
      public bool UseFloatComparisonInUserOutputTimePoints;
   }

   internal class SimulationImports
   {
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr CreateSimulation();

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void DisposeSimulation(IntPtr simulation);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void LoadSimulationFromXMLFile(IntPtr simulation, string fileName, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void LoadSimulationFromXMLString(IntPtr simulation, string simulationXML, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FinalizeSimulation(IntPtr simulation, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetSimulationProgress(IntPtr simulation);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void CancelSimulationRun(IntPtr simulation);

      //TODO
      //char* GetSimModelVersion();

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern string GetObjectPathDelimiter(IntPtr simulation);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      //public static extern SimulationOptionsStructure GetSimulationOptions(IntPtr simulation);
      public static extern void FillSimulationOptions(IntPtr simulation, ref SimulationOptionsStructure options);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetSimulationOptions(IntPtr simulation, SimulationOptionsStructure options);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void RunSimulation(IntPtr simulation, out bool toleranceWasReduced, out double newAbsTol, out double newRelTol, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfTimePoints(IntPtr simulation);

      //The caller must call GetNumberOfTimePoints() first and pass an array where NumberOfTimePoints elements are preallocated 
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillTimeValues(IntPtr simulation, [In, Out] double[] timepoints, out bool success, out string errorMessage);

      //returns XML string of the simulation (with updated formulas for not fixed parameters/species initial values)
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern string GetSimulationXMLString(IntPtr simulation, out bool success, out string errorMessage);

      //checks if simulation has any contains parameters
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern bool ContainsPersistableParameters(IntPtr simulation, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetXMLVersion(IntPtr simulation);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void ReleaseSimulationMemory(IntPtr simulation, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr GetObserverFrom(IntPtr simulation, string entityId, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr GetSpeciesFrom(IntPtr simulation, string entityId, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void GetQuantityProperties(IntPtr quantity, out string containerPath, out string name);
   }

   internal class PInvokeHelper
   {
      public static void EvaluateCppCallResult(bool success, string errorMessage)
      {
         if (success)
            return;

         //TODO
         throw new Exception(errorMessage);
      }
   }

   public class Simulation : IDisposable
   {
      private readonly IntPtr _simulation;
      private SimulationOptionsStructure _simulationOptions;

      private bool _toleranceWasReduced;
      private double _newAbsTol, _newRelTol;

      private bool _disposed = false;

      private void evaluateCppCallResult(bool success, string errorMessage)
      {
         PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
      }

      private void setOptions(Action action)
      {
         action();
         SimulationImports.SetSimulationOptions(_simulation, _simulationOptions);
      }

      public Simulation()
      {
         _simulation = SimulationImports.CreateSimulation();
         //_simulationOptions = SimulationImports.GetSimulationOptions(_simulation);
         SimulationImports.FillSimulationOptions(_simulation,ref _simulationOptions);

         _toleranceWasReduced = false;
         _newAbsTol = double.NaN;
         _newRelTol = double.NaN;
      }

      public void LoadFromXMLFile(string fileName)
      {
         SimulationImports.LoadSimulationFromXMLFile(_simulation, fileName, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public void LoadFromXMLString(string xmlString)
      {
         SimulationImports.LoadSimulationFromXMLString(_simulation, xmlString, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public void FinalizeSimulation()
      {
         SimulationImports.FinalizeSimulation(_simulation, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public int Progress => SimulationImports.GetSimulationProgress(_simulation);

      public void Cancel()
      {
         SimulationImports.CancelSimulationRun(_simulation);
      }

      //TODO
      //char* GetSimModelVersion();

      public string ObjectPathDelimiter => SimulationImports.GetObjectPathDelimiter(_simulation);

      public bool ShowProgress
      {
         get => _simulationOptions.ShowProgress;
         set => setOptions(() => _simulationOptions.ShowProgress = value);
      }

      public double ExecutionTimeLimit
      {
         get => _simulationOptions.ExecutionTimeLimit;
         set => setOptions(()=>_simulationOptions.ExecutionTimeLimit = value); 
      }

      public bool StopOnWarnings
      {
         get => _simulationOptions.StopOnWarnings;
         set => setOptions(() => _simulationOptions.StopOnWarnings = value);
      }

      /// <summary>
      /// If solving of the diff. eq. systems fails with a convergence error <para></para>
      ///AND the value of AutoReduceTolerances is TRUE, both absolute and relative <para></para>
      ///tolerance will be reduced by 10, until either the system was successfully solved <para></para>
      ///or the predefined lower bounds for both tolerances were reached
      ///
      ///Default is TRUE
      /// </summary>
      public bool AutoReduceTolerances
      {
         get => _simulationOptions.AutoReduceTolerances;
         set => setOptions(() => _simulationOptions.AutoReduceTolerances = value);
      }

      public bool WriteLogFile
      {
         get => _simulationOptions.WriteLogFile;
         set => setOptions(() => _simulationOptions.WriteLogFile = value);
      }

      public bool CheckForNegativeValues
      {
         get => _simulationOptions.CheckForNegativeValues;
         set => setOptions(() => _simulationOptions.CheckForNegativeValues = value);
      }

      public bool ValidateWithXMLSchema
      {
         get => _simulationOptions.ValidateWithXMLSchema;
         set => setOptions(() => _simulationOptions.ValidateWithXMLSchema = value);
      }

      public bool IdentifyUsedParameters
      {
         get => _simulationOptions.IdentifyUsedParameters;
         set => setOptions(() => _simulationOptions.IdentifyUsedParameters = value);
      }

      public bool KeepXMLNodeAsString
      {
         get => _simulationOptions.KeepXMLNodeAsString;
         set => setOptions(() => _simulationOptions.KeepXMLNodeAsString = value);
      }

      public bool UseFloatComparisonInUserOutputTimePoints
      {
         get => _simulationOptions.UseFloatComparisonInUserOutputTimePoints;
         set => setOptions(() => _simulationOptions.UseFloatComparisonInUserOutputTimePoints = value);
      }

      public void RunSimulation()
      {
         SimulationImports.RunSimulation(_simulation, out _toleranceWasReduced,out _newAbsTol,out _newRelTol, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public bool ToleranceWasReduced => _toleranceWasReduced;

      public double UsedAbsoluteTolerance => _newAbsTol;

      public double UsedRelativeTolerance => _newRelTol;

      public int GetNumberOfTimePoints => SimulationImports.GetNumberOfTimePoints(_simulation);
      
      public double[] SimulationTimes
      {
         get
         {
            var simulationTimes = new double[GetNumberOfTimePoints];

            SimulationImports.FillTimeValues(_simulation, simulationTimes, out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);

            return simulationTimes;
         }
      }

      public string SimulationXMLString
      {
         get
         {
            var xmlString = SimulationImports.GetSimulationXMLString(_simulation, out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);

            return xmlString;
         }
      }

      public bool ContainsPersistableParameters
      {
         get
         {
            var containsPersistable = SimulationImports.ContainsPersistableParameters(_simulation, out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);

            return containsPersistable;
         }
      }

      public int XMLVersion => SimulationImports.GetXMLVersion(_simulation);

      public void ReleaseMemory()
      {
         SimulationImports.ReleaseSimulationMemory(_simulation, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public VariableValues ValuesFor(string entityId)
      {
         var quantity = SimulationImports.GetSpeciesFrom(_simulation, entityId, out var success, out var errorMessage);
         var variableType = VariableValues.VariableTypes.Species;

         if (!success)
         {
            //entity is not a species. Try observer
            quantity = SimulationImports.GetObserverFrom(_simulation, entityId, out success, out errorMessage);
            if (success)
               variableType = VariableValues.VariableTypes.Observer;
         }

         if(!success)
            throw new Exception($"{entityId} is not a valid species or observer entity id"); //TODO

         SimulationImports.GetQuantityProperties(quantity, out var containerPath, out var name);
         VariableValues variableValues=new VariableValues(quantity,variableType,entityId, containerPath, name);

         return variableValues;
      }

      //-------------------------------------------------------------------------------------------------

      //-------------------------------------------------------------------------------------------------

      public void Dispose()
      {
         Dispose(true);
         GC.SuppressFinalize(this);
      }

      // Protected implementation of Dispose pattern.
      protected virtual void Dispose(bool disposing)
      {
         if (_disposed)
            return;

         if (disposing)
         {
            SimulationImports.DisposeSimulation(_simulation);
         }

         _disposed = true;
      }
   }
}
