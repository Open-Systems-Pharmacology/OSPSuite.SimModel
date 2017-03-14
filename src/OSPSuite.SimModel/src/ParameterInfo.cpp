#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/ParameterInfo.h"

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

ParameterInfo::ParameterInfo(void)
{
	_canBeVaried = false;
	_calculateSensitivity = false;
}

bool ParameterInfo::CanBeVaried() const
{
	return _canBeVaried;
}

void ParameterInfo::SetCanBeVaried(bool canBeVaried)
{
	_canBeVaried = canBeVaried;
}

bool ParameterInfo::CalculateSensitivity() const
{
	return _calculateSensitivity;
}

void ParameterInfo::SetCalculateSensitivity(bool calculateSensitivity)
{
	_calculateSensitivity = calculateSensitivity;
}

	bool ParameterInfo::IsTable() const
{
	return _tablePoints.size() > 0;
}


vector <ValuePoint> & ParameterInfo::GetTablePoints()
{
	return _tablePoints;
}

void ParameterInfo::SetTablePoints(const vector <ValuePoint> & tablePoints)
{
	_tablePoints.clear();

	for(vector <ValuePoint>::const_iterator iter = tablePoints.begin(); iter!= tablePoints.end(); iter++)
	{
		_tablePoints.push_back(*iter);
	}
}

void ParameterInfo::SetValue(double value)
{
	_tablePoints.clear();

	QuantityInfo::SetValue(value);
}

}//.. end "namespace SimModelNative"
