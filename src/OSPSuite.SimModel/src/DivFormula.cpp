#include "SimModel/DivFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/DiffFormula.h"
#include "SimModel/ProductFormula.h"
#include "SimModel/PowerFormula.h"
#include "SimModel/ConstantFormula.h"
#include <assert.h>

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

Formula* DivFormula::DE_Jacobian(const int iEquation)
{
	DivFormula* f = new DivFormula();

	set<int> dep; set<int> empty;
	m_DenominatorFormula->AppendUsedVariables(dep, empty);

	// check for constant denominator
	if (dep.count(iEquation)==0) {
		f->m_NumeratorFormula = m_NumeratorFormula->DE_Jacobian(iEquation);
		f->m_DenominatorFormula = m_DenominatorFormula->clone();
		return f;
	}

	// (dnum * den - num * dden) / den^2
	ProductFormula* den = new ProductFormula();
	DiffFormula* num = new DiffFormula();
	ProductFormula* p1 = new ProductFormula();
	ProductFormula* p2 = new ProductFormula();

	Formula *m1[2] = { m_NumeratorFormula->DE_Jacobian(iEquation), m_DenominatorFormula->clone() };
	p1->setFormula(2, m1);

	Formula *m2[2] = { m_NumeratorFormula->clone(), m_DenominatorFormula->DE_Jacobian(iEquation) };
	p2->setFormula(2, m2);

	num->setFormula(p1, p2);

	Formula *m3[2] = { m_DenominatorFormula->clone(), m_DenominatorFormula->clone() };
	den->setFormula(2, m3);

	f->m_NumeratorFormula = num;
	f->m_DenominatorFormula = den;

	return f;
}

Formula* DivFormula::clone()
{
	DivFormula* f = new DivFormula();
	f->m_NumeratorFormula = m_NumeratorFormula->clone();
	f->m_DenominatorFormula = m_DenominatorFormula->clone();
	return f;
}

Formula * DivFormula::RecursiveSimplify()
{
	m_NumeratorFormula = m_NumeratorFormula->RecursiveSimplify();
	m_DenominatorFormula = m_DenominatorFormula->RecursiveSimplify();
	if (m_NumeratorFormula->IsConstant(CONSTANT_CURRENT_RUN))
	{
		if (m_NumeratorFormula->IsZero())
		{
			ConstantFormula * f = new ConstantFormula(0.0);
			delete this;
			return f;
		}
		if (m_DenominatorFormula->IsConstant(CONSTANT_CURRENT_RUN))
		{
			ConstantFormula * f = new ConstantFormula(DE_Compute(NULL, 0.0, USE_SCALEFACTOR));
			delete this;
			return f;
		}
	}

	return this;
}

void DivFormula::setFormula(Formula* numeratorFormula, Formula* denominatorFormula)
{
	if (m_NumeratorFormula != NULL) delete m_NumeratorFormula;
	if (m_DenominatorFormula != NULL) delete m_DenominatorFormula;

	m_NumeratorFormula = numeratorFormula;
	m_DenominatorFormula = denominatorFormula;
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

void DivFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_NumeratorFormula->WriteCppCode(mrOut);
	mrOut << "/";
	m_DenominatorFormula->WriteCppCode(mrOut);
}

void DivFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_NumeratorFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	m_DenominatorFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void DivFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	m_NumeratorFormula->AppendUsedParameters(usedParameterIDs);
	m_DenominatorFormula->AppendUsedParameters(usedParameterIDs);
}

void DivFormula::UpdateIndicesOfReferencedVariables()
{
	m_NumeratorFormula->UpdateIndicesOfReferencedVariables();
	m_DenominatorFormula->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"