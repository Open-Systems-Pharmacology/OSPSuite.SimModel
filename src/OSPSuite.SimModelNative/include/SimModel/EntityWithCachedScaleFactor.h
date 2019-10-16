#ifndef _EntityWithCachedScaleFactor_H_
#define _EntityWithCachedScaleFactor_H_

namespace SimModelNative
{
	class EntityWithCachedScaleFactor
	{
	public:
		//Update cached scale factor of the ODE variable given by <odeIndex> to the new value given by <ODEScaleFactor>
		virtual void UpdateScaleFactorOfReferencedVariable(const int odeIndex, const double ODEScaleFactor) = 0;
	};

}
#endif 