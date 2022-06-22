#include "SimModel/DiffFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/ConstantFormula.h"
#include <assert.h>

namespace SimModelNative
{

using namespace std;

DiffFormula::DiffFormula ()
{
	m_MinuendFormula=NULL;
	m_SubtrahendFormula=NULL;
}

DiffFormula::~DiffFormula ()
{
	if (m_MinuendFormula!=NULL) delete m_MinuendFormula;
	if (m_SubtrahendFormula!=NULL) delete m_SubtrahendFormula;
}

bool DiffFormula::IsZero(void)
{
	bool forCurrentRunOnly = false;

	if(!m_MinuendFormula->IsConstant(forCurrentRunOnly) ||
	   !m_SubtrahendFormula->IsConstant(forCurrentRunOnly))
	   return false;

	double minuendValue = m_MinuendFormula->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
	double subtrahendValue = m_SubtrahendFormula->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);

	return (minuendValue == subtrahendValue);
}

void DiffFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Diff));

	//Get Numerator and Denominator Nodes
	XMLNode pMinuendNode = pNode.GetChildNode(FormulaConstants::Minuend);
	XMLNode pSubtrahendNode = pNode.GetChildNode(FormulaConstants::Subtrahend);

	//Set Formulas according to nodes  
	m_MinuendFormula =  FormulaFactory::CreateFormula(pMinuendNode.GetFirstChild().GetNodeName());
	m_MinuendFormula->LoadFromXMLNode(pMinuendNode.GetFirstChild());

	m_SubtrahendFormula =  FormulaFactory::CreateFormula(pSubtrahendNode.GetFirstChild().GetNodeName());
	m_SubtrahendFormula->LoadFromXMLNode(pSubtrahendNode.GetFirstChild());
}

void DiffFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Diff));

	//Get Numerator and Denominator Nodes
	XMLNode pMinuendNode = pNode.GetChildNode(FormulaConstants::Minuend);
	XMLNode pSubtrahendNode = pNode.GetChildNode(FormulaConstants::Subtrahend);

	//Set Formulas according to nodes  
	m_MinuendFormula->XMLFinalizeInstance(pMinuendNode.GetFirstChild(), sim);
	m_SubtrahendFormula->XMLFinalizeInstance(pSubtrahendNode.GetFirstChild(), sim);	
}

void DiffFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	m_MinuendFormula->SetQuantityReference(quantityReference);
	m_SubtrahendFormula->SetQuantityReference(quantityReference);
}

double DiffFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	// Abbreviation of values
	const double dMin =  m_MinuendFormula->DE_Compute(y, time, scaleFactorMode);
	const double dSub =  m_SubtrahendFormula->DE_Compute(y, time, scaleFactorMode);

	return  dMin-dSub;
}

void DiffFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	m_MinuendFormula->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
	m_SubtrahendFormula->DE_Jacobian(jacobian, y, time, iEquation, -preFactor);
}

Formula* DiffFormula::DE_Jacobian(const int iEquation)
{
	DiffFormula* f = new DiffFormula();
	f->m_MinuendFormula = m_MinuendFormula->DE_Jacobian(iEquation);
	f->m_SubtrahendFormula = m_SubtrahendFormula->DE_Jacobian(iEquation);
	return f;
}

Formula* DiffFormula::clone()
{
	DiffFormula* f = new DiffFormula();
	f->m_MinuendFormula = m_MinuendFormula->clone();
	f->m_SubtrahendFormula = m_SubtrahendFormula->clone();
	return f;
}

Formula * DiffFormula::RecursiveSimplify()
{
	m_MinuendFormula = m_MinuendFormula->RecursiveSimplify();
	m_SubtrahendFormula = m_SubtrahendFormula->RecursiveSimplify();
	bool minConst = m_MinuendFormula->IsConstant(CONSTANT_CURRENT_RUN);
	bool subConst = m_SubtrahendFormula->IsConstant(CONSTANT_CURRENT_RUN);
	if (minConst && subConst)
	{
		ConstantFormula * f = new ConstantFormula(DE_Compute(NULL, 0.0, USE_SCALEFACTOR));
		delete this;
		return f;
	}
	else if (subConst && m_SubtrahendFormula->IsZero())
	{
		Formula * f = m_MinuendFormula;
		m_MinuendFormula = NULL;  // prevent destructor to delete it
		delete this;
		return f;
	}
	
	return this;
}

void DiffFormula::setFormula(Formula* minuend, Formula* subrahend)
{
	if (m_MinuendFormula != NULL) delete m_MinuendFormula;
	if (m_SubtrahendFormula != NULL) delete m_SubtrahendFormula;

	m_MinuendFormula = minuend;
	m_SubtrahendFormula = subrahend;
}

void DiffFormula::Finalize()
{
	m_MinuendFormula->Finalize();
	m_SubtrahendFormula->Finalize();
}

void DiffFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_MinuendFormula->WriteMatlabCode(mrOut);
	mrOut<<"-";
	m_SubtrahendFormula->WriteMatlabCode(mrOut);
}

void DiffFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_MinuendFormula->WriteCppCode(mrOut);
	mrOut << "-";
	m_SubtrahendFormula->WriteCppCode(mrOut);
}

void DiffFormula::AppendUsedVariables(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
{
	m_MinuendFormula->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
	m_SubtrahendFormula->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
}

void DiffFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	m_MinuendFormula->AppendUsedParameters(usedParameterIDs);
	m_SubtrahendFormula->AppendUsedParameters(usedParameterIDs);
}

void DiffFormula::UpdateIndicesOfReferencedVariables()
{
	m_MinuendFormula->UpdateIndicesOfReferencedVariables();
	m_SubtrahendFormula->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"