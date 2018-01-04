#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/SimulationTask.h"
#include "SimModel/SwitchTask.h"
#include "SimModel/TableFormula.h"
#include "SimModel/TableFormulaWithOffset.h"
#include <set>
#include <map>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

int SimulationTask::NumberOfSimulatedTimeSteps(const vector <OutputTimePoint> & outputTimePoints)
{
	int noOfSimulatedTimeSteps = 0;

	for(unsigned int i=0; i<outputTimePoints.size(); i++)
	{
		if (outputTimePoints[i].SaveSystemSolution())
			noOfSimulatedTimeSteps++;
	}

	return noOfSimulatedTimeSteps;
}

vector <OutputTimePoint> SimulationTask::OutputTimePoints(Simulation * sim)
{
	bool useFloatComparison = sim->Options().UseFloatComparisonInUserOutputTimePoints();

	const DoubleQueue & userOutputTimePoints = useFloatComparison ? 
		                                       sim->GetOutputSchema().AllTimePoints<float>() :
											   sim->GetOutputSchema().AllTimePoints<double>();
	const DoubleQueue & switchTimePoints = SwitchTask::SwitchTimePoints(sim->Switches());
	const DoubleQueue & tableFormulaRestartTimePoints = TableFormulaRestartTimePoints(sim);

	return SimulationTask::OutputTimePoints(userOutputTimePoints, switchTimePoints, 
		                                    tableFormulaRestartTimePoints,
		                                    sim->GetStartTime());
}

DoubleQueue SimulationTask::TableFormulaRestartTimePoints(Simulation * sim)
{
	DoubleQueue restartTimePoints;

	for(int formulaIdx=0; formulaIdx<sim->Formulas().size(); formulaIdx++)
	{
		vector <double> singleRestartTimePoints;
		
		Formula * formula = sim->Formulas()[formulaIdx];

		TableFormula * tableFormula = dynamic_cast <TableFormula *>(formula);
		if (tableFormula != NULL)
			singleRestartTimePoints = tableFormula->RestartTimePoints();
		else
		{
			//not a table formula - try table formula with offset
			TableFormulaWithOffset * tableFormulaWithOffset = dynamic_cast <TableFormulaWithOffset *>(formula);
			if (tableFormulaWithOffset != NULL)
				singleRestartTimePoints = tableFormulaWithOffset->RestartTimePoints();
		}
		
		for(unsigned int pointIdx=0; pointIdx<singleRestartTimePoints.size(); pointIdx++)
			restartTimePoints.push(singleRestartTimePoints[pointIdx]);
	}

	return restartTimePoints;
}

vector <OutputTimePoint> SimulationTask::OutputTimePoints(DoubleQueue userOutputTimePoints, 
                                                          DoubleQueue switchTimePoints,
														  DoubleQueue tableFormulaRestartTimePoints,
 													      double simulationStartTime)
{
	const char * ERROR_SOURCE = "SimulationTask::OutputTimePoints";
	
	map<double, double> tableFormulaRestartTimePointsMap;

	vector <OutputTimePoint> outputTimePoints;
	
	//---- remove duplicates from Restart-Queue
	MakePriorityQueueUnique(tableFormulaRestartTimePoints);

	//---- add all table formula restart time points into the switch time points list
	while(!tableFormulaRestartTimePoints.empty())
	{
		double restartTimePoint = tableFormulaRestartTimePoints.top();
		tableFormulaRestartTimePoints.pop();

		switchTimePoints.push(restartTimePoint);
		tableFormulaRestartTimePointsMap[restartTimePoint] = restartTimePoint;
	}
	
	//---- remove duplicates from the queues
	MakePriorityQueueUnique(userOutputTimePoints); 
	MakePriorityQueueUnique(switchTimePoints);

	//---- remove all time points from user/switch queues which are <= simulation start time
	while(!userOutputTimePoints.empty() && (userOutputTimePoints.top() <= simulationStartTime))
	{
		userOutputTimePoints.pop();
	}

	while(!switchTimePoints.empty() && (switchTimePoints.top() <= simulationStartTime))
	{
		switchTimePoints.pop();
	}

	//at least one time point> simulationStartTime must be available
	if (userOutputTimePoints.size() == 0)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Time points output schema is empty");

	//---- loop through both queues and insert points
	double userTimePoint, switchTimePoint, timePoint;
	bool saveSolution, isSwitchTimePoint, restartSystem;

	while (!switchTimePoints.empty() && !userOutputTimePoints.empty())
	{
		switchTimePoint = switchTimePoints.top();
		switchTimePoints.pop();

		while (!userOutputTimePoints.empty())
		{
			userTimePoint = userOutputTimePoints.top();

			if (userTimePoint<=switchTimePoint)
			{
				timePoint = userTimePoint;
				saveSolution = true;
				isSwitchTimePoint = (userTimePoint == switchTimePoint);
				userOutputTimePoints.pop();
			}

			else //user time point> switch time point - insert switch time point and continue
			{
				timePoint = switchTimePoint;
				saveSolution = false;
				isSwitchTimePoint = true;
			}
			
			restartSystem = (tableFormulaRestartTimePointsMap.find(timePoint) != tableFormulaRestartTimePointsMap.end());

			outputTimePoints.push_back(OutputTimePoint(timePoint, saveSolution, isSwitchTimePoint, restartSystem));

			if (userTimePoint >= switchTimePoint)
				break;
		}
	}

	//---- insert user time points which are > latest switch time point
	while (!userOutputTimePoints.empty())
	{
		userTimePoint = userOutputTimePoints.top();
		userOutputTimePoints.pop();

		saveSolution = true;
		isSwitchTimePoint = false;
		restartSystem = false;
		outputTimePoints.push_back(OutputTimePoint(userTimePoint,saveSolution,isSwitchTimePoint,restartSystem));
	}

	return outputTimePoints;
}

void SimulationTask::MakePriorityQueueUnique(DoubleQueue & queue)
{
	set<double> uniqueElements;

	while (!queue.empty())
	{
		uniqueElements.insert(queue.top());
		queue.pop();
	}

	for (set<double>::iterator iter=uniqueElements.begin(); iter!=uniqueElements.end(); iter++)
	{
		queue.push(*iter);
	}
}


void SimulationTask::SetValuesBelowAbsTolLevelToZero(double * values, int valuesSize, double absTol)
{
	for(int i =0; i<valuesSize;i++)
	{
		if((values[i] < 0.0) && (values[i] > -absTol))
			values[i] = 0.0;
	}
}

void SimulationTask::CheckForNegativeValues(Species ** odeVariables, int numberOfVariables, double absTol)
{
	const char * ERROR_SOURCE = "SimulationTask::CheckForNegativeValues";
	vector<string> positiveVariablesWithNegativeValues;

	for (int varIdx = 0; varIdx < numberOfVariables; varIdx++)
	{
		Species * odeVariable = odeVariables[varIdx];

		if (odeVariable->NegativeValuesAllowed())
			continue;

		if (odeVariable->GetLatestValue() < -absTol * 100)
		{
			//found not allowed negative value. 
			//Cache variable name for the error message and continue with the next variable
			positiveVariablesWithNegativeValues.push_back(odeVariable->GetFullName());
			break;
		}
		
	}

	if (positiveVariablesWithNegativeValues.size()>0)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, GetErrorMessageForNegativeVaribales(positiveVariablesWithNegativeValues));

}

string SimulationTask::GetErrorMessageForNegativeVaribales(const vector<string> & positiveVariablesWithNegativeValues)
{
	string msg;

	msg += "Simulation run failed: some variables became negative. There are different possible reasons for this:";
	msg += "\n\n";
	msg += "  - Solver tolerances are too high. Please reduce the absolute and relative tolerances by one order of magnitude (e.g. from 1E-9 to 1E-10) and restart the simulation.";
	msg += "\n";
	msg += "  - Some variables which are allowed to be negative were defined as non-negative.";
	msg += "\n";
	msg += "  - Model is not properly established (e.g. the kinetic should be k*[A] but was defined as k*[B] etc.)";
	msg += "\n\n";
	msg += "The following variables became negative :\n";

	for (size_t i = 0; i < positiveVariablesWithNegativeValues.size(); i++)
		msg += positiveVariablesWithNegativeValues[i] + "\n";

	return msg;
}


}//.. end "namespace SimModelNative"
