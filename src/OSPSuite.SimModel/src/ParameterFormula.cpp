#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "SimModel/ParameterFormula.h"
#include "SimModel/Parameter.h"
#include "SimModel/Simulation.h"
#include "SimModel/GlobalConstants.h"

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

	using namespace std;

ParameterFormula::ParameterFormula ()
{
	//m_IsTime = false;
}

ParameterFormula::ParameterFormula(long formulaId, const string & name, 
								   Parameter * parameter, const string & alias)
{
	_id = formulaId;
	m_Name = name;
	_quantityRef.SetupFrom(parameter, alias);
}

bool ParameterFormula::IsZero(void)
{
	bool forCurrentRunOnly = false;

	if(!IsConstant(forCurrentRunOnly))
		return false;

	double value = DE_Compute(NULL, 0.0, USE_SCALEFACTOR);

	return (value == 0.0);
}

void ParameterFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	 // Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Parameter));
	m_Name = pNode.GetValue();
	
}

void ParameterFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
 	//Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Parameter));
	
	//assert that we have the good object!! 
	assert(_quantityRef.GetAlias() == m_Name);
}

void ParameterFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	if (m_Name != quantityReference.GetAlias()) return;
	_quantityRef = quantityReference;
}

double ParameterFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return _quantityRef.GetValue(y, time, scaleFactorMode);
}

void ParameterFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	_quantityRef.DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

void ParameterFormula::Finalize()
{
	//nothing to do 
}

bool ParameterFormula::IsTime()
{
	return _quantityRef.IsTime();
}

bool ParameterFormula::IsConstant(bool forCurrentRunOnly)
{
	return _quantityRef.IsConstant(forCurrentRunOnly);
}

void ParameterFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	if (_quantityRef.IsTime())
		mrOut<<csTime;
	else if (_quantityRef.IsParameter())
	{
		Parameter * param = dynamic_cast<Parameter *>(_quantityRef.GetHierarchicalFormulaObject());
		assert (param != NULL); 

		if (param->IsChangedBySwitch() || param->IsTable())
			mrOut<<"EvalParameter("<<param->GetShortUniqueName()<<", Time, y)";
		else
			mrOut<<param->GetShortUniqueName();
	}

	else if (_quantityRef.IsSpecies())
	{
		Species * species = dynamic_cast<Species *>(_quantityRef.GetHierarchicalFormulaObject());
		assert (species != NULL); 

		//TODO isn't it sufficient just to call species->GetInitialValue(NULL,0.0)?
		if (species->IsConstantDuringCalculation())
		{
			double * values = species->GetValues();

			if(values)
				mrOut<<values[0];
			else
				mrOut<<species->GetInitialValue(NULL,0.0);
		}
		else
			mrOut<<species->GetInitialValue(NULL,0.0);
	}

	else
	{
		throw ErrorData(ErrorData::ED_ERROR, "ParameterFormula::WriteFormulaMatlabCode", "Cannot write matlab code for " + _quantityRef.GetHierarchicalFormulaObject()->GetFullName());
	}
}

bool ParameterFormula::UseBracketsForODESystemGeneration ()
{
	return false;
}

vector <double> ParameterFormula::SwitchTimePoints()
{
	return vector <double> ();
}

string ParameterFormula::Equation()
{
	return m_Name;
}

void ParameterFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	//---- first, add variables used in parameter formula
	_quantityRef.AppendUsedVariables(usedVariblesIndices, variblesIndicesUsedInSwitchAssignments);

	//---- second: if parameter is changed by switches, add all DE variables
	//             that can be potentially set into parameter formula
	if (_quantityRef.IsChangedBySwitch())
	{
		usedVariblesIndices.insert(variblesIndicesUsedInSwitchAssignments.begin(), 
			                       variblesIndicesUsedInSwitchAssignments.end());
	}
}

void ParameterFormula::UpdateIndicesOfReferencedVariables()
{
	_quantityRef.UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"