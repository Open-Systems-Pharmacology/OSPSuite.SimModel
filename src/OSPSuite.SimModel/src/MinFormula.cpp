#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/MinFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

MinFormula::MinFormula ()
{
	m_FirstArgument = NULL;
	m_SecondArgument = NULL;
}

MinFormula::~MinFormula ()
{
	if (m_FirstArgument)
		delete m_FirstArgument;

	if (m_SecondArgument)
		delete m_SecondArgument;
}

void MinFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	const char * ERROR_SOURCE = "MinFormula::LoadFromXMLNode";

	// Partial XML
	// <Rate StringToParse="Min(x;y)" >
    //   <Min>
    //     <FirstArgument>
    //       <abstractformula for x.../>
    //     </FirstArgument>
    //     <SecondArgument>
    //       <abstractformula for y.../>
    //     </SecondArgument>
	//   </Min>
	// </Formula>

	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Min));

	XMLNode pFirstArgNode = pNode.GetChildNode(FormulaConstants::FirstArgument);
	XMLNode pSecondArgNode = pNode.GetChildNode(FormulaConstants::SecondArgument);

	if (pFirstArgNode.IsNull() || pSecondArgNode.IsNull())
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Cannot load Min-formula from XML node");

	m_FirstArgument = FormulaFactory::CreateFormula(pFirstArgNode.GetFirstChild().GetNodeName());
	m_FirstArgument->LoadFromXMLNode(pFirstArgNode.GetFirstChild());

	m_SecondArgument = FormulaFactory::CreateFormula(pSecondArgNode.GetFirstChild().GetNodeName());
	m_SecondArgument->LoadFromXMLNode(pSecondArgNode.GetFirstChild());
}

void MinFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Min));

	m_FirstArgument->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::FirstArgument).GetFirstChild(),  sim);
	m_SecondArgument->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::SecondArgument).GetFirstChild(),  sim);
}

void MinFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	assert(m_FirstArgument != NULL);
	assert(m_SecondArgument != NULL);

	m_FirstArgument->SetQuantityReference(quantityReference);
	m_SecondArgument->SetQuantityReference(quantityReference);
}

double MinFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert(m_FirstArgument != NULL);
	assert(m_SecondArgument != NULL);

	double FirstArg = m_FirstArgument->DE_Compute(y, time, scaleFactorMode);
	double SecondArg = m_SecondArgument->DE_Compute(y, time, scaleFactorMode);

	return FirstArg < SecondArg ? FirstArg : SecondArg;
}

bool MinFormula::IsZero(void)
{
	bool forCurrentRunOnly = false;

	if(!m_FirstArgument->IsConstant(forCurrentRunOnly) ||
	   !m_SecondArgument->IsConstant(forCurrentRunOnly))
	   return false;

	double value1 = m_FirstArgument->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
	double value2 = m_SecondArgument->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);

	return (min(value1, value2) == 0.0);
}

void MinFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	assert(m_FirstArgument != NULL);
	assert(m_SecondArgument != NULL);

	double FirstArg = m_FirstArgument->DE_Compute(y, time, USE_SCALEFACTOR);
	double SecondArg = m_SecondArgument->DE_Compute(y, time, USE_SCALEFACTOR);

	if (FirstArg < SecondArg)
		m_FirstArgument->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
	else
		m_SecondArgument->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

void MinFormula::Finalize()
{
	m_FirstArgument->Finalize();
	m_SecondArgument->Finalize();
}

void MinFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	mrOut<<"min(";
	m_FirstArgument->WriteMatlabCode(mrOut);
	mrOut<<",";
	m_SecondArgument->WriteMatlabCode(mrOut);
	mrOut<<")";
}

void MinFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_FirstArgument->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_SecondArgument->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void MinFormula::UpdateIndicesOfReferencedVariables()
{
	m_FirstArgument->UpdateIndicesOfReferencedVariables();
	m_SecondArgument->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"
