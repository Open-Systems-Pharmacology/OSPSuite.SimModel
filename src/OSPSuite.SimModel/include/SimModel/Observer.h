#ifndef _Observer_H_
#define _Observer_H_
 
#include "SimModel/Quantity.h"
#include "SimModel/VariableWithParameterSensitivity.h"

namespace SimModelNative
{

class Observer :
	public Quantity,
	public VariableWithParameterSensitivity
{
protected:
	std::string getFormulaXMLAttributeName();
	
public:
	Observer(void);
	Observer(long objectId, const std::string & name, const std::string & fullName,
		     const std::string & description, const std::string & containerPath, const std::string & unit,
			 Formula *	valueFormula, const std::string & entityId, const std::string & pathWithoutRoot);
	virtual ~Observer(void);
	
	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	//GetValue uses LAST SAVED observer value. Is needed because observers still
	// can be used in another observers
	double GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode);
	virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);

	//CalculateValue uses observer FORMULA to calculate its actual value
	double CalculateValue(const double * y, double time, ScaleFactorUsageMode scaleFactorMode);

	bool IsConstantDuringCalculation();

};

}//.. end "namespace SimModelNative"


#endif //_Observer_H_
