#include "SimModel/Variable.h"
#include "ErrorData.h"

#include <assert.h>

#if defined(linux) || defined (__APPLE__)
#include <stdlib.h>
#include <cstring>
#endif

namespace SimModelNative
{

Variable::Variable(void)
{
	_values = NULL;
	_valuesSize = 0;
	_latestIndex = DE_INVALID_INDEX;
	_comparisonThreshold = 0.0;
}

Variable::~Variable(void)
{
	//Delete values Vector	
	if (_values!=NULL)	
		delete[] _values;
	_values = NULL;
}

void Variable::SetTheOnlyValue(double value)
{
	RedimValues(1);
	SetValue(0, value);
}

void Variable::SetComparisonThreshold(double threshold)
{
	_comparisonThreshold = threshold;
}

double Variable::GetComparisonThreshold() const
{
	return _comparisonThreshold;
}

void Variable::RedimValues (int p_ValuesSize)
{
	const char * ERROR_SOURCE = "Variable::RedimValues";
	
	assert(p_ValuesSize>=0);
	
	//Free memory 
	if (_values != NULL)
	{
		delete[] _values;
		_values = NULL;
	}
	
	//reset latest index
	_latestIndex = DE_INVALID_INDEX;

	if(p_ValuesSize>0)
	{
		_values = new double[p_ValuesSize];
		if (!_values)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Cannot allocate memory for the values vector");
		memset(_values, 0, p_ValuesSize * sizeof(double));
	}

	_valuesSize = p_ValuesSize;
}

void Variable::SetValue (int p_Index, double p_Value)
{
	assert((p_Index>=0) && (p_Index<_valuesSize));
    _values[p_Index] = p_Value;
    _latestIndex = p_Index;
}

double * Variable::GetValues () const
{
    return _values;
}

double Variable::GetLatestValue () const
{	
	if (_latestIndex == DE_INVALID_INDEX)
		return 0.0; 

	assert((_latestIndex>=0) && (_latestIndex<_valuesSize));	
	return _values[_latestIndex];
}

int Variable::GetValuesSize () const
{
    return _valuesSize;
}

void Variable::SetLatestValue (const double dValue)
{
	//First time do nothing
	if (_latestIndex==DE_INVALID_INDEX) return;
	
	assert((_latestIndex>=0) && (_latestIndex<_valuesSize));	
	_values[_latestIndex] = dValue;
}

}//.. end "namespace SimModelNative"
