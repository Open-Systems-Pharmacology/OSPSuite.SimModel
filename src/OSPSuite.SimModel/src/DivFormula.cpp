#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/DivFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include <assert.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

DivFormula::~DivFormula ()
{
	if (m_DenominatorFormula!=NULL) delete m_DenominatorFormula;
	if (m_NumeratorFormula!=NULL) delete m_NumeratorFormula;
}

DivFormula::DivFormula ()
{
	m_DenominatorFormula = NULL;
	m_NumeratorFormula = NULL;
}

bool DivFormula::IsZero(void)
{
	return m_NumeratorFormula->IsZero();
}

void DivFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Div));

	//Get Numerator and Denominator Nodes
	XMLNode pNumeratorNode = pNode.GetChildNode(FormulaConstants::Numerator);
	XMLNode pDenominatorNode = pNode.GetChildNode(FormulaConstants::Denominator);

	//Set Formulas according to nodes  
	m_NumeratorFormula = FormulaFactory::CreateFormula(pNumeratorNode.GetFirstChild().GetNodeName());
	m_NumeratorFormula->LoadFromXMLNode(pNumeratorNode.GetFirstChild());

	m_DenominatorFormula = FormulaFactory::CreateFormula(pDenominatorNode.GetFirstChild().GetNodeName());
	m_DenominatorFormula->LoadFromXMLNode(pDenominatorNode.GetFirstChild());
}

void DivFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Div));

	//Get Numerator and Denominator Nodes
	XMLNode pNumeratorNode = pNode.GetChildNode(FormulaConstants::Numerator);
	XMLNode pDenominatorNode = pNode.GetChildNode(FormulaConstants::Denominator);

	//Set Formulas according to nodes  
	m_NumeratorFormula->XMLFinalizeInstance(pNumeratorNode.GetFirstChild(), sim);	
	m_DenominatorFormula->XMLFinalizeInstance(pDenominatorNode.GetFirstChild(), sim);
}

void DivFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	m_DenominatorFormula->SetQuantityReference(quantityReference);
	m_NumeratorFormula->SetQuantityReference(quantityReference);
}

double DivFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	// Abbreviation of values
	const double dNum =  m_NumeratorFormula->DE_Compute(y, time, scaleFactorMode);
	const double dDen =  m_DenominatorFormula->DE_Compute(y, time, scaleFactorMode);

	return dNum/dDen;
}

void DivFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	double f1 = m_DenominatorFormula->DE_Compute(y, time, USE_SCALEFACTOR);
	double f2 = - m_NumeratorFormula->DE_Compute(y, time, USE_SCALEFACTOR)/(f1*f1);
		
	m_NumeratorFormula->DE_Jacobian(jacobian, y, time, iEquation, preFactor/f1);

	if (f2 == 0.0)
		return;

	m_DenominatorFormula->DE_Jacobian(jacobian, y, time, iEquation, preFactor*f2);
}

void DivFormula::Finalize()
{
	m_NumeratorFormula->Finalize();
	m_DenominatorFormula->Finalize();
}

void DivFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_NumeratorFormula->WriteMatlabCode(mrOut);
	mrOut<<"/";
	m_DenominatorFormula->WriteMatlabCode(mrOut);
}

void DivFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_NumeratorFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_DenominatorFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void DivFormula::UpdateIndicesOfReferencedVariables()
{
	m_NumeratorFormula->UpdateIndicesOfReferencedVariables();
	m_DenominatorFormula->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"