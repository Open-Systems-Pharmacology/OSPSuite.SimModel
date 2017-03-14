#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "SimModel/OutputSchema.h"
#include "ErrorData.h"
#include <set>
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

OutputTimePoint::OutputTimePoint(double time, bool saveSystemSolution, bool isSwitchTimePoint, bool restartSystem)
{
	_time = time;
	_saveSystemSolution = saveSystemSolution;
	_isSwitchTimePoint = isSwitchTimePoint;
	_restartSystem = restartSystem;
}

double OutputTimePoint::Time() const
{
	return _time;
}

bool   OutputTimePoint::SaveSystemSolution() const
{
	return _saveSystemSolution;
}

bool   OutputTimePoint::IsSwitchTimePoint() const
{
	return _isSwitchTimePoint;
}

bool OutputTimePoint::RestartSystem() const
{
	return _restartSystem;
}

OutputInterval::OutputInterval()
{
	_startTime = 0.0;
	_endTime = 0.0;
	_numberOfTimePoints = 0;
	_intervalDistribution = Equidistant;
}

OutputInterval::OutputInterval(double startTime, double endTime, int numberOfTimePoints, enum OutputIntervalDistribution intervalDistribution)
{
	_startTime = startTime;
	_endTime = endTime;
	_numberOfTimePoints = numberOfTimePoints;
	_intervalDistribution = intervalDistribution;
}

double OutputInterval::StartTime()
{
	return _startTime;
}

void   OutputInterval::SetStartTime(double time)
{
	_startTime = time;
}

double OutputInterval::EndTime()
{
	return _endTime;
}

void   OutputInterval::SetEndTime(double time)
{
	_endTime = time;
}

int OutputInterval::NumberOfTimePoints()
{
	return _numberOfTimePoints;
}

void OutputInterval::NumberOfTimePoints(int noOfTimePoints)
{
	_numberOfTimePoints = noOfTimePoints;
}

enum OutputIntervalDistribution OutputInterval::IntervalDistribution()
{
	return _intervalDistribution;
}

void OutputInterval::SetOutputIntervalDistribution(enum OutputIntervalDistribution intervalDistribution)
{
	_intervalDistribution = intervalDistribution;
}

vector<double> OutputInterval::TimePoints()
{
	const char * ERROR_SOURCE = "OutputInterval::TimePoints";

	vector<double> points;
	int i;
	double dt;

	if (_startTime>_endTime)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Value for start time should be less than or equal to value for end time");

	if (_startTime == _endTime) //just a single point
	{
		points.push_back(_startTime);
		return points;
	}

	if (_numberOfTimePoints<2)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "At least 2 points for time interval required");

	switch(_intervalDistribution)
	{
	case Equidistant:
		dt = (_endTime-_startTime)/(_numberOfTimePoints-1);

		points.push_back(_startTime);

		for(i=1;i<_numberOfTimePoints-1;i++)
			points.push_back(_startTime + dt*i);

		points.push_back(_endTime);

		break;

	case Logarithmic:
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Logarithmic interval distribution not implemented yet");

	default:
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Interval distribution unknown");
	}

	return points;
}

void OutputInterval::LoadFromXMLNode (const XMLNode & pNode)
{
	assert(pNode.HasName(XMLConstants::OutputInterval));
	//nothing to do so far
}

void OutputInterval::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	const char * ERROR_SOURCE = "OutputInterval::XMLFinalizeInstance";

	assert(pNode.HasName(XMLConstants::OutputInterval));

	string intervalDistribution = pNode.GetAttribute(XMLConstants::Distribution);

	if (intervalDistribution == XMLConstants::DistributionEquidistant)
		_intervalDistribution = Equidistant;

	else if (intervalDistribution == XMLConstants::DistributionLogarithmic)
		_intervalDistribution = Logarithmic;

	else
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Interval distribution not supported: "+ intervalDistribution);

	_startTime = pNode.GetChildNodeValue(XMLConstants::StartTime, _startTime);
	_endTime = pNode.GetChildNodeValue(XMLConstants::EndTime, _endTime);
	_numberOfTimePoints = (int)pNode.GetChildNodeValue(XMLConstants::NumberOfTimePoints, _numberOfTimePoints);
}


OutputSchema::OutputSchema(void)
{
}

OutputSchema::~OutputSchema(void)
{
	Clear();
}

void OutputSchema::LoadFromXMLNode (const XMLNode & pNode)
{
	assert(pNode.HasName(XMLConstants::OutputSchema));
	ObjectBase::LoadFromXMLNode(pNode);
}

void OutputSchema::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	assert(pNode.HasName(XMLConstants::OutputSchema));
	ObjectBase::XMLFinalizeInstance(pNode, sim);

	//---- load and finalize intervals
	XMLNode intervalListNode = pNode.GetChildNode(XMLConstants::OutputIntervalList);

	if(!intervalListNode.IsNull())
	{
		for(XMLNode pIntervalNode = intervalListNode.GetFirstChild(); !pIntervalNode.IsNull(); pIntervalNode=pIntervalNode.GetNextSibling())
		{
			OutputInterval * interval = new OutputInterval();

			interval->LoadFromXMLNode(pIntervalNode);
			interval->XMLFinalizeInstance(pIntervalNode, sim);

			_outputIntervals.push_back(interval);
		}
	}

	//---- load single points
	XMLNode pointListNode = pNode.GetChildNode(XMLConstants::OutputTimeList);
	
	if (!pointListNode.IsNull())
	{
		for(XMLNode pPointNode = pointListNode.GetFirstChild(); !pPointNode.IsNull(); pPointNode=pPointNode.GetNextSibling())
		{
			double timeValue = pPointNode.GetValue(0.0);

			OutputInterval * interval = new OutputInterval(
				timeValue, timeValue, 1, Equidistant);

			_outputIntervals.push_back(interval);
		}
	}

}

void OutputSchema::Clear()
{
	_outputIntervals.clear();
}

TObjectVector<OutputInterval> & OutputSchema::OutputIntervals()
{
	return _outputIntervals;
}

}//.. end "namespace SimModelNative"
