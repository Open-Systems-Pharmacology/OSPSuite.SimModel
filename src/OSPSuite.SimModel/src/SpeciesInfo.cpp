#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/SpeciesInfo.h"
#include "SimModel/MathHelper.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

SpeciesInfo::SpeciesInfo(void)
{
	_scaleFactor = MathHelper::GetNaN();
}

double SpeciesInfo::GetScaleFactor()
{
	return _scaleFactor;
}

void SpeciesInfo::SetScaleFactor(double scaleFactor)
{
	_scaleFactor = scaleFactor;
}

}//.. end "namespace SimModelNative"
