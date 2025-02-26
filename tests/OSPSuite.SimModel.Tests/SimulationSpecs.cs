using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Xml.Linq;
using NUnit.Framework;
using OSPSuite.BDDHelper;
using OSPSuite.BDDHelper.Extensions;
using OSPSuite.Utility.Extensions;

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

      protected string CreateTempFolder()
      {
         var tempFolder = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
         Directory.CreateDirectory(tempFolder);
         return tempFolder;
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

      protected void LoadFinalizeAndRunSimulation(string shortFileName, bool performBasicTests = false)
      {
         LoadAndFinalizeSimulation(shortFileName);
         RunSimulation();

         if (performBasicTests)
            SimulationOutputsBasicTest();
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

      protected void SimulationOutputsBasicTest()
      {
         var noOfOutputTimePoints = sut.SimulationTimes.Length;
         noOfOutputTimePoints.ShouldBeEqualTo(sut.GetNumberOfTimePoints);

         //make sure that all const variables have length(values)=1 and
         //all non-const variables have the length of the time array
         foreach (var variableValues in sut.AllValues)
         {
            variableValues.Values.Length.ShouldBeEqualTo(variableValues.IsConstant ? 1 : noOfOutputTimePoints);
         }
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

   public class when_running_testsystem_06_modified_setting_parameter_values_and_initial_values : when_running_testsystem_06
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
      public void should_load_finalize_and_run_simulation(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName, performBasicTests:true);
      }

      protected static IEnumerable<string> TestData()
      {
         yield return "12.0_Brockmoller2005_CPA";
         yield return "SimModel4_ExampleInput05";
         yield return "IfFormulaInSwitchCondition";
         yield return "PKSim_Input_01";
         yield return "PKSim_Input_04_MultiApp";
         yield return "AdultPopulation";
         yield return "OralTable01";
         yield return "GrowConst";
         yield return "GIM_Table_01";
         yield return "pH_Solubility_Table"; 
         yield return "pH_Solubility_Table_Zero"; 
         yield return "pH_Solubility_Table_Const";
         yield return "Test4Model_Reduced03";
         //yield return "Neg_t_TimeSinceMeal"; //TODO "Enable test in case negative meal offset will be allowed"
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

         //remove comment as soon as ... (s. below)
         //var dC1_dA0 = sut.SensitivityValuesByPathFor("Organism|C1", "Organism|A0");
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

   public class when_solving_cvsRoberts_FSA_dns_with_sensitivity_Sensitivity_RHS_function_not_set : concern_for_Simulation
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

   public class when_loading_cvsRoberts_FSA_dns : concern_for_Simulation
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

   public class when_loading_simulation_from_file : concern_for_Simulation
   {
      protected static IEnumerable<string> TestData()
      {
         yield return "InfinityTest";
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_load_simulation(string shortFileName)
      {
         LoadSimulation(shortFileName);
      }

   }

   public class when_loading_simulation_with_table_formulas : concern_for_Simulation
   {
      [Observation]
      public void should_load_simulation()
      {
         LoadSimulation("TableParametersViaDCI_Test01");

         var parametersWithTableFormulas = sut.ParameterProperties.Where(p => p.IsTable);
         parametersWithTableFormulas.Count().ShouldBeGreaterThan(0);
      }
   }

   public class when_exporting_bcm_platelet_to_matlab : concern_for_Simulation
   {
      [Observation]
      public void should_export_to_matlab_in_formula_mode()
      {
         LoadSimulation("BCM_Platelet6Lit");
         sut.ExportToCode(CreateTempFolder(), CodeExportLanguage.Matlab, CodeExportMode.Formula);
      }

      [Observation]
      public void should_export_to_matlab_in_value_mode()
      {
         LoadSimulation("BCM_Platelet6Lit");
         sut.ExportToCode(CreateTempFolder(), CodeExportLanguage.Matlab, CodeExportMode.Values);
      }
   }

   public class when_running_simulation_returning_not_allowed_negative_values : concern_for_Simulation
   {
      [Observation]
      public void
         should_perform_simulation_run_without_negative_values_check_and_throw_an_exception_with_negative_values_check()
      {
         LoadAndFinalizeSimulation("NegativeValuesTestSimple");

         sut.Options.CheckForNegativeValues = false;
         try
         {
            RunSimulation();
         }
         catch (Exception)
         {
            throw new Exception("Exception was thrown but negative values check was deactivated");
         }

         sut.Options.CheckForNegativeValues = true;
         try
         {
            RunSimulation();
         }
         catch (Exception ex)
         {
            ex.Message.Contains("negative").ShouldBeTrue();
            ex.Message.Contains("when trying to reach t=").ShouldBeTrue();

            //expected behavior. Leave the test case
            return;
         }

         //failed (no exception with negative values check)
         throw new Exception("No exception was thrown with negative values check");
      }
   }

   public class when_running_simulation_with_events_simultaneously_increasing_a_variable : concern_for_Simulation
   {
      [Observation]
      public void should_perform_all_events()
      {
         LoadFinalizeAndRunSimulation("TestSimultanEvents");

         //simulation has only 1 constant variable with start value 0, which is modified by 2 events:
         //    event #1: M=M+10
         //    event #2: M=M+20
         // both events fire at t=10; simulation output interval is [0..30]
         //
         //thus at the end it must be: Variable[0]=0 and Variable[lastIndex]=30

         var values = sut.AllValues.First().Values;

         values[0].ShouldBeEqualTo(0.0, 1e-5);
         values[values.Length - 1].ShouldBeEqualTo(30.0, 1e-5);
      }
   }

   public class when_running_simulation_with_almost_equal_output_times : concern_for_Simulation
   {
      [Observation]
      public void should_remove_duplicate_time_points_according_to_comparison()
      {
         const double FLOAT_PRECISION = 1e-8;

         //first, run simulation using (default) float comparison for user output points
         LoadFinalizeAndRunSimulation("NonMonotoneBasegridTest");

         var solverTimes = sut.SimulationTimes;
         for (var i = 0; i < solverTimes.Length - 1; i++)
            Math.Abs(solverTimes[i] - solverTimes[i + 1]).ShouldBeGreaterThanOrEqualTo(FLOAT_PRECISION);

         //now switch float comparison off (use double) and rerun simulation
         sut.Options.UseFloatComparisonInUserOutputTimePoints = false;
         RunSimulation();

         //check that we have some points with deviation less than float precision
         solverTimes = sut.SimulationTimes;
         for (var i = 0; i < solverTimes.Length - 1; i++)
         {
            var timeDeviation = Math.Abs(solverTimes[i] - solverTimes[i + 1]);
            if (timeDeviation < FLOAT_PRECISION)
            {
               timeDeviation.ShouldBeGreaterThan(0.0); //still must be unique
               return;
            }
         }

         throw new Exception(
            $"Time values with deviation < {FLOAT_PRECISION} were expected with Double comparison but not found");
      }
   }

   public class when_getting_all_used_parameters_when_identify_used_parameters_set_to_true : concern_for_Simulation
   {
      protected override void OptionalTasksBeforeLoad()
      {
         sut.Options.IdentifyUsedParameters = true;
      }

      protected override void Because()
      {
         base.Because();
         LoadSimulation("TestExportUsedParameters02");
      }

      // test simulation constructed as following
      //---------------- Parameter -----------------
      //
      // P1 = P2
      // P2 = P3 + 2
      // P3 = y1*P4 + P5
      // P4 = P6 AND dP4/dt = P12 (Parameter has RHS)
      // P5 = 1
      // P6 = 1
      // P7 = 1
      // P8 = 1
      // P9 = 1
      // P10 = 1
      // P11 = 1
      // P12 = 1
      // P13 = P14
      // P14 = 1
      // P15 = y1
      // P16 = y1 (additionally parameter is defined as plotable)
      // --------------- Variables --------------------------
      // y1(0) = 0
      // y2(0) = P3
      // dy1/dt = -P7 - P8
      // dy2/dt = P7 + P8
      //--------------- Events --------------------
      // IF (y1 > 0) THEN P9 = P10
      // IF (P11 > 0) THEN y2 = y2 + 1
      //--------------- Observers -----------------
      // Obs1: P13*y1
      //--------------------------------------------------------
      //Expected parameters used:
      // P3 (used in start formula of y2)
      // P5 (used in P3)
      // P6 (used in P4) (because P4 has RHS it becomes ODE variable in SimModel-XML!)
      // P7 (used in RHS of y1, y2)
      // P8 (used in RHS of y1, y2)
      // P10 (used in Event assignment)
      // P11 (used in Event condition)
      // P12 (used in RHS of P4)
      // P13 (used in Observer)
      // P14 (used in P13)
      protected void checkPathsOfUsedParameters(IEnumerable<string> paths, bool includeSimulationName)
      {
         var prefix = "Organism|" + (includeSimulationName ? "TestExportUsedParameters02|" : "");

         var expectedPaths = new[]
         {
            prefix + "P3", prefix + "P5", prefix + "P6", prefix + "P7",
            prefix + "P8", prefix + "P10", prefix + "P11",
            prefix + "P12", prefix + "P13", prefix + "P14"
         };

         var usedPaths = paths.ToList();
         usedPaths.Count.ShouldBeEqualTo(expectedPaths.Length);

         foreach (var expectedPath in expectedPaths)
         {
            usedPaths.Contains(expectedPath).ShouldBeTrue($"{expectedPath} not found in the list of used parameters");
         }
      }

      [Observation]
      public void should_set_is_used_in_simulation_flag_only_for_effectively_used_parameters()
      {
         var usedParametersPath = from p in sut.ParameterProperties
            where p.IsUsedInSimulation
            select p.Path;

         checkPathsOfUsedParameters(usedParametersPath, false);
      }
   }

   public class when_getting_all_used_parameters_when_identify_used_parameters_set_to_false : concern_for_Simulation
   {
      protected override void OptionalTasksBeforeLoad()
      {
         sut.Options.IdentifyUsedParameters = false;
      }

      protected override void Because()
      {
         base.Because();
         LoadSimulation("TestExportUsedParameters02");
      }

      [Observation]
      public void should_set_is_used_in_simulation_flag_for_all_parameters()
      {
         foreach (var parameter in sut.ParameterProperties)
         {
            parameter.IsUsedInSimulation.ShouldBeTrue();
         }
      }
   }

   public class when_setting_scale_factor_to_one : concern_for_Simulation
   {
      private SpeciesProperties _a;

      protected override void OptionalTasksBeforeFinalize()
      {
         _a = sut.SpeciesProperties.First(v => v.EntityId.Equals("a"));
         sut.VariableSpecies = new[] {_a};
      }

      protected override void OptionalTasksBeforeRun()
      {
         _a.ScaleFactor = 1.0;
         sut.SetSpeciesValues();
      }

      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("Modified_ScaleFactors_opt");
      }

      [Observation]
      public void should_solve_the_system_correctly()
      {
         var values = sut.ValuesFor("a").Values;
         for (var i = 100; i < values.Length; i++)
         {
            values[i].ShouldBeSmallerThan(1e-15);
         }
      }
   }

   public abstract class when_running_pkmodelcore_case_study : concern_for_Simulation
   {
      protected double TotalODEVariablesAmountAt(int index)
      {
         var amount = 0.0;

         foreach (var variableValues in sut.AllValues.Where(v => v.VariableType == VariableValues.VariableTypes.Species))
         {
            amount += variableValues.IsConstant ? variableValues.Values[0] : variableValues.Values[index];
         }

         return amount;
      }

      protected static IEnumerable<string> TestData()
      {
         yield return "PKModelCoreCaseStudy_01";
         yield return "PKModelCoreCaseStudy_02";
      }

      protected void CheckMassBalance(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName, performBasicTests: true);
         TotalODEVariablesAmountAt(0).ShouldBeEqualTo(26.5, 1e-10);
         TotalODEVariablesAmountAt(sut.GetNumberOfTimePoints - 1).ShouldBeEqualTo(36.5, 1e-10);
      }

      protected void CheckObserver(string shortFileName)
      {
         LoadFinalizeAndRunSimulation(shortFileName, performBasicTests: true);

         int noOfOutputTimePoints = sut.GetNumberOfTimePoints;

         var Art_pls_A = sut.ValuesFor(3);
         var Art_pls_A_Obs1 = sut.ValuesFor(7);

         Art_pls_A.Values.Length.ShouldBeEqualTo(noOfOutputTimePoints);
         Art_pls_A_Obs1.Values.Length.ShouldBeEqualTo(noOfOutputTimePoints);

         for (var i = 0; i < noOfOutputTimePoints; i++)
         {
            Art_pls_A_Obs1.Values[i].ShouldBeEqualTo(Art_pls_A.Values[i] / 2.0, 1e-5);
         }
      }
   }

   public class when_running_pkmodelcore_case_study_without_scale_factor : when_running_pkmodelcore_case_study
   {
      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void mass_balance_at_t0_and_at_tEnd_should_be_ok(string shortFileName)
      {
         CheckMassBalance(shortFileName);
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void amount_observer_for_arterial_blood_plasma_A_should_return_correct_values(string shortFileName)
      {
         CheckObserver(shortFileName);
      }
   }

   public class when_running_pkmodelcore_case_study_with_scale_factor : when_running_pkmodelcore_case_study
   {
      protected override void OptionalTasksBeforeFinalize()
      {
         var allSpecies = sut.SpeciesProperties.ToList();
         var variableSpecies = new[]
         {
            GetSpeciesByPath(allSpecies, "Organism/ArterialBlood/Plasma/A")
         };

         variableSpecies[0].ScaleFactor = 10.0;
         sut.VariableSpecies = variableSpecies;
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void mass_balance_at_t0_and_at_tEnd_should_be_ok(string shortFileName)
      {
         CheckMassBalance(shortFileName);
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void amount_observer_for_arterial_blood_plasma_A_should_return_correct_values(string shortFileName)
      {
         CheckObserver(shortFileName);
      }
   }

   public class when_running_below_absolute_tolerance_test : concern_for_Simulation
   {
      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("PKSim_Input_BelowAbsTol");
      }

      [Observation]
      public void all_variable_values_below_absolute_tolerance_should_be_zero()
      {
         var absTol = sut.RunStatistics.UsedAbsoluteTolerance;

         foreach (var variableValues in sut.AllValues.Where(v=>v.VariableType==VariableValues.VariableTypes.Species))
         {
            var values = variableValues.Values;

            foreach (var value in values)
            {
               if ((value < 0.0) && (value > -absTol))
                  throw new Exception($"Species {variableValues.Path} has values in [-AbsTol..AbsTol]");
            }
         }
      }
   }

   public class when_running_simulation_with_persistable_parameters : concern_for_Simulation
   {
      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("PersistableParams");
      }

      [Observation]
      public void should_perform_simulation_run_and_return_persistable_parameters_as_observers()
      {
         var observers = sut.AllValues.Where(v => v.VariableType == VariableValues.VariableTypes.Observer).ToArray();

         observers.Length.ShouldBeEqualTo(2);

         var timeParamObserver = observers[0];
         var inverseTimeParamObserver = observers[1];

         timeParamObserver.IsConstant.ShouldBeFalse();
         inverseTimeParamObserver.IsConstant.ShouldBeFalse();

         timeParamObserver.EntityId.ShouldBeEqualTo("74fe8982-69cd-41be-9dc6-7fd8020f2ed4");

         var solverTimes = sut.SimulationTimes;
         var obsValues = timeParamObserver.Values;
         var inverseObsValues = inverseTimeParamObserver.Values;

         for (var i = 0; i < sut.GetNumberOfTimePoints; i++)
         {
            obsValues[i].ShouldBeEqualTo(solverTimes[i], 1e-5);
            inverseObsValues[i].ShouldBeEqualTo(i==0 ? Double.PositiveInfinity : 1.0 / solverTimes[i], 1e-5);
         }
      }
   }

   public class when_changing_ehc_start_time : concern_for_Simulation
   {
      protected ParameterProperties _startTimeParameter;

      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("POP_EHC_StartTime");
      }

      protected override void OptionalTasksBeforeFinalize()
      {
         var parameterProperties = sut.ParameterProperties;
         _startTimeParameter = parameterProperties.First(p => p.Path.Equals("Events|EHC|EHC_1|Start time"));

         sut.VariableParameters = new[] { _startTimeParameter };
      }

      protected double SumVenPls => sut.ValuesFor(121).Values.Sum();

      [Observation]
      public void changing_ehc_start_time_should_alter_simulation_results()
      {
         var value1 = SumVenPls;

         _startTimeParameter.Value=1000000; //EHC beyond simulation time
         sut.SetParameterValues();
         sut.RunSimulation();

         var value2 = SumVenPls;

         //Difference should be > 5%
         var diff = Math.Abs(value1 - value2) / Math.Min(value1, value2);
         diff.ShouldBeGreaterThan(0.05);
      }
   }

   public class when_calculating_comparison_threshold : concern_for_Simulation
   {
      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("ConstantObservers");
      }

      [Observation]
      public void should_calculate_comparison_threshold_of_const_observers()
      {
         //get absolute tolerance used for calculation (might differ from input absolute tolerance)
         double AbsTol = sut.RunStatistics.UsedAbsoluteTolerance;

         //expected threshold for ode variables
         double threshold = 10.0 * AbsTol;

         foreach (var values in sut.AllValues)
         {
            if (!values.IsConstant || values.VariableType != VariableValues.VariableTypes.Observer)
               continue;

            values.ComparisonThreshold.ShouldBeEqualTo(threshold);
         }
      }
   }

   public class when_retrieving_solver_warnings : concern_for_Simulation
   {
      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("TestWarnings_LowMxStep");
      }

      protected override void OptionalTasksBeforeRun()
      {
         sut.Options.StopOnWarnings = false;
      }

      [Observation]
      public void should_retrieve_solver_warnings()
      {
         var warnings = sut.SolverWarnings;
         warnings.Count().ShouldBeGreaterThan(0);
      }
   }

   public class when_exporting_model_to_cpp_code : concern_for_Simulation
   {
      protected override void OptionalTasksBeforeLoad()
      {
         sut.Options.KeepXMLNodeAsString = true;
      }

      [Observation]
      public void should_export_to_cpp()
      {
         var exportFolder = CreateTempFolder();

         LoadSimulation("CPPExportTest01");
         sut.ExportToCode(exportFolder, CodeExportLanguage.Cpp, CodeExportMode.Formula, "CPPExportTest01_Formula");
         File.Exists(Path.Combine(exportFolder, "CPPExportTest01_Formula.cpp")).ShouldBeTrue();

         LoadSimulation("CPPExportTest01");
         sut.ExportToCode(exportFolder, CodeExportLanguage.Cpp, CodeExportMode.Values, "CPPExportTest01_Values");
         File.Exists(Path.Combine(exportFolder, "CPPExportTest01_Values.cpp")).ShouldBeTrue();
      }

   }

   public class when_running_a_simulation_with_const_zero_table_formulation : concern_for_Simulation
   {
      protected override void Because()
      {
         base.Because();
         LoadAndFinalizeSimulation("ConstZeroTableFormulation");
      }

      [Observation]
      public void simulation_run_should_not_crash()
      {
         sut.RunSimulation();
      }
   }

   public class when_running_a_simulation_with_negative_molecule_initial_value : concern_for_Simulation
   {
      protected override void Because()
      {
         base.Because();
         LoadAndFinalizeSimulation("NegativeInititalValue");
      }

      [Observation]
      public void simulation_should_throw_an_exception()
      {
         sut.Options.CheckForNegativeValues = true;
         try
         {
            RunSimulation();
         }
         catch (Exception ex)
         {
            ex.Message.Contains("Initial value").ShouldBeTrue();
            ex.Message.Contains("negative").ShouldBeTrue();

            //expected behavior. Leave the test case
            return;
         }

         //failed (no exception with negative values check)
         throw new Exception("No exception was thrown with negative values check");
      }

      [Observation]
      public void simulation_should_run_if_negative_values_are_globally_allowed()
      {
         sut.Options.CheckForNegativeValues = false;
         RunSimulation();
      }
   }

   public class when_having_infs_or_nans_in_the_formulas_which_are_not_simplified : concern_for_Simulation
   {
      // In the test project, parameter P0 is defined as time dependent
      // Observers "Inf_i" are defined as "P0*<infinity_string>" (<infinity_string> one of {"Inf","inf","Infinity","infinity"})
      // Observers "Neginf_i" are defined as "P0*(-<infinity_string>)" (<infinity_string> one of {"Inf","inf","Infinity","infinity"})
      // Observers "Nan_i" are defined as "P0*<nan_string>" (<nan_string> one of {"NaN","NAN","nan"})
      protected static IEnumerable<object[]> TestData()
      {
         yield return new object[] { "Inf_1", double.PositiveInfinity };
         yield return new object[] { "Inf_2", double.PositiveInfinity };
         yield return new object[] { "Inf_3", double.PositiveInfinity };
         yield return new object[] { "Inf_4", double.PositiveInfinity };

         yield return new object[] { "Neginf_1", double.NegativeInfinity };
         yield return new object[] { "Neginf_2", double.NegativeInfinity };
         yield return new object[] { "Neginf_3", double.NegativeInfinity };
         yield return new object[] { "Neginf_4", double.NegativeInfinity };

         yield return new object[] { "NaN_1", double.NaN };
         yield return new object[] { "NaN_2", double.NaN };
         yield return new object[] { "NaN_3", double.NaN };
      }

      private IEnumerable<VariableValues> _results;

      protected override void Because()
      {
         base.Because();
         LoadFinalizeAndRunSimulation("NaN_Inf_NegInf");
         _results = sut.AllValues;
      }

      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void observer_should_return_correct_value(string observerName, double expectedValue)
      {
         _results.First(r=>r.Name.Equals(observerName)).Values.Last().ShouldBeEqualTo(expectedValue);
      }
   }

   public class when_nans_appear_in_conditional_if_min_max_formulas : concern_for_Simulation
   {
      //for the definition of parameters s. the test project tests\TestData\Nan_If_Conditional_Min_Max.mbp3 
      private IEnumerable<ParameterProperties> _nan_parameters;  //parameters which value must be NaN
      private IEnumerable<ParameterProperties> _one_parameters;  //parameters which value must be 1
      private IEnumerable<ParameterProperties> _zero_parameters; //parameters which value must be 0

      protected override void Because()
      {
         base.Because();
         LoadAndFinalizeSimulation("Nan_If_Conditional_Min_Max");
         
         var namePrefix = "Nan_If_Conditional_Min_Max|Organism|";

         _nan_parameters = sut.ParameterProperties.Where(p=>p.Name.StartsWith($"{namePrefix}P"));
         _one_parameters = sut.ParameterProperties.Where(p => p.Name.StartsWith($"{namePrefix}T"));
         _zero_parameters = sut.ParameterProperties.Where(p => p.Name.StartsWith($"{namePrefix}F"));
      }

      [Observation]
      public void all_nan_parameters_should_be_nan()
      {
         _nan_parameters.Count().ShouldBeEqualTo(19);
         _nan_parameters.Each(p => p.Value.ShouldBeEqualTo(double.NaN, $"{p.Name} was not NaN"));
      }

      [Observation]
      public void all_one_parameters_should_be_one()
      {
         _one_parameters.Count().ShouldBeEqualTo(21);
         _one_parameters.Each(p=>p.Value.ShouldBeEqualTo(1, $"{p.Name} was not 1")); 
      }

      [Observation]
      public void all_zero_parameters_should_be_zero()
      {
         _zero_parameters.Count().ShouldBeEqualTo(11);
         _zero_parameters.Each(p => p.Value.ShouldBeEqualTo(0, $"{p.Name} was not 0"));
      }
   }

   public class when_running_system_with_output_values_below_the_threshold : concern_for_Simulation
   {
      protected override void Because()
      {
         LoadFinalizeAndRunSimulation("RemoveBelowAbsTolTest");
      }

      [Ignore("Enable when implementing OSPSuite.SimModel/issues/162 and MoBi/issues/1741")]
      [Observation]
      public void should_set_all_values_below_absolute_tolerance_to_zero()
      {
         var absTol = sut.RunStatistics.UsedAbsoluteTolerance;
         var speciesList = sut.AllValues.Where(values => values.VariableType == VariableValues.VariableTypes.Species)
            .ToList();
         speciesList.Count.ShouldBeGreaterThan(0);

         foreach (var species in speciesList)
         {
            species.Values.Length.ShouldBeGreaterThan(0);
            species.Values.Where(value=>value!=0).Each(value => Math.Abs(value).ShouldBeGreaterThanOrEqualTo(absTol));
         }
      }

   }

   public class when_calculating_table_formula_with_x_argument_defined_by_parameter_with_rhs : concern_for_Simulation
   {
      protected override void Because()
      {
         LoadFinalizeAndRunSimulation("TableFormulaWithXArgumentRHS");
      }

      [Observation]
      public void parameter_defined_by_table_formula_with_xargument_should_increase()
      {
         var volume2 = sut.AllValues.First(v => v.Name.Equals("Volume2"));
         volume2.Values.Last().ShouldBeGreaterThan(0);
      }

   }
}