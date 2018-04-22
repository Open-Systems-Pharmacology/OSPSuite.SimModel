#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/Observer.h"
#include "SimModel/Simulation.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

Observer::Observer(void)
{
	_originalValueFormula = NULL;
	_originalValue = 0.0;
}

Observer::Observer(long objectId, const string & name, const string & fullName,
	               const string & description, const string & containerPath, const string & unit,
	               Formula * valueFormula, const string & entityId, const string & pathWithoutRoot)
{
	_id = objectId;
    _name = name;
	_fullName = fullName;
	_description = description;
	_containerPath = containerPath;
	_unit = unit;
    _valueFormula = valueFormula;
	_originalValueFormula = _valueFormula;
	_originalFormulaID = _valueFormula->GetId();
	_entityId = entityId;
	_isPersistable = true;
	_pathWithoutRoot = pathWithoutRoot;
}


Observer::~Observer(void)
{
	//_parameterSensitivities.clear();
}

void Observer::LoadFromXMLNode (const XMLNode & pNode)
{
	// ---- XML sample
	//<Observer Id="O1" Name="Obs1" Path="Liver/Cells" FormulaId="F5" Unit="µmol"/>

	//common quantity part
	Quantity::LoadFromXMLNode(pNode);

	//observer specific part
	// nothing to do so far
}

string Observer::getFormulaXMLAttributeName()
{
	return XMLConstants::FormulaId;
}

void Observer::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	//---- common quantity part
	Quantity::XMLFinalizeInstance(pNode, sim);

	//---- observer specific part
	//     (nothing to do so far)
}

double Observer::GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode)
{
	return GetLatestValue();
}

double Observer::CalculateValue(const double * y, double time, ScaleFactorUsageMode scaleFactorMode)
{
	if (_valueFormula)
	{
		return _valueFormula->DE_Compute(y, time, scaleFactorMode);
	}
	else
		return _value;
}

void Observer::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	//may not be called (unless observers are allowed in RHS formulas)
	throw ErrorData(ErrorData::ED_ERROR, "Observer::DE_Jacobian", "Jacobian of observer may not be called!");
}

Formula* Observer::DE_Jacobian(const int iEquation)
{
	return _valueFormula->DE_Jacobian(iEquation);
	//may not be called (unless observers are allowed in RHS formulas)
	//throw ErrorData(ErrorData::ED_ERROR, "Observer::DE_Jacobian", "Jacobian of observer may not be called!");
}

Formula* Observer::getValueFormula() {
	return _valueFormula;
}

bool Observer::IsConstantDuringCalculation()
{
	bool forCurrentRunOnly = true;

	return IsConstant(forCurrentRunOnly);
}

}//.. end "namespace SimModelNative"
