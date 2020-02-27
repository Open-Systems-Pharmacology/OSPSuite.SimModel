using System;
using System.Runtime.InteropServices;
using OSPSuite.Utility.Exceptions;

// ReSharper disable UnusedMember.Global

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

   public class SimulationOptions
   {
      private readonly IntPtr _simulation;
      private SimulationOptionsStructure _simulationOptions;
      private string _logFile = "";

      public SimulationOptions(IntPtr simulation)
      {
         _simulation = simulation;
         //_simulationOptions = SimulationImports.GetSimulationOptions(_simulation);
         SimulationImports.FillSimulationOptions(_simulation, ref _simulationOptions);
      }

      private void setOptions(Action action)
      {
         action();
         SimulationImports.SetSimulationOptions(_simulation, _simulationOptions);
      }

      /// <summary>
      /// Defines im simulation  progress should be calculated during simulation <para></para>
      /// Progress can be shown by calling program.
      /// </summary>
      public bool ShowProgress
      {
         get => _simulationOptions.ShowProgress;
         set => setOptions(() => _simulationOptions.ShowProgress = value);
      }

      /// <summary>
      /// If a simulation is still running after /<ExecutionTimeLimit/> seconds: it will be stopped with exception.
      /// Setting /<ExecutionTimeLimit/> to 0 will deactivate this feature.
      /// Default is <value>0</value>
      /// </summary>
      public double ExecutionTimeLimit
      {
         get => _simulationOptions.ExecutionTimeLimit;
         set => setOptions(() => _simulationOptions.ExecutionTimeLimit = value);
      }

      /// <summary>
      /// Sets if ODE solver warnings should be treated as errors
      /// Default is <value>true</value>
      /// </summary>
      public bool StopOnWarnings
      {
         get => _simulationOptions.StopOnWarnings;
         set => setOptions(() => _simulationOptions.StopOnWarnings = value);
      }

      /// <summary>
      /// If solving of the diff. eq. systems fails with a convergence error <para></para>
      /// AND the value of AutoReduceTolerances is TRUE, both absolute and relative <para></para>
      /// tolerance will be reduced by 10, until either the system was successfully solved <para></para>
      /// or the predefined lower bounds for both tolerances were reached
      /// Default is <value>true</value>
      /// </summary>
      public bool AutoReduceTolerances
      {
         get => _simulationOptions.AutoReduceTolerances;
         set => setOptions(() => _simulationOptions.AutoReduceTolerances = value);
      }

      /// <summary>
      /// If set to true AND a log file name is set as well:
      /// some debug information will be exported into a log file
      /// Default is <value>true</value>
      /// </summary>
      public bool WriteLogFile
      {
         get => _simulationOptions.WriteLogFile;
         set => setOptions(() => _simulationOptions.WriteLogFile = value);
      }

      /// <summary>
      /// Enables/disables checking for negative values of positive ODE variables.
      /// Default value is <value>true</value>
      /// </summary>
      public bool CheckForNegativeValues
      {
         get => _simulationOptions.CheckForNegativeValues;
         set => setOptions(() => _simulationOptions.CheckForNegativeValues = value);
      }

      /// <summary>
      /// Sets if simulation should be validated against OSPSuite.SimModel-schema during loading.
      /// Default value is <value>false</value>
      /// </summary>
      public bool ValidateWithXMLSchema
      {
         get => _simulationOptions.ValidateWithXMLSchema;
         set => setOptions(() => _simulationOptions.ValidateWithXMLSchema = value);
      }

      /// <summary>
      /// Sets if only parameters effectively used in ODE Variables and/or Observers
      /// should be marked as /"used/". If set to false: ALL Parameters will be marked as used.
      /// Default value is <value>false</value>
      /// </summary>
      public bool IdentifyUsedParameters
      {
         get => _simulationOptions.IdentifyUsedParameters;
         set => setOptions(() => _simulationOptions.IdentifyUsedParameters = value);
      }

      /// <summary>
      /// Sets if the original simulation xml string used for simulation loading should be stored
      /// permanently. Must be set to <value>true</value> BEFORE loading simulation if it is intended
      /// to save a (modified) simulation to XML later on.
      /// Default value is <value>false</value>
      /// </summary>
      public bool KeepXMLNodeAsString
      {
         get => _simulationOptions.KeepXMLNodeAsString;
         set => setOptions(() => _simulationOptions.KeepXMLNodeAsString = value);
      }

      /// <summary>
      /// If set to <value>true</value>, float comparison will be used for user output time points.
      /// Otherwise: double comparison.
      /// Default value is <value>true</value>
      /// </summary>
      public bool UseFloatComparisonInUserOutputTimePoints
      {
         get => _simulationOptions.UseFloatComparisonInUserOutputTimePoints;
         set => setOptions(() => _simulationOptions.UseFloatComparisonInUserOutputTimePoints = value);
      }

      public string LogFile
      {
         get => _logFile;
         set => throw new OSPSuiteException("Not implemented yet");
      }
   }
}
