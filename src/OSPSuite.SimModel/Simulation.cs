using System;
using System.Collections.Generic;
using System.Linq;
using System.Resources;
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

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern string GetObjectPathDelimiter(IntPtr simulation);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
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
      public static extern IntPtr GetObserverFromId(IntPtr simulation, int id, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr GetSpeciesFromId(IntPtr simulation, int id, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void GetQuantityProperties(IntPtr quantity, out string containerPath, out string name, out string entityId);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfQuantitiesWithValues(IntPtr simulation, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillEntityIdsForQuantitiesWithValues(IntPtr simulation, [In, Out] string[] entityIds, int size, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillIdsForQuantitiesWithValues(IntPtr simulation, [In, Out] int[] ids, int size, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr CreateParameterInfoVector();

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void DisposeParameterInfoVector(IntPtr parameterInfos);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void ClearParameterInfoVector(IntPtr parameterInfos);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillParameterProperties(IntPtr simulation, IntPtr parameterInfos, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfParameterProperties(IntPtr parameterInfos, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfParameterTablePoints(IntPtr parameterInfos, int parameterIndex, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillSingleParameterProperties(IntPtr simulation, IntPtr parameterInfos,
         int parameterIndex, out string entityId, out string pathWithoutRoot, out string fullName, out double value,
         [In, Out] double[] tablePointsX, [In, Out] double[] tablePointsY, [In, Out] bool[] tablePointsRestartSolver,
         int tablePointsSize, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetParameterValue(IntPtr parameterInfos, int parameterIndex, double value, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetParameterCalculateSensitivity(IntPtr parameterInfos, int parameterIndex, bool calculateSensitivity, out bool success, out string errorMessage);
      
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetParameterTablePoints(IntPtr parameterInfos, int parameterIndex,
         [In] double[] tablePointsX, [In] double[] tablePointsY, [In] bool[] tablePointsRestartSolver, int tablePointsSize,
         out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern bool ParameterIsUsedInSimulation(IntPtr parameterInfos, int parameterIndex, out bool success, out string errorMessage);
      
      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetVariableParameters(IntPtr simulation, IntPtr parameterInfos, [In] int[] parameterIndices,
                                                      int numberOfVariableParameters, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetParameterValues(IntPtr simulation, IntPtr parameterInfos, [In] int[] parameterIndices,
         int numberOfVariableParameters, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr CreateSpeciesInfoVector();

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void DisposeSpeciesInfoVector(IntPtr speciesInfos);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void ClearSpeciesInfoVector(IntPtr speciesInfos);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillSpeciesProperties(IntPtr simulation, IntPtr speciesInfos, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfSpeciesProperties(IntPtr speciesInfos, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillSingleSpeciesProperties(IntPtr simulation, IntPtr speciesInfos,
         int speciesIndex, out string entityId, out string pathWithoutRoot, out string fullName, out double initialValue,
         out double scaleFactor, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetSpeciesInitialValue(IntPtr speciesInfos, int speciesIndex, double initialValue, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetSpeciesScaleFactor(IntPtr speciesInfos, int speciesIndex, double scaleFactor, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern bool SpeciesIsUsedInSimulation(IntPtr speciesInfos, int speciesIndex, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetVariableSpecies(IntPtr simulation, IntPtr speciesInfos, [In] int[] speciesIndices,
                                                      int numberOfVariableSpecies, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void SetSpeciesValues(IntPtr simulation, IntPtr speciesInfos, [In] int[] speciesIndices,
         int numberOfVariableSpecies, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern int GetNumberOfSolverWarnings(IntPtr simulation);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern void FillSolverWarnings(IntPtr simulation, [In, Out] double[] outputTimes, [In, Out] string[] warnings, 
         int size, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr GetQuantityByPath(IntPtr simulation, string quantityPathWithoutRoot, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr ExportSimulationToMatlabCode(IntPtr simulation, string outputFolder, bool fullMode, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr ExportSimulationToCppCode(IntPtr simulation, string outputFolder, bool fullMode, out bool success, out string errorMessage);

      [DllImport(SimModelImportDefinitions.NATIVE_DLL, CallingConvention = SimModelImportDefinitions.CALLING_CONVENTION)]
      public static extern IntPtr ExportSimulationToRCode(IntPtr simulation, string outputFolder, bool fullMode, out bool success, out string errorMessage);

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

   public enum CodeExportMode
   {
      Formula = 1,
      Values = 2
   }

   public enum CodeExportLanguage
   {
      Matlab = 1,
      Cpp = 2,
      R = 3
   }

   public class Simulation : IDisposable
   {
      private readonly IntPtr _simulation;

      private readonly IntPtr _allParameters;
      private IList<ParameterProperties> _variableParameters;

      private readonly IntPtr _allSpecies;
      private IList<SpeciesProperties> _variableSpecies;

      private readonly IList<SolverWarning> _solverWarnings;

      private bool _disposed = false;

      private void evaluateCppCallResult(bool success, string errorMessage)
      {
         PInvokeHelper.EvaluateCppCallResult(success, errorMessage);
      }

      public Simulation()
      {
         _simulation = SimulationImports.CreateSimulation();

         _allParameters = SimulationImports.CreateParameterInfoVector();
         _variableParameters = new List<ParameterProperties>();

         _allSpecies = SimulationImports.CreateSpeciesInfoVector();
         _variableSpecies = new List<SpeciesProperties>();

         _solverWarnings = new List<SolverWarning>();

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

         //initial filling of parameter and species properties (must be done between load and finalize)
         fillParameterAndSpeciesProperties();
      }

      /// <summary>
      /// Load simulation from a SimModel-XML string
      /// </summary>
      public void LoadFromXMLString(string xmlString)
      {
         SimulationImports.LoadSimulationFromXMLString(_simulation, xmlString, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);

         //initial filling of parameter and species properties (must be done between load and finalize)
         fillParameterAndSpeciesProperties();
      }

      private void fillParameterAndSpeciesProperties()
      {
         SimulationImports.FillParameterProperties(_simulation, _allParameters, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);

         SimulationImports.FillSpeciesProperties(_simulation, _allSpecies, out success, out errorMessage);
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

      internal string ObjectPathDelimiter => SimulationImports.GetObjectPathDelimiter(_simulation);

      public void RunSimulation()
      {
         SimulationImports.RunSimulation(_simulation, out var toleranceWasReduced,out var newAbsTol,out var newRelTol, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);

         RunStatistics.ToleranceWasReduced = toleranceWasReduced;
         RunStatistics.UsedAbsoluteTolerance = newAbsTol;
         RunStatistics.UsedRelativeTolerance = newRelTol;

         fillSolverWarnings();
      }

      private void fillSolverWarnings()
      {
         var numberOfWarnings = SimulationImports.GetNumberOfSolverWarnings(_simulation);

         _solverWarnings.Clear();
         if (numberOfWarnings == 0)
            return;

         var times = new double[numberOfWarnings];
         var warnings=new string[numberOfWarnings];

         SimulationImports.FillSolverWarnings(_simulation, times, warnings, numberOfWarnings, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);

         for (var i = 0; i < numberOfWarnings; i++)
         {
            _solverWarnings.Add(new SolverWarning(times[i], warnings[i]));
         }
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

         SimulationImports.GetQuantityProperties(quantity, out var containerPath, out var name, out entityId);
         VariableValues variableValues=new VariableValues(quantity,variableType,entityId, containerPath, name);

         return variableValues;
      }

      /// <summary>
      /// Species or observer time course for a given ID
      ///  - Species also includes parameters with RHS
      ///  - Observer also includes persistable parameters
      /// </summary>
      public VariableValues ValuesFor(int id)
      {
         var quantity = SimulationImports.GetSpeciesFromId(_simulation, id, out var success, out _);
         var variableType = VariableValues.VariableTypes.Species;

         if (!success)
         {
            //entity is not a species. Try observer
            quantity = SimulationImports.GetObserverFromId(_simulation, id, out success, out _);
            if (success)
               variableType = VariableValues.VariableTypes.Observer;
         }

         if (!success)
            throw new Exception($"{id} is not a valid species or observer entity id"); //TODO

         SimulationImports.GetQuantityProperties(quantity, out var containerPath, out var name, out var entityId);
         VariableValues variableValues = new VariableValues(quantity, variableType, entityId, containerPath, name);

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

            var ids =new int[numberOfQuantitiesWithValues];
            SimulationImports.FillIdsForQuantitiesWithValues(_simulation, ids, numberOfQuantitiesWithValues,out success,out errorMessage);

            for (var i=0; i<numberOfQuantitiesWithValues; i++)
               allValues.Add(ValuesFor(ids[i]));

            return allValues;
         }
      }

      public IEnumerable<ParameterProperties> ParameterProperties
      {
         get
         {
            var parameterPropertiesList = new List<ParameterProperties>();

            SimulationImports.FillParameterProperties(_simulation, _allParameters, out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);

            var numberOfParameters = SimulationImports.GetNumberOfParameterProperties(_allParameters, out success, out errorMessage);
            evaluateCppCallResult(success, errorMessage);

            for (var parameterIdx = 0; parameterIdx < numberOfParameters; parameterIdx++)
            {
               var numberOfTablePoints =
                  SimulationImports.GetNumberOfParameterTablePoints(_allParameters, parameterIdx, out success, out errorMessage);
               evaluateCppCallResult(success, errorMessage);

               var tablePointsX = new double[numberOfTablePoints];
               var tablePointsY = new double[numberOfTablePoints];
               var tablePointsRestartSolver = new bool[numberOfTablePoints];

               SimulationImports.FillSingleParameterProperties(_simulation, _allParameters, parameterIdx,
                  out var entityId, out var pathWithoutRoot, out var fullName, out var value,
                  tablePointsX, tablePointsY, tablePointsRestartSolver, numberOfTablePoints,
                  out success, out errorMessage);
               evaluateCppCallResult(success, errorMessage);

               var parameterProperties = new ParameterProperties(_allParameters,parameterIdx,entityId,pathWithoutRoot,fullName,value,
                  tablePointsX, tablePointsY, tablePointsRestartSolver);

               parameterPropertiesList.Add(parameterProperties);
            }

            return parameterPropertiesList;
         }
      }

      public IEnumerable<SpeciesProperties> SpeciesProperties
      {
         get
         {
            var speciesPropertiesList = new List<SpeciesProperties>();

            SimulationImports.FillSpeciesProperties(_simulation, _allSpecies, out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);

            var numberOfSpecies = SimulationImports.GetNumberOfSpeciesProperties(_allSpecies, out success, out errorMessage);
            evaluateCppCallResult(success, errorMessage);

            for (var speciesIdx = 0; speciesIdx < numberOfSpecies; speciesIdx++)
            {
               SimulationImports.FillSingleSpeciesProperties(_simulation, _allSpecies, speciesIdx,
                  out var entityId, out var pathWithoutRoot, out var fullName, out var initialValue, out var scaleFactor,
                  out success, out errorMessage);
               evaluateCppCallResult(success, errorMessage);

               var speciesProperties = new SpeciesProperties(_allSpecies, speciesIdx, entityId, pathWithoutRoot, fullName,
                                                             initialValue, scaleFactor);

               speciesPropertiesList.Add(speciesProperties);
            }

            return speciesPropertiesList;
         }
      }

      private int[] getVariableParameterIndices => _variableParameters.Select(p => p.ParameterIndex).ToArray();

      private int[] getVariableSpeciesIndices => _variableSpecies.Select(p => p.SpeciesIndex).ToArray();

      public IEnumerable<ParameterProperties> VariableParameters
      {
         get => _variableParameters;
         set 
         { 
            _variableParameters = value.ToList();
            var variableParameterIndices = getVariableParameterIndices;

            SimulationImports.SetVariableParameters(_simulation,_allParameters, variableParameterIndices, variableParameterIndices.Length,
                                                    out var success, out var errorMessage);
            evaluateCppCallResult(success,errorMessage);
         }
      }

      public IEnumerable<SpeciesProperties> VariableSpecies
      {
         get => _variableSpecies;
         set
         {
            _variableSpecies = value.ToList();
            var variableSpeciesIndices = getVariableSpeciesIndices;

            SimulationImports.SetVariableSpecies(_simulation, _allSpecies, variableSpeciesIndices, variableSpeciesIndices.Length,
               out var success, out var errorMessage);
            evaluateCppCallResult(success, errorMessage);
         }
      }

      public void SetParameterValues()
      {
         var variableParameterIndices = getVariableParameterIndices;
         SimulationImports.SetParameterValues(_simulation, _allParameters, variableParameterIndices, variableParameterIndices.Length,
            out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public void SetSpeciesValues()
      {
         var variableSpeciesIndices = getVariableSpeciesIndices;
         SimulationImports.SetSpeciesValues(_simulation, _allSpecies, variableSpeciesIndices, variableSpeciesIndices.Length,
            out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);
      }

      public void Dispose()
      {
         Dispose(true);
         GC.SuppressFinalize(this);
      }

      ~Simulation()
      {
         Dispose(false);
      }

      public IEnumerable<SolverWarning> SolverWarnings => _solverWarnings;

      public double[] SensitivityValuesByPathFor(string quantityPath, string parameterPath)
      {
         var quantity =
            SimulationImports.GetQuantityByPath(_simulation, quantityPath, out var success, out var errorMessage);
         evaluateCppCallResult(success, errorMessage);

         var size = QuantityImports.GetQuantityValuesSize(quantity, out success, out errorMessage);
         evaluateCppCallResult(success, errorMessage);

         var values = new double[size];
         QuantityImports.FillSensitivityValues(quantity, values, size, parameterPath, out success, out errorMessage);
         evaluateCppCallResult(success, errorMessage);

         return values;
      }

      public void ExportToCode(string outputFolder, CodeExportLanguage language, CodeExportMode mode)
      {
         var fullMode = (mode == CodeExportMode.Formula);

         switch (language)
         {
            case CodeExportLanguage.Matlab:
               SimulationImports.ExportSimulationToMatlabCode(_simulation, outputFolder, fullMode, out var success,
                  out var errorMessage);
               evaluateCppCallResult(success, errorMessage);
               break;
            case CodeExportLanguage.Cpp:
               SimulationImports.ExportSimulationToCppCode(_simulation, outputFolder, fullMode, out success,
                  out errorMessage);
               evaluateCppCallResult(success, errorMessage);
               break;
            case CodeExportLanguage.R:
               SimulationImports.ExportSimulationToRCode(_simulation, outputFolder, fullMode, out success,
                  out errorMessage);
               evaluateCppCallResult(success, errorMessage);
               break;
            default:
               throw new Exception("Unsupported code export language"); //TODO
         }
      }

      // Protected implementation of Dispose pattern.
      protected virtual void Dispose(bool disposing)
      {
         if (_disposed)
            return;

         SimulationImports.DisposeParameterInfoVector(_allParameters);
         SimulationImports.DisposeSpeciesInfoVector(_allSpecies);
         SimulationImports.DisposeSimulation(_simulation);

         _disposed = true;
      }
   }
}
