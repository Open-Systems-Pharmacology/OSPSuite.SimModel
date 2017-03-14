#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/MaxFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

MaxFormula::MaxFormula ()
{
	m_FirstArgument = NULL;
	m_SecondArgument = NULL;
}

MaxFormula::~MaxFormula ()
{
	if (m_FirstArgument)
		delete m_FirstArgument;

	if (m_SecondArgument)
		delete m_SecondArgument;
}

void MaxFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	const char * ERROR_SOURCE = "MaxFormula::LoadFromXMLNode";

	// Partial XML
	// <Rate StringToParse="Max(x;y)" >
    //   <Min>
    //     <FirstArgument>
    //       <Variable>x</Variable>
    //     </FirstArgument>
    //     <SecondArgument>
    //       <Variable>y</Variable>
    //     </SecondArgument>
	//   </Min>
	// </Formula>

	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Max));

	XMLNode pFirstArgNode = pNode.GetChildNode(FormulaConstants::FirstArgument);
	XMLNode pSecondArgNode = pNode.GetChildNode(FormulaConstants::SecondArgument);

	if (pFirstArgNode.IsNull() || pSecondArgNode.IsNull())
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Cannot load Max-formula from XML node");

	m_FirstArgument = FormulaFactory::CreateFormula(pFirstArgNode.GetFirstChild().GetNodeName());
	m_FirstArgument->LoadFromXMLNode(pFirstArgNode.GetFirstChild());

	m_SecondArgument = FormulaFactory::CreateFormula(pSecondArgNode.GetFirstChild().GetNodeName());
	m_SecondArgument->LoadFromXMLNode(pSecondArgNode.GetFirstChild());
}

void MaxFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Max));

	m_FirstArgument->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::FirstArgument).GetFirstChild(),  sim);
	m_SecondArgument->XMLFinalizeInstance(pNode.GetChildNode(FormulaConstants::SecondArgument).GetFirstChild(),  sim);
}

void MaxFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	assert(m_FirstArgument != NULL);
	assert(m_SecondArgument != NULL);

	m_FirstArgument->SetQuantityReference(quantityReference);
	m_SecondArgument->SetQuantityReference(quantityReference);

}

double MaxFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert(m_FirstArgument != NULL);
	assert(m_SecondArgument != NULL);

	double FirstArg = m_FirstArgument->DE_Compute(y, time, scaleFactorMode);
	double SecondArg = m_SecondArgument->DE_Compute(y, time, scaleFactorMode);

	return FirstArg > SecondArg ? FirstArg : SecondArg;
}

bool MaxFormula::IsZero(void)
{
	bool forCurrentRunOnly = false;

	if(!m_FirstArgument->IsConstant(forCurrentRunOnly) ||
	   !m_SecondArgument->IsConstant(forCurrentRunOnly))
	   return false;

	double value1 = m_FirstArgument->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
	double value2 = m_SecondArgument->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);

	return (max(value1, value2) == 0.0);
}

void MaxFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	assert(m_FirstArgument != NULL);
	assert(m_SecondArgument != NULL);

	double FirstArg = m_FirstArgument->DE_Compute(y, time, USE_SCALEFACTOR);
	double SecondArg = m_SecondArgument->DE_Compute(y, time, USE_SCALEFACTOR);

	if (FirstArg > SecondArg)
		m_FirstArgument->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
	else
		m_SecondArgument->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

void MaxFormula::Finalize()
{
	m_FirstArgument->Finalize();
	m_SecondArgument->Finalize();
}

void MaxFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	mrOut<<"max(";
	m_FirstArgument->WriteMatlabCode(mrOut);
	mrOut<<",";
	m_SecondArgument->WriteMatlabCode(mrOut);
	mrOut<<")";
}

void MaxFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_FirstArgument->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_SecondArgument->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void MaxFormula::UpdateIndicesOfReferencedVariables()
{
	m_FirstArgument->UpdateIndicesOfReferencedVariables();
	m_SecondArgument->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"
