using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
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
//         sut.Options.KeepXMLNodeAsString = true;
      }

      protected string TestFileFrom(string shortFilename)
      {
         var location = Path.GetDirectoryName(Assembly.GetCallingAssembly().Location);
         var file = Path.Combine(location, @"..\..\..\..\TestData", shortFilename);

         if (!file.ToLower().EndsWith(".xml"))
            file += ".xml";

         return file;
      }

      protected void LoadSimulation(string shortFileName)
      {
         OptionalTasksBeforeLoad();
         sut.LoadFromXMLFile(TestFileFrom(shortFileName));
      }

      protected void FinalizeSimulation()
      {
         OptionalTasksBeforeFinalize();
         sut.FinalizeSimulation();
      }

      protected void RunSimulation()
      {
         OptionalTasksBeforeRun();
         sut.RunSimulation();
      }

      protected void LoadAndFinalizeSimulation(string shortFileName)
      {
         LoadSimulation(shortFileName);
         FinalizeSimulation();
      }

      protected void LoadFinalizeAndRunSimulation(string shortFileName)
      {
         LoadAndFinalizeSimulation(shortFileName);
         RunSimulation();
      }

      protected virtual void OptionalTasksBeforeLoad() { }

      protected virtual void OptionalTasksBeforeFinalize() { }

      protected virtual void OptionalTasksBeforeRun() { }

      protected ParameterProperties GetParameterByPath(IEnumerable<ParameterProperties> parameterProperties, string path)
      {
         return parameterProperties.FirstOrDefault(p => p.Path.Equals(path));
      }

      //TODO
      //protected SpeciesProperties GetSpeciesByPath(IEnumerable<SpeciesProperties> speciesProperties, string path)
      //{
      //   return speciesProperties.FirstOrDefault(p => p.Path.Equals(path));
      //}

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
      protected static IEnumerable<string> TestData()
      {
         yield return "SimModel4_ExampleInput06";
         yield return "SimModel4_ExampleInput06_NewSchema";
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_produce_correct_result(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName);
         base.TestResult();
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_calculate_comparison_threshold(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName);

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

   public class when_running_testsystem_06_setting_all_parameters_as_variable_dense : when_running_testsystem_06
   {
      protected override void OptionalTasksBeforeFinalize()
      {
         //---- set all parameters as variable
         sut.VariableParameters = sut.ParameterProperties;
      }

      protected static IEnumerable<string> TestData()
      {
         yield return "SimModel4_ExampleInput06";
         yield return "SimModel4_ExampleInput06_NewSchema";
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_produce_correct_result(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName);
         base.TestResult();
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

   public class when_getting_all_parameter_values_and_all_initial_values : concern_for_Simulation
   {
      [Observation]
      public void should_return_correct_value_for_dependent_parameters_and_initial_value_before_and_after_changing_of_basis_parameter()
      {
         sut.LoadFromXMLFile(TestFileFrom("TestAllParametersInitialValues"));

         var allParameters = sut.ParameterProperties.ToList();

         //---- set P1 and P2 as variable
         var variableParameters = new ParameterProperties[]
         {
            GetParameterByPath(allParameters, "P1"),
            GetParameterByPath(allParameters, "P2")
         };

         sut.VariableParameters = variableParameters;

         sut.FinalizeSimulation();

         //value of P10 should be equal P1+P2, which is initially 1
         GetParameterByPath(sut.ParameterProperties, "P10").Value.ShouldBeEqualTo(1.0, 1e-5);

         //initial value of y2 should be equal P1+P2-1, which is initially 0
         //TODO
         //GetSpeciesByPath(sut.SpeciesProperties, "y2").Value.ShouldBeEqualTo(0.0, 1e-5);

         //update variable parameters
         variableParameters = sut.VariableParameters.ToArray();
         GetParameterByPath(variableParameters, "P1").Value = 3;
         GetParameterByPath(variableParameters, "P2").Value = 4;

         //---- set new parameter values
         sut.SetParameterValues();

         //value of P10 should be equal P1+P2, which is now 7
         GetParameterByPath(sut.ParameterProperties, "P10").Value.ShouldBeEqualTo(7.0, 1e-5);

         //initial value of y2 should be equal P1+P2-1, which is now 6
         //TODO
         //GetSpeciesByPath(sut.SpeciesProperties, "y2").Value.ShouldBeEqualTo(6.0, 1e-5);

      }
   }

   public class when_running_system_with_all_constant_species : concern_for_Simulation
   {
      protected override void Because()
      {
         LoadFinalizeAndRunSimulation("SimModel4_ExampleInput04");
      }

      [Observation]
      public void should_return_constant_values_for_all_species()
      {
         var numberOfTimePoints = sut.GetNumberOfTimePoints;
         numberOfTimePoints.ShouldBeEqualTo(120);

         var speciesList = sut.AllValues.Where(values => values.VariableType == VariableValues.VariableTypes.Species).ToList();
         speciesList.Count.ShouldBeGreaterThan(0);

         foreach (var species in speciesList)
         {
            species.Values.Length.ShouldBeEqualTo(1);
            species.IsConstant.ShouldBeTrue();
         }
      }

   }
}