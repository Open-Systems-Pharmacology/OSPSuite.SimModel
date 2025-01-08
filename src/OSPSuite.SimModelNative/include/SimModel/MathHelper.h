#ifndef _MathHelper_H_
#define _MathHelper_H_

#ifdef _WINDOWS
#include <math.h>
#endif

#if defined(linux) || defined (__APPLE__)
#include <cmath>
#endif

#include <string>

namespace SimModelNative
{

class MathHelper
{

	public:
		static double GetNaN ();
		static bool IsNaN (double d);
		static double GetInf ();
		static double GetNegInf ();
		static bool IsFinite (double d);
		static bool IsInf (double d);
		static bool IsNegInf (double d);
		static bool IsNumeric (const std::string & aString);
		static double Pi ();
		static double NormalDistribution (double Mean, double Sigma, double X);
		static double LogNormalDistribution (double Mean, double Std, double X, double & IntegralValue);
		static double LogNormalIntegral (double Mean, double Sigma);

		//Values-Array must be created by caller!!!
		static void LogDistribution (double Min, double Max, long NUM_POINTS, double * Values);
		static std::string ToString (double value);
};

}//.. end "namespace SimModelNative"


#endif //_MathHelper_H_

