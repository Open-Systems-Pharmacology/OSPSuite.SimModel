#include "SimModel/MaxFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/ConstantFormula.h"
#include "SimModel/IfFormula.h"
#include "SimModel/BooleanFormula.h"
#include <assert.h>

#include "SimModel/MathHelper.h"

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

	auto firstArgument = m_FirstArgument->DE_Compute(y, time, scaleFactorMode);
	auto secondArgument = m_SecondArgument->DE_Compute(y, time, scaleFactorMode);

	if (isnan(firstArgument) || isnan(secondArgument))
		return MathHelper::GetNaN();

	return firstArgument > secondArgument ? firstArgument : secondArgument;
}

bool MaxFormula::IsZero()
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

	auto firstArgument = m_FirstArgument->DE_Compute(y, time, USE_SCALEFACTOR);
	auto secondArgument = m_SecondArgument->DE_Compute(y, time, USE_SCALEFACTOR);

	if (isnan(firstArgument) || isnan(secondArgument))
		return; //TODO is this ok? or should we throw an error instead?

	if (firstArgument > secondArgument)
		m_FirstArgument->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
	else
		m_SecondArgument->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

Formula* MaxFormula::DE_Jacobian(const int iEquation)
{
	auto f = new IfFormula();
	auto g = new GreaterFormula();

	g->setFormula(m_FirstArgument->clone(), m_SecondArgument->clone());
	f->setFormula(g, m_FirstArgument->DE_Jacobian(iEquation), m_SecondArgument->DE_Jacobian(iEquation));

	return f;
}

Formula* MaxFormula::clone()
{
	auto f = new MaxFormula();
	f->m_FirstArgument = m_FirstArgument->clone();
	f->m_SecondArgument = m_SecondArgument->clone();
	return f;
}

Formula * MaxFormula::RecursiveSimplify()
{
	m_FirstArgument = m_FirstArgument->RecursiveSimplify();
	m_SecondArgument = m_SecondArgument->RecursiveSimplify();
	if (m_FirstArgument->IsConstant(CONSTANT_CURRENT_RUN) && m_SecondArgument->IsConstant(CONSTANT_CURRENT_RUN))
	{
		auto f = new ConstantFormula(DE_Compute(NULL, 0.0, USE_SCALEFACTOR));
		delete this;
		return f;
	}

	return this;
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

void MaxFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	// TODO: change to ? operator to prevent template problems?
	mrOut << "max(";
	m_FirstArgument->WriteCppCode(mrOut);
	mrOut << ",";
	m_SecondArgument->WriteCppCode(mrOut);
	mrOut << ")";
}

void MaxFormula::AppendUsedVariables(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
{
	m_FirstArgument->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
	m_SecondArgument->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
}

void MaxFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	m_FirstArgument->AppendUsedParameters(usedParameterIDs);
	m_SecondArgument->AppendUsedParameters(usedParameterIDs);
}

void MaxFormula::UpdateIndicesOfReferencedVariables()
{
	m_FirstArgument->UpdateIndicesOfReferencedVariables();
	m_SecondArgument->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"
