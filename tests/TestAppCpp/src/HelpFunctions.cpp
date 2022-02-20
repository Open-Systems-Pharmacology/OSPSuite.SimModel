#include "TestAppCpp.h"
#include <iostream>
#include <string.h>
#include <codecvt>

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

   SimulationOptionsStructure options{};
   FillSimulationOptions(sim, &options);
   options.AutoReduceTolerances = false;
   options.KeepXMLNodeAsString = keepXmlString;
   SetSimulationOptions(sim, options);

   cout << "loading " << fileName.c_str() << " ... "; 	fflush(stdout);
   
   auto t1 = GetTickCount64();
   LoadSimulationFromXMLFile(sim, TestFileFrom(fileName).c_str(), success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   auto t2 = GetTickCount64();
   
   ShowTimeSpan(t1, t2);

   return sim;
}

Simulation* LoadSimulationFromString(const string& simulationString, bool keepXmlString)
{
   bool success;
   char* errorMsg = NULL;

   Simulation* sim = CreateSimulation();

   SimulationOptionsStructure options{};
   FillSimulationOptions(sim, &options);
   options.AutoReduceTolerances = false;
   options.KeepXMLNodeAsString = keepXmlString;
   SetSimulationOptions(sim, options);

//   cout << "loading from string ... "; 	fflush(stdout);

   auto t1 = GetTickCount64();
   LoadSimulationFromXMLString(sim, simulationString.c_str(), success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   auto t2 = GetTickCount64();

   cout << "loading from string ... "; 	fflush(stdout);
   ShowTimeSpan(t1, t2);

   return sim;
}

void FinalizeSimulation(Simulation* sim)
{
   bool success;
   char* errorMsg = NULL;

//   cout << "finalizing " << " ... "; 	fflush(stdout);

   auto t1 = GetTickCount64();
   FinalizeSimulation(sim, success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   auto t2 = GetTickCount64();

   cout << "finalizing " << " ... "; 	fflush(stdout);
   ShowTimeSpan(t1, t2);
}

void RunSimulation(Simulation* sim, bool showInfo)
{
   bool success;
   char* errorMsg = NULL;
   bool toleranceWasReduced;
   double newAbsTol, newRelTol;

   //if (showInfo)
   //{
   //   cout << "running ... ";
   //   fflush(stdout);
   //}

   auto t1 = GetTickCount64();
   RunSimulation(sim, toleranceWasReduced, newAbsTol, newRelTol, success, &errorMsg);
   evalPInvokeErrorMsg(success, errorMsg);
   auto t2 = GetTickCount64();

   if (showInfo)
   {
      cout << "running ... ";
      ShowTimeSpan(t1, t2);
   }

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

void ShowTimeSpan(ULONGLONG tstart, ULONGLONG tend)
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

void TestLeaks()
{
   auto* xxx = new char[10];
   strcpy_s(xxx,10, "Fat cat");
//   auto* yyy = CoTaskMemAlloc(100);
}

void ClearDynamicLibrary()
{
//   auto * factory = DynamicLibraryFactory::GetFactory();
//   factory->Clear();
}

string readFile(const string& path)
{
   wifstream wif(path);
   wif.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));
   wstringstream wss;
   wss << wif.rdbuf();

   using convert_type = codecvt_utf8<wchar_t>;
   wstring_convert<convert_type, wchar_t> converter;

   auto contents = converter.to_bytes(wss.str());
   return contents.substr(contents.find('<'));
}

string readFileIntoString(const string& path) {
   ifstream input_file(path);
//   input_file.imbue(locale(locale::empty(), new codecvt_utf8<wchar_t>));
   if (!input_file.is_open()) {
      cerr << "Could not open the file - '"
         << path << "'" << endl;
      exit(EXIT_FAILURE);
   }
   auto contents = string((istreambuf_iterator<char>(input_file)), istreambuf_iterator<char>());
   return contents.substr(contents.find('<'));

   //return string((istreambuf_iterator<char>(input_file)), istreambuf_iterator<char>());
}