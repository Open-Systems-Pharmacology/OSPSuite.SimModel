using System;
using System.Collections.Generic;
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
            Test2();
         }
         catch (Exception e)
         {
            Console.WriteLine(e);
         }

         Console.WriteLine("Press Enter");
         Console.ReadLine();
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
