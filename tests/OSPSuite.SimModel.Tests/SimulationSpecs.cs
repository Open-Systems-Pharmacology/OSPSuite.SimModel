using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml.Linq;
using NUnit.Framework;
using OSPSuite.BDDHelper;
using OSPSuite.BDDHelper.Extensions;

namespace OSPSuite.SimModel.Tests
{
   public abstract class concern_for_Simulation : ContextSpecification<Simulation>
   {
      protected override void Context()
      {
         sut = new Simulation();
         sut.Options.KeepXMLNodeAsString = true;
      }

      protected string TestFileFrom(string shortFilename)
      {
         var location = Path.GetDirectoryName(Assembly.GetCallingAssembly().Location);
         var file = Path.Combine(location, @"..\..\..\..\TestData", shortFilename);

         if (!file.ToLower().EndsWith(".xml"))
            file += ".xml";

         return file;
      }

      protected void LoadFinalizeAndRunSimulation(string shortFileName)
      {
         sut.LoadFromXMLFile(TestFileFrom(shortFileName));
         OptionalTasksBeforeFinalize();
         sut.FinalizeSimulation();
         sut.RunSimulation();
      }

      protected virtual void OptionalTasksBeforeFinalize() { }
   }

   public abstract class when_running_testsystem_06 : concern_for_Simulation
   {
      //---- solving the system:
      //
      // y1' = (P1+P2)*y2 + (P3-Time)*y1 + (y3-2)
      // y2' = y1 + P4
      // y3' = 0 + 0
      //
      // y1(0) = 2
      // y2(0) = P1 + P2 -1
      // y3(0) = y1
      //
      // P1 = sin(y3)^2
      // P2 = cos(y3)^2
      // P3 = Time
      // P4 = y3 - 2
      //
      // System is equivalent to:
      //
      // y1' = y2   y1(0) = 2
      // y2' = y1   y2(0) = 0
      // y3' = 0    y3(0) = 2
      //
      // Analytical solution is:
      //
      // y1 = exp(Time) + exp(-Time)
      // y2 = exp(Time) - exp(-Time)
      // y3 = 2
      //
      // Additionally, an Observer Obs1 is defined as Obs1 = 2*y1
      protected void TestResult()
      {
         //IntPtr sim = sut.GetNativeSimulation();
         int noOfOutputTimePoints = sut.GetNumberOfTimePoints;
         var solverTimes = sut.SimulationTimes;

         var y1 = sut.ValuesFor("y1");
         var y2 = sut.ValuesFor("y2");
         var y3 = sut.ValuesFor("y3");

         var y1_Values = y1.Values;
         var y2_Values = y2.Values;
         var y3_Values = y3.Values;

         y1_Values.Length.ShouldBeEqualTo(noOfOutputTimePoints);
         y2_Values.Length.ShouldBeEqualTo(noOfOutputTimePoints);
         //y3.IsConstantDuringCalculation.ShouldBeTrue(); //TODO! 
         y3_Values.Length.ShouldBeEqualTo(1);

         const double relTol = 1e-5; //max. allowed relative deviation 0.001%

         y3_Values[0].ShouldBeEqualTo(2.0, relTol);

         for (var i = 0; i < noOfOutputTimePoints; i++)
         {
            double time = solverTimes[i];

            y1_Values[i].ShouldBeEqualTo(Math.Exp(time) + Math.Exp(-time), relTol);
            y2_Values[i].ShouldBeEqualTo(Math.Exp(time) - Math.Exp(-time), relTol);
         }
      }
   }

   public class when_running_testsystem_06_without_scale_factor_dense : when_running_testsystem_06
   {
      protected override void Because()
      {
         LoadFinalizeAndRunSimulation("SimModel4_ExampleInput06");
      }

      [Observation]
      public void should_produce_correct_result()
      {
         base.TestResult();
      }

      [Observation]
      public void should_calculate_comparison_threshold()
      {
         //get absolute tolerance used for calculation (might differ from input absolute tolerance)
         double AbsTol = sut.RunStatistics.UsedAbsoluteTolerance;

         //expected threshold for ode variables
         double threshold = 10.0 * AbsTol;

         foreach (var values in sut.AllValues)
         {
            if (values.VariableType == VariableValues.VariableTypes.Observer)
            {
               //the (only) observer is defined as 2*y1 and thus must retrieve the threshold 2*Threshold(y1)
               values.ComparisonThreshold.ShouldBeEqualTo(2.0 * threshold);
            }
            else
            {
               values.ComparisonThreshold.ShouldBeEqualTo(threshold);
            }
         }
      }
   }

   public class when_loading_simulation_from_file_and_running : concern_for_Simulation
   {
      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_load_simulation(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName);
      }

      protected static IEnumerable<string> TestData()
      {
         yield return "SimModel4_ExampleInput05";
      }

      [TestCase]
      [Ignore("Required just for Resharper - otherwise no tests are found in the solution")]
      public void Dummy() { }
   }

   public class when_loading_from_string : concern_for_Simulation
   {
      private string _stringToLoad;

      protected override void Context()
      {
         base.Context();
         _stringToLoad = XDocument.Load(TestFileFrom("SimModel4_ExampleInput05")).ToString();
      }

      [Observation]
      public void should_load_simulation()
      {
         sut.LoadFromXMLString(_stringToLoad);
      }
   }
}