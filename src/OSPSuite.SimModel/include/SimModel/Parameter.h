#ifndef _Parameter_H_
#define _Parameter_H_

#include "SimModel/Quantity.h"
#include "SimModel/HierarchicalFormulaObject.h"
#include "SimModel/ParameterInfo.h"

namespace SimModelNative
{

class Observer;

class Parameter :
	public HierarchicalFormulaObject
{
protected:
	bool _canBeVaried;
	bool _calculateSensitivity;

	std::string _shortUniqueName;
	std::string getFormulaXMLAttributeName();
	void FillInfoWithParameterSpecificProperties(ParameterInfo & info);

public:
	Parameter(void);
	virtual ~Parameter(void);

	bool CanBeVaried () const;
	void SetCanBeVaried(bool canBeVaried);

	bool CalculateSensitivity() const;
	void SetCalculateSensitivity(bool calculateSensitivity);

	virtual void LoadFromXMLNode (const XMLNode & pNode);
	virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	std::vector < HierarchicalFormulaObject * > GetUsedHierarchicalFormulaObjects ();

	double GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode);
	virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);

	std::string GetShortUniqueName ();
	void SetShortUniqueNameForDESystem (const std::string & uniqueName);
	void WriteMatlabCode (std::ostream & mrOut, bool forInitialValue = false);

	//will be called between Load and Finalize of the parent simulation
	void InitialFillInfo(ParameterInfo & info);

	//will be called after the finalize of the parent simulation
	void FillInfo(ParameterInfo & info,const double * speciesInitialValues, double simulationStartTime);

	//create observer from parameter in order to return parameter values over time
	Observer * CreateObserverWithId(long objectId, Formula * observerFormula);

	bool IsConstant(bool forCurrentRunOnly);
	bool IsTable(void);
	void SetTablePoints(const std::vector <ValuePoint> & valuePoints);

	//true, if parameter should be declared as global in Matlab export code
	bool ExportAsGlobalForMatlab();

	//the name of the table function which will be assigned to the parameter in case it's table-based
	std::string TableFunctionNameForMatlab();

	//writes the table function in matlab code
	void WriteTableFunctionForMatlab(std::ostream & mrOut);
};

}//.. end "namespace SimModelNative"


#endif //_Parameter_H_
