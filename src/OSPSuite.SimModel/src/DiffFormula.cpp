#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/DiffFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

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

void DiffFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_MinuendFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_SubtrahendFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void DiffFormula::UpdateIndicesOfReferencedVariables()
{
	m_MinuendFormula->UpdateIndicesOfReferencedVariables();
	m_SubtrahendFormula->UpdateIndicesOfReferencedVariables();
}


}//.. end "namespace SimModelNative"