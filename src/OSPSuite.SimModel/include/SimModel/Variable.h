#ifndef _Variable_H_
#define _Variable_H_

#include <string>
#include "SimModel/GlobalConstants.h"

namespace SimModelNative
{

class Variable
{
protected:
	int _latestIndex;
	double * _values;
	std::string m_Name;
	int _valuesSize;
	double _comparisonThreshold;

public:
	Variable(void);
	virtual ~Variable(void);
	
public:
	virtual void RedimValues (int p_ValuesSize);
	void SetValue (int p_Index, double p_Value);
	SIM_EXPORT double * GetValues () const;
	double GetLatestValue () const;
	SIM_EXPORT int GetValuesSize () const;
	void SetLatestValue (const double dValue);
	void SetTheOnlyValue(double value);
	void SetComparisonThreshold(double threshold);
	SIM_EXPORT double GetComparisonThreshold() const;
};

}//.. end "namespace SimModelNative"

#endif //_Variable_H_
