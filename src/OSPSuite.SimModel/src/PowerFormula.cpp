#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/PowerFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

PowerFormula::PowerFormula ()
{
	m_BaseFormula = NULL;
	m_ExponentFormula = NULL;
}

PowerFormula::~PowerFormula ()
{
	if (m_BaseFormula!=NULL) delete m_BaseFormula;
	if (m_ExponentFormula!=NULL) delete m_ExponentFormula;
}

bool PowerFormula::IsZero(void)
{
	return false;
}

void PowerFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Power));

	//Get Numerator and Denominator Nodes
	XMLNode pBaseNode = pNode.GetChildNode(FormulaConstants::Base);
	XMLNode pExponentNode = pNode.GetChildNode(FormulaConstants::Exponent);

	//Set Formulas according to nodes  
	m_BaseFormula =  FormulaFactory::CreateFormula(pBaseNode.GetFirstChild().GetNodeName());
	m_BaseFormula->LoadFromXMLNode(pBaseNode.GetFirstChild());

	m_ExponentFormula = FormulaFactory::CreateFormula(pExponentNode.GetFirstChild().GetNodeName());
	m_ExponentFormula->LoadFromXMLNode(pExponentNode.GetFirstChild());
}

void PowerFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Power));

	//Get Numerator and Denominator Nodes
	XMLNode pBaseNode = pNode.GetChildNode(FormulaConstants::Base);
	XMLNode pExponentNode = pNode.GetChildNode(FormulaConstants::Exponent);
		
	//Set Formulas according to nodes  
	m_BaseFormula->XMLFinalizeInstance(pBaseNode.GetFirstChild(), sim);
	m_ExponentFormula->XMLFinalizeInstance(pExponentNode.GetFirstChild(), sim);
}

void PowerFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	m_BaseFormula->SetQuantityReference(quantityReference);
	m_ExponentFormula->SetQuantityReference(quantityReference);
}

double PowerFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	const double dBase =  m_BaseFormula->DE_Compute(y, time, scaleFactorMode);
	const double dExp =  m_ExponentFormula->DE_Compute(y, time, scaleFactorMode);

	return pow(dBase, dExp);
}

void PowerFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	// jacobian is the Jacobian of the RHS of the ODE system
	// y is the solution at the current time
	// time is the current time
	// prefactor is a multiplicative factor (to take product rule into account)
	// iEquation is the index of the equation number we're looking at

	//Formula is here (R)=m_Base ^ m_Exp
	//Derivative (R)' = ((m_Exp)'*ln(m_Base) + m_Exp.(m_Base)'/m_Base) * R 
	//Derivative (R)' = (ln(m_Base)*R)*(m_Exp)' + (m_Exp*R/m_Base) *(m_Base)'
	
	double R 	  =	DE_Compute(y, time, USE_SCALEFACTOR);
	double m_Exp  =	m_ExponentFormula->DE_Compute(y, time, USE_SCALEFACTOR);
	double m_Base =	m_BaseFormula->DE_Compute(y, time, USE_SCALEFACTOR);
	
	if (m_Base == 0.0)
		return;
	
	m_BaseFormula->DE_Jacobian(jacobian, y, time, iEquation, preFactor*m_Exp/m_Base*R);
	m_ExponentFormula->DE_Jacobian(jacobian, y, time, iEquation, preFactor * log(m_Base)*R);
}

void PowerFormula::Finalize()
{
	m_BaseFormula->Finalize();
	m_ExponentFormula->Finalize();
}

void PowerFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_BaseFormula->WriteMatlabCode(mrOut);
	mrOut<<"^";
	m_ExponentFormula->WriteMatlabCode(mrOut);
}

void PowerFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_BaseFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_ExponentFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void PowerFormula::UpdateIndicesOfReferencedVariables()
{
	m_BaseFormula->UpdateIndicesOfReferencedVariables();
	m_ExponentFormula->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"