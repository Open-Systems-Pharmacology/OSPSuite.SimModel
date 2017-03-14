#ifndef _OutputSchema_H_
#define _OutputSchema_H_

#include "SimModel/ObjectBase.h"
#include <vector>
#include <set>
#include "SimModel/TObjectVector.h"
#include "SimModel/SimModelTypeDefs.h"

namespace SimModelNative
{

enum OutputIntervalDistribution
{
	Equidistant = 1,
	Logarithmic = 2
};

class OutputTimePoint
{
private:
	double _time;
	bool   _saveSystemSolution;
	bool   _isSwitchTimePoint;
	bool   _restartSystem;
public:
	OutputTimePoint(double time, bool saveSystemSolution, bool isSwitchTimePoint, bool restartSystem);

	double Time() const;
	bool   SaveSystemSolution() const;
	bool   IsSwitchTimePoint() const;
	bool   RestartSystem() const;
};

class OutputInterval : 
	public XMLLoader
{
private:
	double _startTime;
	double _endTime;
	int _numberOfTimePoints;
	enum OutputIntervalDistribution _intervalDistribution;

public:
	SIM_EXPORT OutputInterval();
	SIM_EXPORT OutputInterval(double startTime, double endTime, int numberOfTimePoints, enum OutputIntervalDistribution intervalDistribution);

	SIM_EXPORT double StartTime();
	SIM_EXPORT void   SetStartTime(double time);

	SIM_EXPORT double EndTime();
	SIM_EXPORT void   SetEndTime(double time);

	SIM_EXPORT int    NumberOfTimePoints();
	SIM_EXPORT void   NumberOfTimePoints(int noOfTimePoints);

	SIM_EXPORT enum OutputIntervalDistribution IntervalDistribution();
	SIM_EXPORT void SetOutputIntervalDistribution(enum OutputIntervalDistribution intervalDistribution);

	std::vector<double> TimePoints();

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
};

class OutputSchema :
	public ObjectBase
{
private:
	TObjectVector<OutputInterval> _outputIntervals;

public:
	SIM_EXPORT OutputSchema(void);
	SIM_EXPORT virtual ~OutputSchema(void);

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	SIM_EXPORT void Clear();

	SIM_EXPORT TObjectVector<OutputInterval> & OutputIntervals();

	template<typename T>
	DoubleQueue AllTimePoints()
	{
		int i;
		unsigned int j;

		//first, put all time points into set to get only UNIQUE values
		std::set<double, std::greater<T> > timePointsSet;

		for (i = 0; i<_outputIntervals.size(); i++)
		{
			std::vector<double> intervalPoints = _outputIntervals[i]->TimePoints();

			for (j = 0; j<intervalPoints.size(); j++)
				timePointsSet.insert(intervalPoints[j]);
		}

		//now insert all time points into double queue to get them in increasing order
		DoubleQueue doubleQueue;

		std::set<double, std::greater<float> >::iterator iter;
		for (iter = timePointsSet.begin(); iter != timePointsSet.end(); iter++)
			doubleQueue.push(*iter);

		return doubleQueue;
	}
};

}//.. end "namespace SimModelNative"


#endif //_OutputSchema_H_

