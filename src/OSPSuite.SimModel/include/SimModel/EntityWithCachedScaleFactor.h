#ifndef _EntityWithCachedScaleFactor_H_
#define _EntityWithCachedScaleFactor_H_

namespace SimModelNative
{
	class EntityWithCachedScaleFactor
	{
	public:
		//Update the value to ODEScaleFactor of the scale factor of the variable with the id referenced by this formula.
		virtual void UpdateScaleFactorOfReferencedVariable(const int odeIndex, const double ODEScaleFactor) = 0;
	};

}
#endif 