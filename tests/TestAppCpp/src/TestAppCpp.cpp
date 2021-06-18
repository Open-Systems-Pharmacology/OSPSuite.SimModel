#include "TestAppCpp.h"
//#include <vld.h>

int main()
{

   try
   {
      CoInitialize(NULL); 

      //TestLeaks();
      //TestSetTablePoints();

      string simName;
      simName = "Test_dynamic_reduced_3"; 
//      simName = "SimModel4_ExampleInput06_Modified";
      //simName = "SolverError01";

      Test1(simName);
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

void TestSetTablePoints()
{
   bool success;
   char* errorMsg = NULL;

   const string simName = "SimModel4_ExampleInput06_Modified2";//"SimModel4_ExampleInput06_Modified";

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