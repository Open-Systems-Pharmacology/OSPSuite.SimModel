using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml.Linq;
using NUnit.Framework;
using OSPSuite.BDDHelper;

namespace OSPSuite.SimModel.Tests
{
   public abstract class concern_for_Simulation : ContextSpecification<Simulation>
   {
      protected override void Context()
      {
         sut = new Simulation
         {
            KeepXMLNodeAsString = true
         };
      }

      protected string TestFileFrom(string shortFilename)
      {
         var location = Path.GetDirectoryName(Assembly.GetCallingAssembly().Location);
         var file= Path.Combine(location, @"..\..\..\..\TestData", shortFilename);

         if (!file.ToLower().EndsWith(".xml"))
            file += ".xml";

         return file;
      }
   }

   public class when_loading_simulation_from_file_and_running : concern_for_Simulation
   {
      [Observation]
      [TestCaseSource(nameof(TestData))]
      public void should_load_simulation(string shortFileName)
      {
         sut.LoadFromXMLFile(TestFileFrom(shortFileName));
         sut.FinalizeSimulation();
         sut.RunSimulation();
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