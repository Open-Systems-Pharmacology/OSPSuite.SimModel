using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
// ReSharper disable UnusedMember.Global

namespace OSPSuite.SimModel
{
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

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfQuantitiesWithValues(IntPtr simulation, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillEntityIdsForQuantitiesWithValues(IntPtr simulation, [In, Out] string[] entityIds, int size, out bool success, out string errorMessage);

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
      
      private bool _disposed = false;

      private void evaluateCppCallResult(bool success, string errorMessage)
      {
         PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
      }

      public Simulation()
      {
         _simulation = SimulationImports.CreateSimulation();
         Options = new SimulationOptions(_simulation);
         RunStatistics = new SimulationRunStatistics(_simulation);
      }

      public SimulationOptions Options { get; }

      /// <summary>
      /// Stores some information about successful simulation run: used tolerances, number of ODE variables, ...
      /// </summary>
      public SimulationRunStatistics RunStatistics { get; }

      /// <summary>
      /// Load simulation from a SimModel-XML file
      /// </summary>
      /// <param name="fileName">Full path of a simulation file</param>
      public void LoadFromXMLFile(string fileName)
      {
         SimulationImports.LoadSimulationFromXMLFile(_simulation, fileName, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      /// <summary>
      /// Load simulation from a SimModel-XML string
      /// </summary>
      public void LoadFromXMLString(string xmlString)
      {
         SimulationImports.LoadSimulationFromXMLString(_simulation, xmlString, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      /// <summary>
      /// Finalize simulation (perform internal optimizations etc.)
      /// </summary>
      public void FinalizeSimulation()
      {
         SimulationImports.FinalizeSimulation(_simulation, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      /// <summary>
      /// Returns simulation progress in % during calculation
      /// </summary>
      public int Progress => SimulationImports.GetSimulationProgress(_simulation);

      /// <summary>
      /// Cancels current simulation run
      /// </summary>
      public void Cancel()
      {
         SimulationImports.CancelSimulationRun(_simulation);
      }

      //TODO
      //char* GetSimModelVersion();

      internal string ObjectPathDelimiter => SimulationImports.GetObjectPathDelimiter(_simulation);

      public void RunSimulation()
      {
         SimulationImports.RunSimulation(_simulation, out var toleranceWasReduced,out var newAbsTol,out var newRelTol, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);

         RunStatistics.ToleranceWasReduced = toleranceWasReduced;
         RunStatistics.UsedAbsoluteTolerance = newAbsTol;
         RunStatistics.UsedRelativeTolerance = newRelTol;
      }

      /// <summary>
      /// Returns the length of Output time raster
      /// </summary>
      public int GetNumberOfTimePoints => SimulationImports.GetNumberOfTimePoints(_simulation);

      /// <summary>
      /// Output time raster
      /// </summary>
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

      /// <summary>
      /// Species or observer time course for a given entity ID
      ///  - Species also includes parameters with RHS
      ///  - Observer also includes persistable parameters
      /// </summary>
      public VariableValues ValuesFor(string entityId)
      {
         var quantity = SimulationImports.GetSpeciesFrom(_simulation, entityId, out var success, out _);
         var variableType = VariableValues.VariableTypes.Species;

         if (!success)
         {
            //entity is not a species. Try observer
            quantity = SimulationImports.GetObserverFrom(_simulation, entityId, out success, out _);
            if (success)
               variableType = VariableValues.VariableTypes.Observer;
         }

         if(!success)
            throw new Exception($"{entityId} is not a valid species or observer entity id"); //TODO

         SimulationImports.GetQuantityProperties(quantity, out var containerPath, out var name);
         VariableValues variableValues=new VariableValues(quantity,variableType,entityId, containerPath, name);

         return variableValues;
      }

      /// <summary>
      /// Species- and Observer- time courses calculated by the system.
      ///  - Species also includes parameters with RHS
      ///  - Observer also includes persistable parameters
      /// </summary>
      public IEnumerable<VariableValues> AllValues
      {
         get
         {
            var allValues = new List<VariableValues>();

            var numberOfQuantitiesWithValues=SimulationImports.GetNumberOfQuantitiesWithValues(_simulation,out var success,out var errorMessage);
            evaluateCppCallResult(success,errorMessage);

            var entityIds =new string[numberOfQuantitiesWithValues];
            SimulationImports.FillEntityIdsForQuantitiesWithValues(_simulation, entityIds, numberOfQuantitiesWithValues,out success,out errorMessage);

            for (var i=0; i<numberOfQuantitiesWithValues; i++)
               allValues.Add(ValuesFor(entityIds[i]));

            return allValues;
         }
      }

      //-------------------------------------------------------------------------------------------------
      /////list of (initial) properties of ALL parameters of the simulation
      //property IList<IParameterProperties^>^ ParameterProperties

      //{
      //   IList < IParameterProperties ^> ^get();
      //}

      /////parameters which should be varied
      //property IList<IParameterProperties^>^ VariableParameters

      //{
      //   IList < IParameterProperties ^> ^get();
      //   void set(IList<IParameterProperties^> ^);
      //}

      /////set parameter initial values for the next simulation run
      //void SetParameterValues(IList<IParameterProperties^>^ parameterProperties);

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
