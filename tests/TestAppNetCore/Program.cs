using System;
using System.IO;
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
            Test1();
         }
         catch (Exception e)
         {
            Console.WriteLine(e);
         }

         Console.WriteLine("Press Enter");
         Console.ReadLine();
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
