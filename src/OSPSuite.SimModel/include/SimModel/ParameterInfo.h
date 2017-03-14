#ifndef _ParameterInfo_H_
#define _ParameterInfo_H_

#include "SimModel/Formula.h"
#include "SimModel/QuantityInfo.h"

namespace SimModelNative
{

class ParameterInfo 
		: public QuantityInfo
{
protected:
	bool _canBeVaried;
	bool _calculateSensitivity;
	std::vector <ValuePoint> _tablePoints;

public:
	SIM_EXPORT ParameterInfo(void);

	SIM_EXPORT bool CanBeVaried() const;
	void SetCanBeVaried(bool canBeVaried);

	SIM_EXPORT bool CalculateSensitivity() const;
	SIM_EXPORT void SetCalculateSensitivity(bool calculateSensitivity);

	SIM_EXPORT bool IsTable() const;

	SIM_EXPORT std::vector <ValuePoint> & GetTablePoints();
	SIM_EXPORT void SetTablePoints(const std::vector <ValuePoint> & tablePoints);

	SIM_EXPORT void SetValue(double value);
};

}//.. end "namespace SimModelNative"

#endif //_ParameterInfo_H_

