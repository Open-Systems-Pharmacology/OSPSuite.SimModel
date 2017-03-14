#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/IfFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

IfFormula::IfFormula ()
{
	m_IfStatement = NULL;
	m_ThenStatement = NULL;
	m_ElseStatement = NULL;
}

IfFormula::~IfFormula ()
{
	if (m_IfStatement)
		delete m_IfStatement;

	if (m_ThenStatement)
		delete m_ThenStatement;

	if (m_ElseStatement)
		delete m_ElseStatement;
}

void IfFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	const char * ERROR_SOURCE = "IfFormula::LoadFromXMLNode";

	// Partial XML
	//<Formula StringToParse="x ? y : z" >
    //  <IF>
    //    <IfStatement>
    //      <abstractformula for x ...>
    //    </IfStatement>
    //    <ThenStatement>
    //      <abstractformula for y ...>
    //    </ThenStatement>
    //    <ElseStatement>
    //      <abstractformula for z ...>
    //    </ElseStatement>
	//  </IF>
	//</Formula>
	
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::IF));

	XMLNode pIfNode = pNode.GetChildNode(FormulaConstants::IfStatement);
	XMLNode pThenNode = pNode.GetChildNode(FormulaConstants::ThenStatement);
	XMLNode pElseNode = pNode.GetChildNode(FormulaConstants::ElseStatement);

	if (pIfNode.IsNull() || pThenNode.IsNull() || pElseNode.IsNull())
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Cannot load IF-formula from XML node");
		
	m_IfStatement = FormulaFactory::CreateFormula(pIfNode.GetFirstChild().GetNodeName());
	m_IfStatement->LoadFromXMLNode(pIfNode.GetFirstChild());

	m_ThenStatement = FormulaFactory::CreateFormula(pThenNode.GetFirstChild().GetNodeName());
	m_ThenStatement->LoadFromXMLNode(pThenNode.GetFirstChild());

	m_ElseStatement = FormulaFactory::CreateFormula(pElseNode.GetFirstChild().GetNodeName());
	m_ElseStatement->LoadFromXMLNode(pElseNode.GetFirstChild());
}

void IfFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::IF));
	
	m_IfStatement->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::IfStatement).GetFirstChild(),  sim);
	m_ThenStatement->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::ThenStatement).GetFirstChild(),  sim);
	m_ElseStatement->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::ElseStatement).GetFirstChild(),  sim);
}

void IfFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	assert(m_IfStatement != NULL);
	assert(m_ThenStatement != NULL);
	assert(m_ElseStatement != NULL);
	
	m_IfStatement->SetQuantityReference(quantityReference);
	m_ThenStatement->SetQuantityReference(quantityReference);
	m_ElseStatement->SetQuantityReference(quantityReference);
}


double IfFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert(m_IfStatement != NULL);
	assert(m_ThenStatement != NULL);
	assert(m_ElseStatement != NULL);

	if (m_IfStatement->DE_Compute(y, time, scaleFactorMode) == 1)
		return m_ThenStatement->DE_Compute(y, time, scaleFactorMode);
	else
		return m_ElseStatement->DE_Compute(y, time, scaleFactorMode);
}

vector <double> IfFormula::SwitchTimePoints()
{
	vector <double> switchTimePoints;

	bool forCurrentRunOnly = true;

	if (!m_IfStatement->IsConstant(forCurrentRunOnly))
	{
		//---- if-statement not constant. In this case, return potential switch time points
		//from both then- and else-statements
		vector <double> switchTimePoints1, switchTimePoints2;
		switchTimePoints1 = m_ThenStatement->SwitchTimePoints();
		switchTimePoints2 = m_ElseStatement->SwitchTimePoints();

		switchTimePoints1.insert(switchTimePoints1.begin(), switchTimePoints2.begin(), switchTimePoints2.end());

		return switchTimePoints1;
	}

	//---- if statement is constant
	if (m_IfStatement->DE_Compute(NULL, 0.0, USE_SCALEFACTOR) == 1)
	{
		return m_ThenStatement->SwitchTimePoints();
	}
	else
	{
		return m_ElseStatement->SwitchTimePoints();
	}
}

bool IfFormula::IsZero(void)
{
	bool forCurrentRunOnly = false;

	if(!m_IfStatement->IsConstant(forCurrentRunOnly))
	{
		//---- if-statement not constant. In this case, formula is constant zero
		//if both then- and else-statements are constant zero
		return (m_ThenStatement->IsZero() && m_ElseStatement->IsZero());
	}

	//---- if statement is constant
	if (m_IfStatement->DE_Compute(NULL, 0.0, USE_SCALEFACTOR) == 1)
	{
		return m_ThenStatement->IsZero();
	}
	else
	{
		return m_ElseStatement->IsZero();
	}
}

void IfFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	assert(m_IfStatement != NULL);
	assert(m_ThenStatement != NULL);
	assert(m_ElseStatement != NULL);

	if (m_IfStatement->DE_Compute(y, time, USE_SCALEFACTOR) == 1)
		m_ThenStatement->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
	else
		m_ElseStatement->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

void IfFormula::Finalize()
{
	m_IfStatement->Finalize();
	m_ThenStatement->Finalize();
	m_ElseStatement->Finalize();
}

void IfFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	mrOut<<"IIf(";
	m_IfStatement->WriteMatlabCode(mrOut);
	mrOut<<",";
	m_ThenStatement->WriteMatlabCode(mrOut);
	mrOut<<",";
	m_ElseStatement->WriteMatlabCode(mrOut);
	mrOut<<")";
}

void IfFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_IfStatement->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_ThenStatement->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_ElseStatement->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void IfFormula::UpdateIndicesOfReferencedVariables()
{
	m_IfStatement->UpdateIndicesOfReferencedVariables();
	m_ThenStatement->UpdateIndicesOfReferencedVariables();
	m_ElseStatement->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"