
#ifndef _SimulationTask_H_
#define _SimulationTask_H_


#include "SimModel/Simulation.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/OutputSchema.h"
#include <vector>

namespace SimModelNative
{

class SimulationTask
{
private:
	static std::string GetErrorMessageForNegativeVariables(const std::vector<std::string> & positiveVariablesWithNegativeValues, double solverOutputTime);
protected:
	static std::vector <OutputTimePoint> OutputTimePoints(DoubleQueue userOutputTimePoints, 
	                                                      DoubleQueue switchTimePoints,
														  DoubleQueue tableFormulaRestartTimePoints,
														  double simulationStartTime);
	static DoubleQueue TableFormulaRestartTimePoints(Simulation * sim);
	static void MakePriorityQueueUnique(DoubleQueue & queue);
public:
	static std::vector <OutputTimePoint> OutputTimePoints(Simulation * sim);
	static int NumberOfSimulatedTimeSteps(const std::vector <OutputTimePoint> & outputTimePoints);

	static void SetValuesBelowAbsTolLevelToZero(double * values, int valuesSize, double absTol);

	static void CheckForNegativeValues(Species ** odeVariables, int numberOfVariables, double absTol, double solverOutputTime);

	//mark all parameters which are used in any ODE varaible or observer as unused
	static void MarkUsedParameters(Simulation * sim);

	//cache DE variables indices used in the RHS equations
	//in order to speed up the jacobian calculation
	static void CacheRHSUsedVariables(Simulation * sim);

	//for debug only: write out RHS dependency matrix
	static void WriteRHSDependencyMatrix(Simulation * sim, const std::string & filename);
};

}//.. end "namespace SimModelNative"


#endif //_SwitchTask_H_
