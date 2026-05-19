#include "TestAppCpp.h"
#include <thread>
//#include <vld.h>
#include <windows.h>
#include <ppl.h>

using namespace concurrency;

int main(int argc, char** argv)
{

   try
   {
      CoInitialize(NULL); 

      //TestLeaks();
      //TestSetTablePoints();

      string simName;
      simName = "Vanessa_sim2";
//      simName = "CPPExportTest01";
//      simName = "Test_dynamic_reduced_3"; 
//      simName = "SimModel4_ExampleInput06_Modified";
      //simName = "SolverError01";

      //TestCPPExport(simName);
      //Test1(simName);

      vector<string> variableParameterPaths = {
          "Neighborhoods|Kidney_pls_Kidney_ur|FLU-benzamide|Renal Clearances-Default Renal Cl|Specific clearance",
          "FLU to FLU-benzamide-CYP3A4-Default CYP|kcat",
          "FLU-benzamide|Fraction unbound (plasma, reference value)",
          "Fluopyram|Fraction unbound (plasma, reference value)",
          "FLU-benzamide|Lipophilicity",
          "Fluopyram|Lipophilicity",
          "FLU to FLU-benzamide-CYP3A4-Default CYP|Km"
      };

      vector<double> valuesToUse = {
          0.1,
          50,
          0.08,   
          0.298168334380156,
          0.4,
          3.6,
          10
      };

      vector<int> variableParameterIndices = {
        10174,
        13238,
        12584,
        12433,
        12583,
        12432,
        13237
      };

      Test1(simName);
      vector<bool> calculateSensitivities = { true, false, false, false, false, false, false };
      TestSensitivities(simName, variableParameterPaths, valuesToUse, variableParameterIndices, calculateSensitivities);
      //TestParallel1(argc, argv);
   }
   catch (ErrorData& ED)
   {
      ShowErrorMessage(ED);
   }
   catch (const string& str)
   {
      ShowErrorMessage(str);
   }
   catch (SimModelSolverErrorData sED)
   {
      ShowErrorMessage(ErrorData(ErrorData::errNumber(sED.GetNumber()), sED.GetSource(), sED.GetDescription()));
   }
   catch (...)
   {
      ShowErrorMessage("Unknown Error");
   }

   ClearDynamicLibrary();
   cout << "Enter anything";
   _getch();

   return 0;
}

void TestParallel1(int argc, char** argv)
{
   string* simXMLStrings = NULL;
   try
   {
      string runMode = "threads";

      if (argc < 3)
         throw "Missing arguments";
      
      int numberOfThreads = atoi(argv[2]);
      if (numberOfThreads <= 0)
         throw string("Number of threads must be >0. Passed value:") + argv[1];

      if (argc > 3)
      {
         runMode = argv[3];
      }

      string filePath = argv[1];
      auto simulationString = readFileIntoString(filePath);

      cout << "Running '" << filePath.c_str() << "' (" << numberOfThreads << " runs)" << endl;

      simXMLStrings = new string[numberOfThreads];
      for (auto i = 0; i < numberOfThreads; i++)
         simXMLStrings[i] = simulationString;

      auto t1 = GetTickCount64();

      if (runMode == "threads")
      {
         ParallelLoop_Threads(numberOfThreads, simXMLStrings);
      }
      else if (runMode == "omp")
      {
         ParallelLoop_Omp(numberOfThreads, simXMLStrings);
      }
      else if (runMode == "sequential")
      {
         ParallelLoop_Sequential(numberOfThreads, simXMLStrings);
      }
      else if (runMode == "parallel_for")
      {
         ParallelLoop_ParallelFor(numberOfThreads, simXMLStrings);
      }
      else
         throw "Invalid run mode passed (" + runMode + "). Valid values are: threads, omp, sequential, parallel_for";

      auto t2 = GetTickCount64();
      cout << "Overall time "; 	fflush(stdout);
      ShowTimeSpan(t1, t2);
   }
   catch (...)
   {
      if (simXMLStrings != NULL)
         delete[] simXMLStrings;
      throw;
   }
}

void ParallelLoop_Threads(int numberOfThreads, string* simXMLStrings)
{
   cout << "Parallel loop: Threads" << endl;

   vector <thread> threads;
   for (auto i = 0; i < numberOfThreads; i++)
   {
      threads.push_back(thread{ SingleSimulationRunForParallel, ref(simXMLStrings[i]) });
   }
   for (auto& singleRunThread : threads)
   {
      singleRunThread.join();
   }
}

//to run this, enable openMP support under C++/Language in VS and add additional compiler option /Zc:twoPhase- 
void ParallelLoop_Omp(int numberOfThreads, string* simXMLStrings)
{
   cout << "Parallel loop: OMP" << endl;

#pragma omp parallel for
   for (auto i = 0; i < numberOfThreads; i++)
   {
      SingleSimulationRunForParallel(simXMLStrings[i]);
   }
}

void ParallelLoop_ParallelFor(int numberOfThreads, string* simXMLStrings)
{
   cout << "Parallel loop: Parallel_For" << endl;

   parallel_for(size_t(0), (size_t)numberOfThreads, [&](size_t i)
   {
      SingleSimulationRunForParallel(simXMLStrings[i]);
   });
}

void ParallelLoop_Sequential(int numberOfThreads, string* simXMLStrings)
{
   cout << "Parallel loop: Sequential" << endl;

   for (auto i = 0; i < numberOfThreads; i++)
   {
      SingleSimulationRunForParallel(simXMLStrings[i]);
   }
}

void SingleSimulationRunForParallel(const string& simXMLString)
{
   Simulation* simulation = NULL;
   try
   {
      CoInitialize(NULL);

      simulation = LoadSimulationFromString(simXMLString);
      FinalizeSimulation(simulation);
      RunSimulation(simulation);

      DisposeSimulation(simulation);
      simulation = NULL;
   }
   catch (...)
   {
      if (simulation != NULL)
         DisposeSimulation(simulation);
      throw;
   }
}

void TestSetTablePoints()
{
   bool success;
   char* errorMsg = NULL;

   const string simName = "SimModel4_ExampleInput06_Modified";

   Simulation* sim = NULL;
   vector<ParameterInfo>* parameterInfos = NULL;

   try
   {
      sim = LoadSimulation(simName);

      parameterInfos = GetParameterProperties(sim);
      vector<int> variableParameterIndices;

      for (auto i = 0; i < parameterInfos->size(); i++)
      {
         if ((*parameterInfos)[i].GetFullName() == "TopContainer/Subcontainer1/P5")
         {
            variableParameterIndices.push_back(i);
            break;
         }
      }
      if (variableParameterIndices.size() == 0)
         throw "TopContainer/Subcontainer1/P5 not found";

      SetVariableParameters(sim, parameterInfos, variableParameterIndices);

      FinalizeSimulation(sim);
      RunSimulation(sim);

      double xValues[3] = { 1,2,3 };
      double yValues[3] = { 4,5,6 };
      bool restart[3] = { false,false,false };
      SetParameterTablePoints(parameterInfos, variableParameterIndices[0], xValues, yValues, restart,3, success, &errorMsg);
      evalPInvokeErrorMsg(success, errorMsg);

      SetParameterValues(sim, parameterInfos, variableParameterIndices);
      RunSimulation(sim);

      double xValues2[10] = {0, 1,2,3,4,5,6,7,8,9 };
      double yValues2[10] = { 4,5,6,6,6,6,6,6,6,6 };
      bool restart2[10] = { false,false,false,false,false,false,false,false,false,false };
      SetParameterTablePoints(parameterInfos, variableParameterIndices[0], xValues2, yValues2, restart2, 10, success, &errorMsg);
      evalPInvokeErrorMsg(success, errorMsg);

      SetParameterValues(sim, parameterInfos, variableParameterIndices);
      RunSimulation(sim);

      SetParameterTablePoints(parameterInfos, variableParameterIndices[0], NULL, NULL, NULL, 0, success, &errorMsg);
      evalPInvokeErrorMsg(success, errorMsg);

      SetParameterValues(sim, parameterInfos, variableParameterIndices);
      RunSimulation(sim);

      DisposeParameterInfoVector(parameterInfos);
      parameterInfos = NULL;

      DisposeSimulation(sim);
      sim = NULL;
   }
   catch (...)
   {
      if (sim != NULL)
         DisposeSimulation(sim);
      if (parameterInfos != NULL)
         DisposeParameterInfoVector(parameterInfos);
      throw;
   }
}

void TestSensitivities(
    const string& simName, 
    const vector<string> variableParameterPaths, 
    const vector<double> variableParameterValues,
    vector<int> variableParameterIndices,
    const vector<bool> calculateSensitivities) {
    Simulation* sim = NULL;
    try
    {
        sim = LoadSimulation(simName);
        vector<ParameterInfo>* parameterInfos = NULL;
        parameterInfos = GetParameterProperties(sim);
        for (auto i = 0; i < variableParameterPaths.size(); i++)
        {
            (*parameterInfos)[variableParameterIndices[i]].SetValue(variableParameterValues[i]);
            (*parameterInfos)[variableParameterIndices[i]].SetCalculateSensitivity(calculateSensitivities[i]);
        }
        SetVariableParameters(sim, parameterInfos, variableParameterIndices);

        FinalizeSimulation(sim);
        RunSimulation(sim);

        //double* sensitivities = SensitivityValuesByPathsFor(sim, "Organism|Egg 1|DefaultCompound|Whole Egg", "Neighborhoods|Kidney_pls_Kidney_ur|FLU-benzamide|Renal Clearances-Default Renal Cl|Specific clearance");

        DisposeSimulation(sim);
        //delete sensitivities;
        sim = NULL;
    }
    catch (...)
    {
        if (sim != NULL)
            DisposeSimulation(sim);
        throw;
    }
}

void Test1(const string& simName)
{
   Simulation* sim = NULL;
   try
   {
      sim = LoadSimulation(simName);
      FinalizeSimulation(sim);
      RunSimulation(sim);

      DisposeSimulation(sim);
      sim = NULL;
   }
   catch (...)
   {
      if (sim != NULL)
         DisposeSimulation(sim);
      throw;
   }
}

void TestCPPExport(const string& simName)
{
   Simulation* sim = NULL;
   auto cppExporter = new CppODEExporter();

   try
   {
      //sim = LoadSimulation(simName, true);
      //cppExporter->WriteCppCode(sim, "C:\\SW-Dev\\SimModel\\CPPExportTest01", true);
      //DisposeSimulation(sim);

      sim = LoadSimulation(simName, true);
      cppExporter->WriteCppCode(sim, ".\\", false);
      DisposeSimulation(sim);

      sim = NULL;
   }
   catch (...)
   {
      if (sim != NULL)
         DisposeSimulation(sim);
      throw;
   }
}