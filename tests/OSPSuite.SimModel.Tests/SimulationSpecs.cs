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
         var file = Path.Combine(location, @"../../../../TestData", shortFilename);

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

      protected virtual void OptionalTasksBeforeLoad()
      {
      }

      protected virtual void OptionalTasksBeforeFinalize()
      {
      }

      protected virtual void OptionalTasksBeforeRun()
      {
      }

      protected ParameterProperties GetParameterByPath(IEnumerable<ParameterProperties> parameterProperties,
         string path)
      {
         return parameterProperties.FirstOrDefault(p => p.Path.Equals(path));
      }

      protected SpeciesProperties GetSpeciesByPath(IEnumerable<SpeciesProperties> speciesProperties, string path)
      {
         return speciesProperties.FirstOrDefault(p => p.Path.Equals(path));
      }

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
         TestResult();
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

   public class when_running_testsystem_06_with_scale_factor_dense : when_running_testsystem_06
   {
      private double _scaleFactor;

      protected override void OptionalTasksBeforeFinalize()
      {
         //_scaleFactor = 100;

         //GetSpeciesByPath(sut.SpeciesProperties, "y1").ScaleFactor = _scaleFactor;
         //GetSpeciesByPath(sut.SpeciesProperties, "y2").ScaleFactor = _scaleFactor;
         //GetSpeciesByPath(sut.SpeciesProperties, "y3").ScaleFactor = _scaleFactor;

         var allSpecies = sut.SpeciesProperties.ToList();
         var variableSpecies = new[]
         {
            GetSpeciesByPath(allSpecies, "y1"),
            GetSpeciesByPath(allSpecies, "y2"),
            GetSpeciesByPath(allSpecies, "y3")
         };

         sut.VariableSpecies = variableSpecies;
      }

      protected override void OptionalTasksBeforeRun()
      {
         _scaleFactor = 100;

         GetSpeciesByPath(sut.SpeciesProperties, "y1").ScaleFactor = _scaleFactor;
         GetSpeciesByPath(sut.SpeciesProperties, "y2").ScaleFactor = _scaleFactor;
         GetSpeciesByPath(sut.SpeciesProperties, "y3").ScaleFactor = _scaleFactor;

         sut.SetSpeciesValues();
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
         TestResult();
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_calculate_comparison_threshold(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName);

         //get absolute tolerance used for calculation (might differ from input absolute tolerance)
         double AbsTol = sut.RunStatistics.UsedAbsoluteTolerance;

         //expected threshold for ode variables without scaling
         double threshold = 10.0 * AbsTol;

         foreach (var values in sut.AllValues)
         {
            if (values.VariableType == VariableValues.VariableTypes.Observer)
            {
               //the (only) observer is defined as 2*y1 and thus must retrieve the threshold 2*Threshold(y1)
               values.ComparisonThreshold.ShouldBeEqualTo(2.0 * threshold * _scaleFactor);
            }
            else
            {
               //for non-const variables, Threshold must be equal to DefaultThreshold*ScaleFactor
               var variableThreshold = values.IsConstant ? threshold : threshold * _scaleFactor;
               values.ComparisonThreshold.ShouldBeEqualTo(variableThreshold);
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
         TestResult();
      }

   }

   public class
      when_running_testsystem_06_modified_setting_parameter_values_and_initial_values : when_running_testsystem_06
   {
      //Modifications made in the system (compared to the test system) in XML
      // y2(0) changed from P1 + P2 -1 (=0) to 10
      // P1 changed from sin(y3)^2
      // P2 changed from cos(y3)^2
      //
      // In order to get the original system, one must set:
      // y2(0) = 0
      // P1+P2=1


      protected override void OptionalTasksBeforeFinalize()
      {
         //---- set P1 and P2 as variable
         var allParameters = sut.ParameterProperties.ToList();
         var variableParameters = new[]
         {
            GetParameterByPath(allParameters, "Subcontainer1/P1"),
            GetParameterByPath(allParameters, "Subcontainer1/P2")
         };

         sut.VariableParameters = variableParameters;

         //---- set y2 initial value as variable
         var allSpecies = sut.SpeciesProperties.ToList();
         var variableSpecies = new[]
         {
            GetSpeciesByPath(allSpecies, "Subcontainer1/y2")
         };

         sut.VariableSpecies = variableSpecies;
      }

      protected override void OptionalTasksBeforeRun()
      {
         //---- update variable parameters: set P1+P2=1
         var variableParameters = sut.VariableParameters.ToList();

         GetParameterByPath(variableParameters, "Subcontainer1/P1").Value = 0.3;
         GetParameterByPath(variableParameters, "Subcontainer1/P2").Value = 0.7;

         sut.SetParameterValues();

         //---- update variable species: set y2(0)=0
         var variableSpecies = sut.VariableSpecies.ToList();
         GetSpeciesByPath(variableSpecies, "Subcontainer1/y2").InitialValue = 0;

         sut.SetSpeciesValues();
      }

      protected static IEnumerable<string> TestData()
      {
         yield return "SimModel4_ExampleInput06_Modified";
         yield return "SimModel4_ExampleInput06_Modified_V4";
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_produce_correct_result(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName);
         TestResult();
      }
   }

   public class when_running_testsystem_06_modified_setting_table_parameter_values : when_running_testsystem_06
   {
      //in the model, y4 is defined as following:
      //
      // y4(0) = 33
      // y4'   = P5
      // P5 is table parameter const 0

      private bool _modifySystem;

      protected override void OptionalTasksBeforeFinalize()
      {
         if (!_modifySystem)
            return;

         //---- set P5 as variable
         var allParameters = sut.ParameterProperties.ToList();

         var variableParameters = new[]
         {
            GetParameterByPath(allParameters, "Subcontainer1/P5")
         };

         sut.VariableParameters = variableParameters;
      }

      protected override void OptionalTasksBeforeRun()
      {
         if (!_modifySystem)
            return;

         //---- set P5 as table:
         //   0,  0
         //   5,  10
         //   10, 10
         //After this changes:
         //
         //y4'(t) = 2   for 0<=t<=5
         //y4'(t) = 0   for t>5
         //
         //=> y4(t)=y4(0)+2*t for 0<=t<=5
         //   y4(t)=y4(5)=y4(0)+10 for t>=5

         GetParameterByPath(sut.VariableParameters, "Subcontainer1/P5").TablePoints = new[]
         {
            new ValuePoint(0, 0, false),
            new ValuePoint(5, 10, false),
            new ValuePoint(10, 10, false)
         };

         sut.SetParameterValues();
      }

      protected static IEnumerable<string> TestData()
      {
         yield return "SimModel4_ExampleInput06_Modified";
         yield return "SimModel4_ExampleInput06_Modified_V4";
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_produce_correct_result_with_and_without_changing_of_table_parameter(string shortFileName)
      {
         //---- run without parameter modification
         _modifySystem = false;
         LoadFinalizeAndRunSimulation(shortFileName);
         var y4_without_change = sut.ValuesFor("y4").Values;

         //---- run with table parameter modification
         _modifySystem = true;
         LoadFinalizeAndRunSimulation(shortFileName);
         var y4_with_change = sut.ValuesFor("y4").Values;

         var outputTimes = sut.SimulationTimes;

         const double relTol = 1e-5; //max. allowed relative deviation 0.001%

         for (var i = 0; i < outputTimes.Length; i++)
         {
            //without modification y4=const=y4(0)
            y4_without_change[i].ShouldBeEqualTo(y4_without_change[0], relTol);

            //with modification
            //   y4(t) = y4(0)+2*t         for 0<=t<=5
            //   y4(t) = y4(5) = y4(0)+10  for t>=5
            var time = outputTimes[i];
            y4_with_change[i].ShouldBeEqualTo(y4_with_change[0] + (time <= 5 ? 2 * time : 10), relTol);
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
      public void Dummy()
      {
      }
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
      protected override void OptionalTasksBeforeFinalize()
      {
         var allParameters = sut.ParameterProperties.ToList();

         //---- set P1 and P2 as variable
         var variableParameters = new[]
         {
            GetParameterByPath(allParameters, "P1"),
            GetParameterByPath(allParameters, "P2")
         };

         sut.VariableParameters = variableParameters;
      }

      [Observation]
      public void
         should_return_correct_value_for_dependent_parameters_and_initial_value_before_and_after_changing_of_basis_parameter()
      {
         LoadAndFinalizeSimulation("TestAllParametersInitialValues");

         //value of P10 should be equal P1+P2, which is initially 1
         GetParameterByPath(sut.ParameterProperties, "P10").Value.ShouldBeEqualTo(1.0, 1e-5);

         //initial value of y2 should be equal P1+P2-1, which is initially 0
         GetSpeciesByPath(sut.SpeciesProperties, "y2").InitialValue.ShouldBeEqualTo(0.0, 1e-5);

         //update variable parameters
         var variableParameters = sut.VariableParameters.ToArray();
         GetParameterByPath(variableParameters, "P1").Value = 3;
         GetParameterByPath(variableParameters, "P2").Value = 4;

         //---- set new parameter values
         sut.SetParameterValues();

         //value of P10 should be equal P1+P2, which is now 7
         GetParameterByPath(sut.ParameterProperties, "P10").Value.ShouldBeEqualTo(7.0, 1e-5);

         //initial value of y2 should be equal P1+P2-1, which is now 6
         GetSpeciesByPath(sut.SpeciesProperties, "y2").InitialValue.ShouldBeEqualTo(6.0, 1e-5);
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

         var speciesList = sut.AllValues.Where(values => values.VariableType == VariableValues.VariableTypes.Species)
            .ToList();
         speciesList.Count.ShouldBeGreaterThan(0);

         foreach (var species in speciesList)
         {
            species.Values.Length.ShouldBeEqualTo(1);
            species.IsConstant.ShouldBeTrue();
         }
      }

   }

   public class when_solving_A_exp_minus_kT_with_sensitivity : concern_for_Simulation
   {
      private double _A0;
      private double _k;

      private double A0 => _A0;
      private double k => _k;

      protected override void Because()
      {
         LoadSimulation("S3_reduced");
         var parameterProperties = sut.ParameterProperties;
         var variableParameters = new List<ParameterProperties>();

         foreach (var param in parameterProperties)
         {
            if (param.EntityId.Equals("A0") || param.EntityId.Equals("k"))
            {
               param.CalculateSensitivity = true;
               variableParameters.Add(param);

               if (param.EntityId.Equals("A0"))
                  _A0 = param.Value;
               else
                  _k = param.Value;
            }
         }

         sut.VariableParameters = variableParameters;
         sut.FinalizeSimulation();
         sut.RunSimulation();
      }

      [Observation]
      public void should_solve_example_system_and_return_correct_sensitivity_values()
      {
         //The (only) variable of the system d(C1)/dt=-k*C1; C1(0)=A0
         //Solution: C1(t)=A0*exp(-kt)
         //Sensitivities: d(C1)/d(A0) = exp(-kt)
         //               d(C1)/d(k)  = -A0*t*exp(-kt)

         var solverTimes = sut.SimulationTimes;
         var noOfOutputTimePoints = solverTimes.Length;
         var C1 = sut.ValuesFor("C1").Values;

         var dC1_dA0 = sut.SensitivityValuesByPathFor("Organism|C1", "Organism|A0");
         var dC1_dk = sut.SensitivityValuesByPathFor("Organism|C1", "Organism|k");
         const double relTol = 1e-3;

         for (var i = 1; i < noOfOutputTimePoints; i++)
         {
            var t = solverTimes[i];
            C1[i].ShouldBeEqualTo(A0 * Math.Exp(-k * t), relTol);

            //TODO remove comment as soon as https://github.com/Open-Systems-Pharmacology/OSPSuite.SimModel/issues/16 is fixed
            //dC1_dA0[i].ShouldBeEqualTo(Math.Exp(-k * t), relTol);

            dC1_dk[i].ShouldBeEqualTo(-A0 * t * Math.Exp(-k * t), relTol);
         }

      }
   }

   public class when_calculating_sensitivity_of_persistable_parameter : concern_for_Simulation
   {
      protected override void Because()
      {
         LoadSimulation("SensitivityOfPersistableParameter");
         var parameterProperties = sut.ParameterProperties;
         var variableParameters = new List<ParameterProperties>();

         foreach (var param in parameterProperties)
         {
            if (param.EntityId.Equals("Volume"))
            {
               param.CalculateSensitivity = true;
               variableParameters.Add(param);
            }
         }

         sut.VariableParameters = variableParameters;
         sut.FinalizeSimulation();
         sut.RunSimulation();
      }

      [Observation]
      public void should_calculate_sensitivity_values_of_persistable_parameter()
      {
         sut.SensitivityValuesByPathFor("Organism|P1", "Organism|Volume");
      }
   }

   public class
      when_solving_cvsRoberts_FSA_dns_with_sensitivity_Sensitivity_RHS_function_not_set : concern_for_Simulation
   {
      private const int _numberOfTimeSteps = 2;
      private const int _numberOfUnknowns = 3;
      private const int _numberOfSensitivityParameters = 3;
      private double[,,] _expectedSensitivities;

      protected override void Because()
      {
         _expectedSensitivities = fillExpectedSensitivities();

         LoadSimulation("cvsRoberts_FSA_dns");
         var parameterProperties = sut.ParameterProperties;
         var variableParameters = new List<ParameterProperties>();

         foreach (var param in parameterProperties)
         {
            if (param.EntityId.Equals("P1") || param.EntityId.Equals("P2") || param.EntityId.Equals("P3"))
            {
               param.CalculateSensitivity = true;
               variableParameters.Add(param);
            }
         }

         sut.VariableParameters = variableParameters;

         sut.FinalizeSimulation();
         sut.RunSimulation();
      }

      //values produced via direct usage of CVODES
      private double[,,] fillExpectedSensitivities()
      {
         double[,,] sensitivities =
         {
            {
               //time step #1
               {-3.5611e-001, 9.4831e-008, -1.5733e-011}, // {dy1/dp1, dy1/dp2, dy1/dp3}
               {3.9023e-004, -2.1325e-010, -5.2897e-013}, // {dy2/dp1, dy2/dp2, dy2/dp3}
               {3.5572e-001, -9.4618e-008, 1.6262e-011} // {dy3/dp1, dy3/dp2, dy3/dp3}
            },

            {
               //time step #2
               {-1.8761e+000, 2.9612e-006, -4.9330e-010}, // {dy1/dp1, dy1/dp2, dy1/dp3}
               {1.7922e-004, -5.8308e-010, -2.7624e-013}, // {dy2/dp1, dy2/dp2, dy2/dp3}
               {1.8760e+000, -2.9606e-006, 4.9357e-010} // {dy3/dp1, dy3/dp2, dy3/dp3}
            }
         };

         return sensitivities;
      }

      private void checkSensitivities(double[] dy1_dp1, double[] dy1_dp2, double[] dy1_dp3,
         double[] dy2_dp1, double[] dy2_dp2, double[] dy2_dp3,
         double[] dy3_dp1, double[] dy3_dp2, double[] dy3_dp3,
         double[] dObs1_dp1, double[] dObs1_dp2, double[] dObs1_dp3)
      {
         double[,,] sensitivities =
         {
            {
               //time step #1
               {dy1_dp1[1], dy1_dp2[1], dy1_dp3[1]}, // {dy1/dp1, dy1/dp2, dy1/dp3}
               {dy2_dp1[1], dy2_dp2[1], dy2_dp3[1]}, // {dy2/dp1, dy2/dp2, dy2/dp3}
               {dy3_dp1[1], dy3_dp2[1], dy3_dp3[1]} // {dy3/dp1, dy3/dp2, dy3/dp3}
            },

            {
               //time step #2
               {dy1_dp1[2], dy1_dp2[2], dy1_dp3[2]}, // {dy1/dp1, dy1/dp2, dy1/dp3}
               {dy2_dp1[2], dy2_dp2[2], dy2_dp3[2]}, // {dy2/dp1, dy2/dp2, dy2/dp3}
               {dy3_dp1[2], dy3_dp2[2], dy3_dp3[2]} // {dy3/dp1, dy3/dp2, dy3/dp3}
            }
         };

         //---- TODO ------------------------------------------------------
         //Test passes with relTol 1e-2 but fails already with relTol 1e-3
         //This should be investigated further! Deviation seems too high for me
         //
         //Test output with relTol=1e-3:
         //    Time step: 1 Variable: 1 Parameter: 2 Expected sensitivity: 9,4831E-08 Returned sensitivity: 9,54238142897576E-08
         //    9,54238142897576E-08 and 9,4831E-08 are not equal within relative tolerance 0,001
         //----------------------------------------------------------------
         const double relTol = 1e-2; //max. allowed relative deviation 1%

         for (var i = 0; i < _numberOfTimeSteps; i++)
         {
            for (var j = 0; j < _numberOfUnknowns; j++)
            {
               for (var k = 0; k < _numberOfSensitivityParameters; k++)
               {
                  var msg =
                     $"Time step: {i + 1}\nVariable: {j + 1}\nParameter: {k + 1}\nExpected sensitivity: {_expectedSensitivities[i, j, k]}\nReturned sensitivity: {sensitivities[i, j, k]}\n";
                  sensitivities[i, j, k].ShouldBeEqualTo(_expectedSensitivities[i, j, k], relTol, msg);
               }
            }
         }

         //check observer sensitivity values. 
         //Observer is defined as y1+2*y2+3*y3
         //the same must apply for all parameter sensitivities
         double[,] observerSensitivities =
         {
            {
               //time step #1
               dObs1_dp1[1], dObs1_dp2[1], dObs1_dp3[1]
            },

            {
               //time step #2
               dObs1_dp1[2], dObs1_dp2[2], dObs1_dp3[2]
            }
         };


         for (var i = 0; i < _numberOfTimeSteps; i++)
         {
            for (var k = 0; k < _numberOfSensitivityParameters; k++)
            {
               double expectedSensitivity =
                  sensitivities[i, 0, k] + 2 * sensitivities[i, 1, k] + 3 * sensitivities[i, 2, k];
               var msg =
                  $"Time step: {i + 1}\nParameter: {k + 1}\nExpected sensitivity: {expectedSensitivity}\nReturned sensitivity: {observerSensitivities[i, k]}\n";
               observerSensitivities[i, k].ShouldBeEqualTo(expectedSensitivity, relTol, msg);
            }
         }
      }

#if !_WINDOWS
      [Ignore("Ignore under Linux for the moment")]
#endif
      [Observation]
      public void should_solve_example_system_and_return_correct_sensitivity_values()
      {
         var dy1_dp1 = sut.SensitivityValuesByPathFor("SubContainer/y1", "SubContainer/P1");
         var dy1_dp2 = sut.SensitivityValuesByPathFor("SubContainer/y1", "SubContainer/P2");
         var dy1_dp3 = sut.SensitivityValuesByPathFor("SubContainer/y1", "SubContainer/P3");

         var dy2_dp1 = sut.SensitivityValuesByPathFor("SubContainer/y2", "SubContainer/P1");
         var dy2_dp2 = sut.SensitivityValuesByPathFor("SubContainer/y2", "SubContainer/P2");
         var dy2_dp3 = sut.SensitivityValuesByPathFor("SubContainer/y2", "SubContainer/P3");

         var dy3_dp1 = sut.SensitivityValuesByPathFor("SubContainer/y3", "SubContainer/P1");
         var dy3_dp2 = sut.SensitivityValuesByPathFor("SubContainer/y3", "SubContainer/P2");
         var dy3_dp3 = sut.SensitivityValuesByPathFor("SubContainer/y3", "SubContainer/P3");

         var dObs1_dp1 = sut.SensitivityValuesByPathFor("SubContainer/Obs1", "SubContainer/P1");
         var dObs1_dp2 = sut.SensitivityValuesByPathFor("SubContainer/Obs1", "SubContainer/P2");
         var dObs1_dp3 = sut.SensitivityValuesByPathFor("SubContainer/Obs1", "SubContainer/P3");

         checkSensitivities(dy1_dp1, dy1_dp2, dy1_dp3, dy2_dp1, dy2_dp2, dy2_dp3, dy3_dp1, dy3_dp2, dy3_dp3, dObs1_dp1,
            dObs1_dp2, dObs1_dp3);
      }
   }

   public class loading_cvsRoberts_FSA_dns : concern_for_Simulation
   {
#if !_WINDOWS
      [Ignore("Ignore under Linux for the moment")]
#endif
      [Observation]
      public void should_load_simulation()
      {
         LoadSimulation("cvsRoberts_FSA_dns");
      }
   }
}