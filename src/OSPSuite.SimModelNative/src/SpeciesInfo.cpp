#include "SimModel/SpeciesInfo.h"
#include "SimModel/MathHelper.h"

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
