
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
	static std::string GetErrorMessageForNegativeVaribales(const std::vector<std::string> & positiveVariablesWithNegativeValues);
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

	static void CheckForNegativeValues(Species ** odeVariables, int numberOfVariables, double absTol);
};

}//.. end "namespace SimModelNative"


#endif //_SwitchTask_H_
