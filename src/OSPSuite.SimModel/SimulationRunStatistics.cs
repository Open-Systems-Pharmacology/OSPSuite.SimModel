using System;

namespace OSPSuite.SimModel
{
   /// <summary>
   /// Stores some information about successful simulation run: used tolerances, number of ODE variables, ...
   /// </summary>
   public class SimulationRunStatistics
   {
      private readonly IntPtr _simulation;

      public SimulationRunStatistics(IntPtr simulation)
      {
         _simulation = simulation;
         ToleranceWasReduced = false;
         UsedAbsoluteTolerance = double.NaN;
         UsedRelativeTolerance = double.NaN;
      }

      /// <summary>
      /// Returns true, if tolerance reduction was used to solve the ODE system.
      /// (can only be the case if <see cref="SimulationOptions.AutoReduceTolerances"/> was set to <value>true</value>
      /// </summary>
      public bool ToleranceWasReduced { get; internal set; }

      /// <summary>
      /// Returns absolute tolerance used to solve the ODE system.
      /// Might deviate from the absolute tolerance set by user
      /// (can only be the case if <see cref="SimulationOptions.AutoReduceTolerances"/> was set to <value>true</value>
      /// </summary>
      public double UsedAbsoluteTolerance { get; internal set; }

      /// <summary>
      /// Returns relative tolerance used to solve the ODE system.
      /// Might deviate from the relative tolerance set by user
      /// (can only be the case if <see cref="SimulationOptions.AutoReduceTolerances"/> was set to <value>true</value>
      /// </summary>
      public double UsedRelativeTolerance { get; internal set; }

   }
}
