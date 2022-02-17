#ifndef __TESTSIMMODEL_H
#define __TESTSIMMODEL_H

#include <iostream>
#include <conio.h>

#include "SimModel/Simulation.h"
#include "SimModel/PInvokeHelper.h"
#include "SimModel/CppODEExporter.h"
#include "SimModel/PInvokeQuantity.h"
#include "SimModel/PInvokeSimulation.h"
#include "DynamicLibrary.h"

using namespace std;
using namespace SimModelNative;

void evalPInvokeErrorMsg(const bool success, char* errorMessage);
void ShowErrorMessage(const string& msg);
void ShowErrorMessage(const ErrorData& ED);

string BasisDir(const string & exeName);
string TestFileFrom(const string& fileName);
void ShowTimeSpan(double tstart, double tend);

Simulation* LoadSimulation(const string& fileName, bool keepXmlString =false);
void FinalizeSimulation(Simulation* sim);
void RunSimulation(Simulation* sim, bool showInfo=true);
void ShowFirstWarning(Simulation* sim);
vector<ParameterInfo>* GetParameterProperties(Simulation* sim);
void SetVariableParameters(Simulation* sim, vector<ParameterInfo>* parameterProperties, vector<int> variableParameterIndices);
void SetParameterValues(Simulation* sim, vector<ParameterInfo>* parameterProperties, vector<int> variableParameterIndices);

void TestLeaks(void);

void Test1(const string& simName);
void TestSensitivities(
    const string& simName,
    const vector<string> variableParameterPaths,
    const vector<double> variableParameterValues,
    vector<int> variableParameterIndices,
    const vector<bool> calculateSensitivities);
void TestSetTablePoints();
void TestCPPExport(const string& simName);

void ClearDynamicLibrary();
double* SensitivityValuesByPathsFor(Simulation* simulation, const char* quantityPath, const char* parameterPath);
#endif
