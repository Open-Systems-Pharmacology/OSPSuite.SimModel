#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/MathHelper.h"
#include "XMLWrapper/XMLHelper.h"
#include <ErrorData.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

#ifdef linux
#include <cstring>
#endif

namespace SimModelNative
{

double MathHelper::GetNaN ()
{
#ifdef _WINDOWS
	return _Nan._Double;
#endif

#ifdef linux
	return NAN;
#endif
}

bool MathHelper::IsNaN (double d)
{
#ifdef _WINDOWS
        return _isnan(d) ? true : false;
#endif
#ifdef linux
        return std::isnan(d);
#endif
}

double MathHelper::GetInf ()
{
#ifdef _WINDOWS
	return _Inf._Double;
#endif

#ifdef linux
	return INFINITY;
#endif
}

double MathHelper::GetNegInf ()
{
	return -GetInf();
}

bool MathHelper::IsFinite (double d)
{
#ifdef _WINDOWS
        return _finite(d) ? true : false;
#endif
#ifdef linux
        return (finite(d) != 0);
#endif
}

bool MathHelper::IsInf (double d)
{
	return (d==GetInf());
}

bool MathHelper::IsNegInf (double d)
{
	return (d==GetNegInf());
}

bool MathHelper::IsNumeric (const std::string & aString)
{
	if (aString=="") return false;

	//replace point and comma with current decimal separator USED BY C++ RUNTIME LIBRARY
	//This decimal separator is used by strtod function while converting strings to
	//double numbers
	std::string NewString = aString;
	const char * DecSep = localeconv()->decimal_point;

	if (strcmp(",", DecSep))
		NewString = XMLHelper::StringReplace(NewString, ",", DecSep);

	if (strcmp(".", DecSep))
		NewString = XMLHelper::StringReplace(NewString, ".", DecSep);

	const char *p = NewString.c_str();
 	char *pend = 0;
	strtod( p, &pend );
	return (*pend == '\0');
}

double MathHelper::Pi ()
{
	return 2*asin(1.0);
}

double MathHelper::NormalDistribution (double Mean, double Sigma, double X)
{
	double a, b;

	a = 1.0 / (Sigma * sqrt(2.0 * MathHelper::Pi()));
	b = 1.0 / (2.0 * Sigma * Sigma);

	return a * exp(-(X - Mean) * (X - Mean) * b);
}

double MathHelper::LogNormalDistribution (double Mean, double Std, double X, double & IntegralValue)
{
	double LogMean = log(Mean);
	double LogSigma = sqrt(log(1.0 + Std * Std));

	if (IntegralValue == 0.0) //if integral value of the distribution is not given - calculate it
		IntegralValue = LogNormalIntegral(LogMean, LogSigma);

	return NormalDistribution(LogMean, LogSigma, log(X)) / IntegralValue;
}

double MathHelper::LogNormalIntegral (double Mean, double Sigma)
{
	const long NUM_POINTS = 1000;

	double XValues[NUM_POINTS];
	double XLogValues[NUM_POINTS];
	double YValues[NUM_POINTS];

	double Min = Mean - 10.0 * Sigma;
	double Max = Mean + 10.0 * Sigma;

	double IntegralValue = 0.0;

	long i;

	for(i=0; i<NUM_POINTS; i++)
	{
		//linear distribution of X values in log. scale
		XLogValues[i] = Min + i * (Max - Min) / (NUM_POINTS - 1);

		//normal distribution values
		YValues[i] = NormalDistribution(Mean, Sigma, XLogValues[i]);
	}

	//log. distribution of X values in linear scale
	LogDistribution(pow(exp(1.0), Min), pow(exp(1.0), Max), NUM_POINTS, XValues);

	//calculate integral value
	for(i=1; i<NUM_POINTS; i++)
		IntegralValue += YValues[i-1] * (XValues[i] - XValues[i - 1]);

	return IntegralValue;
}

void MathHelper::LogDistribution (double Min, double Max, long NUM_POINTS, double * Values)
{
	const char * ERROR_SOURCE = "MathHelper::LogDistribution";

	double Multiplier;
	long i;

	if ((NUM_POINTS<=0) || (Min == 0.0) || ((Min < 0.0) && (Max > 0.0)) || ((Min > 0.0) && (Max < 0.0)))
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Invalid parameters passed");

	Values[0] = Min;
	if (NUM_POINTS == 1) //not so much sense to produce log distribution for 1 point, by why not...
		return;

	Multiplier = pow((Max / Min), 1.0 / (NUM_POINTS - 1));

	for (i=1; i<NUM_POINTS; i++)
		Values[i] = Values[i-1] * Multiplier;
}

std::string MathHelper::ToString (double value)
{
	if (IsNaN(value))
		return "NaN";

	if (IsInf(value))
		return "Inf";

	if (IsNegInf(value))
		return "-Inf";

	return XMLHelper::ToString(value);
}

}//.. end "namespace SimModelNative"
