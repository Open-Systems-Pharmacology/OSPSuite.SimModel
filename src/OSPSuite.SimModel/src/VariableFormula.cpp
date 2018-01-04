#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/VariableFormula.h"
#include <assert.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

VariableFormula::VariableFormula()
{
	m_ODEVariableIndex = DE_INVALID_INDEX;
	m_ODEVariableScaleFactor = 1.0;
}

std::string VariableFormula::GetName ()
{
    return m_Name;
}

bool VariableFormula::IsZero(void)
{
	return false;
}

void VariableFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	 // Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Variable));
	m_Name = pNode.GetValue();
}

void VariableFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	 // Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Variable));
}

void VariableFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	if (m_Name != quantityReference.GetAlias()) return;
	_quantityRef = quantityReference;

	m_ODEVariableIndex = _quantityRef.GetODEIndex();
	m_ODEVariableScaleFactor = _quantityRef.GetODEScaleFactor();
}

void VariableFormula::Finalize()
{
}

double VariableFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	//Formula is :  
	//C[DE_INDEX]*ScaleFactor
	if (m_ODEVariableIndex == DE_INVALID_INDEX)
		throw ErrorData(ErrorData::ED_ERROR, "VariableFormula::DE_Compute", "Variable ODE Index not set" + FormulaInfoForErrorMessage());
	
	double scaleFactor;
	switch (scaleFactorMode)
		{
		case USE_SCALEFACTOR:
			scaleFactor = m_ODEVariableScaleFactor;
			break;
		case IGNORE_SCALEFACTOR:
			scaleFactor = 1.0;
			break;
		default:
			throw ErrorData(ErrorData::ED_ERROR, "VariableFormula::DE_Compute", "Invalid scale factor mode passed" + FormulaInfoForErrorMessage());
		}

	// in our new system C~[i] = C[i]/f_i
	return y[m_ODEVariableIndex]*scaleFactor;
}


void VariableFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	assert(m_ODEVariableIndex != DE_INVALID_INDEX);
	
	MATRIX_ELEM(jacobian,iEquation,m_ODEVariableIndex) +=  preFactor * m_ODEVariableScaleFactor;
}

void VariableFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	//Simmodel indexing starts at 0, matlab indexing at 1 (!)
	mrOut<<"y("<< _quantityRef.GetODEIndex()+1<<")";
}

bool VariableFormula::UseBracketsForODESystemGeneration ()
{
	return false;
}

void VariableFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	usedVariblesIndices.insert(m_ODEVariableIndex);
}

void VariableFormula::UpdateIndicesOfReferencedVariables()
{
	m_ODEVariableIndex = _quantityRef.GetODEIndex();
}

}//.. end "namespace SimModelNative"