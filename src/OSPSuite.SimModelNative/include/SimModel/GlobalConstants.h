#ifndef _GlobalConstants_H_
#define _GlobalConstants_H_

#ifdef _WINDOWS
#include <ymath.h>
#define SIM_EXPORT __declspec(dllexport)
#pragma warning( disable : 4005)
#endif

#if defined(linux) || defined (__APPLE__)
#include <cmath>
#define SIM_EXPORT
#endif

#include <string>

namespace SimModelNative
{

enum ScaleFactorUsageMode
{
	USE_SCALEFACTOR = 1,
	IGNORE_SCALEFACTOR = 2
};

extern const char * csTime;
extern const double csSimModelVersion;
extern const double csSimModelMinVersion;
extern const int DE_INVALID_INDEX;
extern const int HFOBECT_INVALID_INDEX;
extern const int HFOBJECT_TOP_LEVEL;
extern const int DE_NOERROR;
extern const long INVALID_QUANTITY_ID;
extern const long TIME_QUANTITY_ID;
const bool IsNan (const double d) ;
extern const int OLD_SIMMODEL_XML_VERSION;

class XMLConstants
{
public:
	static const std::string SimModelSchema;
	SIM_EXPORT static const std::string GetSchemaNamespace();

	static const std::string Simulation;
	static const std::string ObjectPathDelimiter;
	static const std::string SimModelXMLVersion;

	static const std::string Id;
	static const std::string EntityId;
	static const std::string Name;
	static const std::string Description;
	static const std::string Path;
	static const std::string Persistable;

	static const std::string ParameterList;
	static const std::string Parameter;
	static const std::string FormulaId;
	static const std::string Unit;
	static const std::string CanBeVaried;
	static const std::string Value;
	static const std::string CalculateSensitivity;

	static const std::string VariableList;
	static const std::string Variable;
	static const std::string InitialValueFormulaId;
	static const std::string ScaleFactor;
	static const std::string RHSFormulaList;
	static const std::string RHSFormula;
	static const std::string NegativeValuesAllowed;

	static const std::string ReferenceList;
	static const std::string Reference;

	static const std::string ObserverList;
	static const std::string Observer;

	static const std::string FormulaList;
	static const std::string Formula;

	static const std::string ExplicitFormula;
	static const std::string Alias;
	static const std::string Equation;

	static const std::string SwitchList;
	static const std::string Switch;
	static const std::string ConditionFormulaId;
	static const std::string ChangeList;
	static const std::string Change;
	static const std::string ObjectId;
	static const std::string NewFormulaId;
	static const std::string UseAsValue;
	static const std::string OneTime;

	static const std::string OutputSchema;
	static const std::string OutputInterval;
	static const std::string OutputIntervalList;
	static const std::string Distribution;
	static const std::string DistributionEquidistant;
	static const std::string DistributionLogarithmic;
	static const std::string StartTime;
	static const std::string EndTime;
	static const std::string NumberOfTimePoints;
	static const std::string OutputTime;
	static const std::string OutputTimeList;

	static const std::string Solver;
	static const std::string AbsTol;
	static const std::string RelTol;
	static const std::string H0;
	static const std::string HMin;
	static const std::string HMax;
	static const std::string MxStep;
	static const std::string UseJacobian;
	static const std::string CheckForNegativeValues;
	static const std::string SolverOptionsList;
	static const std::string SolverOption;

    static const std::string TableFormula;
    static const std::string PointList;
	static const std::string ValuePointX;
	static const std::string ValuePointY;
	static const std::string RestartSolver;
	static const std::string UseDerivedValues;
	
	static const std::string TableWithOffsetFormula;
	static const std::string TableObject;
	static const std::string OffsetObject;

	static const std::string TableWithXArgumentFormula;
	static const std::string XArgumentObject;
};

class FormulaName
{
public:
	static const std::string Constant;
	static const std::string Power; 
	static const std::string Exp; 
	static const std::string Product; 
	static const std::string Sum; 
	static const std::string Diff;
	static const std::string Div;
	static const std::string Parameter;
	static const std::string Variable;
	static const std::string SimpleProduct;
	static const std::string ExplicitFormula; 
	static const std::string TableFormula; 
	static const std::string TableFormulaWithOffset;
	static const std::string TableFormulaWithXArgument;
	static const std::string Min; 
	static const std::string Max; 
	static const std::string IF; 
	static const std::string Equal; 
	static const std::string Unequal; 
	static const std::string Less; 
	static const std::string LessEqual; 
	static const std::string Greater; 
	static const std::string GreaterEqual; 
	static const std::string And; 
	static const std::string Or; 
	static const std::string Not; 
	static const std::string VariableWithArgument;
	static const std::string Log10;
    static const std::string Acos;
    static const std::string Asin;
    static const std::string Atan;
    static const std::string Cosh;
    static const std::string Cos;
    static const std::string Log;
    static const std::string Ln;
    static const std::string Sinh;
    static const std::string Sin;
    static const std::string Sqrt;
    static const std::string Tanh;
    static const std::string Tan;
};

class FormulaConstants
{
public:
	static const std::string FirstOperand;
	static const std::string SecondOperand; 

	static const std::string FirstArgument;
	static const std::string SecondArgument; 

	static const std::string IfStatement; 
	static const std::string ThenStatement; 
	static const std::string ElseStatement; 

	static const std::string Base;
	static const std::string Exponent;
	static const std::string Numerator;
	static const std::string Denominator;
	static const std::string Minuend;
	static const std::string Subtrahend;

};

}//.. end "namespace SimModelNative"


#endif //_GlobalConstants_H_

