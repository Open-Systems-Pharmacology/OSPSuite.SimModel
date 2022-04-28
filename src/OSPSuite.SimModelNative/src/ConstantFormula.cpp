#include "SimModel/ConstantFormula.h"
#include "SimModel/MathHelper.h"
#include "XMLWrapper/XMLNode.h"
#include <assert.h>

namespace SimModelNative
{

	using namespace std;

	ConstantFormula::ConstantFormula()
	{}

	ConstantFormula::ConstantFormula(double value)
	{
		m_Value = value;
	}

	bool ConstantFormula::IsZero()
	{
		return (m_Value == 0.0);
	}

	void ConstantFormula::SetQuantityReference (const QuantityReference & quantityReference)
	{
		//nothing to do for constant formula
	}

	double ConstantFormula::Safe_DE_Compute(const double* y, const double time, ScaleFactorUsageMode scaleFactorMode)
	{
		return m_Value;
	}

	double ConstantFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
	{
		return m_Value;
	}

	void ConstantFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
	{
		//nothing to do for constant formula
	}

	Formula* ConstantFormula::DE_Jacobian(const int iEquation)
	{
		return new ConstantFormula(0.0);
	}

	Formula* ConstantFormula::clone()
	{
		return new ConstantFormula(m_Value);
	}

	Formula * ConstantFormula::RecursiveSimplify()
	{
		// nothing to do
		return this;
	}

	void ConstantFormula::LoadFromXMLNode (const XMLNode & pNode)
	{
		// Partial XML
		//
		// <Constant>3</Constant>

		// Check if the current tag is actually the one we expect
		assert(pNode.HasName(FormulaName::Constant));	
	}

	void SetQuantityReference (const QuantityReference & quantityReference)
	{
		//nothing to do
	}

	void ConstantFormula::SetValue (double p_Value)
	{
		m_Value=p_Value;
	}

	void ConstantFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
	{
		// Check if the current tag is actually the one we expect
		assert(pNode.HasName(FormulaName::Constant));	
		m_Value = pNode.GetValue(MathHelper::GetNaN());
	}

	bool ConstantFormula::IsConstant(bool forCurrentRunOnly)
	{
		return true;
	}

	vector <double> ConstantFormula::SwitchTimePoints()
	{
		return vector <double> ();
	}

	void ConstantFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
	{
		mrOut<<MathHelper::ToString(m_Value);
	}

	void ConstantFormula::WriteFormulaCppCode(std::ostream & mrOut)
	{
		mrOut << MathHelper::ToString(m_Value);
	}

	bool ConstantFormula::UseBracketsForODESystemGeneration ()
	{
		return (m_Value < 0.0);
	}

	void ConstantFormula::AppendUsedVariables(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
	{
		//nothing to do so far
	}

	void ConstantFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
	{
		//nothing to do so far
	}

	void ConstantFormula::UpdateIndicesOfReferencedVariables()
	{
		//nothing to do so far
	}

}//.. end "namespace SimModelNative"
