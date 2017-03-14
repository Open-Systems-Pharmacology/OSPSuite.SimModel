#ifndef _SpeciesInfo_H_
#define _SpeciesInfo_H_

#include "SimModel/Formula.h"
#include "SimModel/QuantityInfo.h"

namespace SimModelNative
{

class SpeciesInfo 
		: public QuantityInfo
{
protected:
	double _scaleFactor;
public:
	SIM_EXPORT SpeciesInfo(void);

	SIM_EXPORT double GetScaleFactor();
	SIM_EXPORT void SetScaleFactor(double scaleFactor);
};

}//.. end "namespace SimModelNative"


#endif //_SpeciesInfo_H_

