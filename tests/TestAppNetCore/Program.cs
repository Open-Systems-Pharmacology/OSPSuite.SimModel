using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Transactions;
using OSPSuite.SimModel;

namespace TestAppNetCore
{
   class Program
   {
      static void Main(string[] args)
      {
         try
         {
            Test3();
         }
         catch (Exception e)
         {
            Console.WriteLine(e);
         }

         Console.WriteLine("Press Enter");
         Console.ReadLine();
      }

      static void Test3()
      {
         var testClass = new Testsystem_06_with_scale_factor_dense();
         var values = testClass.GetValues("SimModel4_ExampleInput06");
      }

      class Testsystem_06_with_scale_factor_dense : SimulationTestsBase
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

         public VariableValues[] GetValues(string shortFileName)
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
                  Console.WriteLine($"Observer {values.Path}.Threshold = {values.ComparisonThreshold}. Expected: {2.0* threshold * _scaleFactor}");
               }
               else
               {
                  //for non-const variables, Threshold must be equal to DefaultThreshold*ScaleFactor
                  var variableThreshold = values.IsConstant ? threshold : threshold * _scaleFactor;
                  Console.WriteLine($"Species {values.Path}.Threshold = {values.ComparisonThreshold}. Expected: {variableThreshold}");
               }
            }

            return sut.AllValues.ToArray();
         }
      }

      abstract class SimulationTestsBase
      {
         protected Simulation sut;

         protected SimulationTestsBase()
         {
            Context();
         }

         protected void Context()
         {
            sut = new Simulation();
            //         sut.Options.KeepXMLNodeAsString = true;
         }

         protected void Because(){ }

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

         protected SpeciesProperties GetSpeciesByPath(IEnumerable<SpeciesProperties> speciesProperties, string path)
         {
            return speciesProperties.FirstOrDefault(p => p.Path.Equals(path));
         }

      }

      static void Test2()
      {
         var sim = new Simulation();
         sim.LoadFromXMLFile(TestFileFrom("TestAllParametersInitialValues"));

         var allParameters = sim.ParameterProperties.ToList();

         //---- set P1 and P2 as variable
         var variableParameters = new ParameterProperties[]
         {
            GetParameterByPath(allParameters, "P1"),
            GetParameterByPath(allParameters, "P2")
         };

         sim.VariableParameters = variableParameters;

         sim.FinalizeSimulation();

         //value of P10 should be equal P1+P2, which is initially 1
         Console.WriteLine($"P10={GetParameterByPath(sim.ParameterProperties, "P10").Value}");

         //update variable parameters
         variableParameters = sim.VariableParameters.ToArray();
         GetParameterByPath(variableParameters, "P1").Value = 3;
         GetParameterByPath(variableParameters, "P2").Value = 4;

         //---- set new parameter values
         sim.SetParameterValues();

         //value of P10 should be equal P1+P2, which is now 7
         Console.WriteLine($"P10={GetParameterByPath(sim.ParameterProperties, "P10").Value}");
      }

      static ParameterProperties GetParameterByPath(IEnumerable<ParameterProperties> parameterProperties, string path)
      {
         return parameterProperties.FirstOrDefault(p => p.Path.Equals(path));
      }


      static void Test1()
      {
         var sim = new Simulation();
         sim.LoadFromXMLFile(TestFileFrom("SimModel4_ExampleInput06"));
         sim.FinalizeSimulation();
         sim.RunSimulation();

         var y3 = sim.ValuesFor("y3");
         bool isConst = y3.IsConstant;

         Console.WriteLine($"y3.IsConstant={isConst}");
      }

      static string TestFileFrom(string shortFilename)
      {
         var location = Path.GetDirectoryName(Assembly.GetCallingAssembly().Location);
         var file = Path.Combine(location, @"..\..\..\..\TestData", shortFilename);

         if (!file.ToLower().EndsWith(".xml"))
            file += ".xml";

         return file;
      }
   }
}
