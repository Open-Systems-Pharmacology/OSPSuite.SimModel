#include "TestAppCpp.h"
#include <iostream>
#include <string.h>

void evalPInvokeErrorMsg(const bool success, char* errorMessage)
{
   if (success)
   {
      if (errorMessage != NULL)
         CoTaskMemFree(errorMessage);
      return;
   }

   string error = errorMessage;
   CoTaskMemFree(errorMessage);

   throw(error);
}

Simulation* LoadSimulation(const string& fileName, bool keepXmlString)
{
   bool success;
   char *errorMsg = NULL;

   Simulation* sim = CreateSimulation();

   SimulationOptionsStructure options;
   FillSimulationOptions(sim, &options);
   options.AutoReduceTolerances = false;
   options.KeepXMLNodeAsString = keepXmlString;
   SetSimulationOptions(sim, options);

   double t1, t2;
   cout << "loading " << fileName.c_str() << " ... "; 	fflush(stdout);
   
   t1 = GetTickCount();
   LoadSimulationFromXMLFile(sim, TestFileFrom(fileName).c_str(), success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   t2 = GetTickCount();
   
   ShowTimeSpan(t1, t2);

   return sim;
}

void FinalizeSimulation(Simulation* sim)
{
   bool success;
   char* errorMsg = NULL;

   double t1, t2;
   cout << "finalizing " << " ... "; 	fflush(stdout);

   t1 = GetTickCount();
   FinalizeSimulation(sim, success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   t2 = GetTickCount();

   ShowTimeSpan(t1, t2);
}

void RunSimulation(Simulation* sim, bool showInfo)
{
   bool success;
   char* errorMsg = NULL;
   bool toleranceWasReduced;
   double newAbsTol, newRelTol;

   double t1, t2;
   if (showInfo)
   {
      cout << "running ... ";
      fflush(stdout);
   }

   t1 = GetTickCount();
   RunSimulation(sim, toleranceWasReduced, newAbsTol, newRelTol, success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   t2 = GetTickCount();

   if (showInfo)
      ShowTimeSpan(t1, t2);

   ShowFirstWarning(sim);
}

void ShowFirstWarning(Simulation* sim)
{
   bool success;
   char* errorMsg = NULL;
   double* times = NULL;
   char** warnings = NULL;

   auto noOfWarnings = GetNumberOfSolverWarnings(sim);
   if (noOfWarnings == 0)
      return;

   times = new double[noOfWarnings];
   warnings = new char* [noOfWarnings];

   FillSolverWarnings(sim, noOfWarnings, times, warnings, success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);

   cout << noOfWarnings << "solver warnings. 1st warning: " << warnings[0] << endl;

   delete[] times;
   for (auto i = 0; i < noOfWarnings; i++)
      CoTaskMemFree(warnings[i]);

   delete[] warnings;
}

vector<ParameterInfo>* GetParameterProperties(Simulation* sim)
{
   bool success;
   char* errorMsg = NULL;

   vector<ParameterInfo>* parameterInfos = CreateParameterInfoVector();
   FillParameterProperties(sim, parameterInfos, success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);

   return parameterInfos;
}

void SetVariableParameters(Simulation* sim, vector<ParameterInfo>* parameterProperties, vector<int> variableParameterIndices)
{
   bool success;
   char* errorMsg = NULL;
   int* indices = NULL;

   try
   {
      auto noOfVariableParameters = (int)variableParameterIndices.size();
      if (noOfVariableParameters == 0)
         return;

      indices = new int[noOfVariableParameters];
      for (auto i = 0; i < noOfVariableParameters; i++)
         indices[i] = variableParameterIndices[i];

      SetVariableParameters(sim, parameterProperties, indices, noOfVariableParameters, success, &errorMsg);
      evalPInvokeErrorMsg(success, errorMsg);

      delete[] indices;
   }
   catch (...)
   {
      if (indices != NULL)
         delete[] indices;
      throw;
   }
}

void SetParameterValues(Simulation* sim, vector<ParameterInfo>* parameterProperties, vector<int> variableParameterIndices)
{
   bool success;
   char* errorMsg = NULL;
   int* indices = NULL;

   try
   {
      auto noOfVariableParameters = (int)variableParameterIndices.size();
      if (noOfVariableParameters == 0)
         return;

      indices = new int[noOfVariableParameters];
      for (auto i = 0; i < noOfVariableParameters; i++)
         indices[i] = variableParameterIndices[i];

      SetParameterValues(sim, parameterProperties, indices, noOfVariableParameters, success, &errorMsg);
      evalPInvokeErrorMsg(success, errorMsg);

      delete[] indices;
   }
   catch (...)
   {
      if (indices != NULL)
         delete[] indices;
      throw;
   }
}

void ShowErrorMessage(const string & msg)
{
	cout<<msg.c_str()<<endl;
}

void ShowErrorMessage(const ErrorData & ED)
{
	cout<<ED.GetDescription()<<endl<<"    in "<<ED.GetSource()<<endl;
}

void ShowTimeSpan(double tstart, double tend)
{
	cout<<(tend-tstart)/1000.0<<"s"<<endl;
	fflush(stdout);
}

string BasisDir(const string& exeName)
{
	//TODO
	return "C:\\SW-Dev\\SimModel\\branches\\4.0\\tests\\TestAppCpp\\x64\\Debug\\";
}

string TestFileFrom(const string& fileName)
{
   //TODO
   return BasisDir("") + "..\\..\\..\\TestData\\" + fileName + ".xml";
}

void TestLeaks(void)
{
   double d = 0;
   auto* xxx = new char[10];
   strcpy_s(xxx,10, "Fat cat");
//   auto* yyy = CoTaskMemAlloc(100);
}

void ClearDynamicLibrary()
{
//   auto * factory = DynamicLibraryFactory::GetFactory();
//   factory->Clear();
}
