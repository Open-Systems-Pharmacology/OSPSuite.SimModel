#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/BooleanFormula.h"
#include "SimModel/FormulaFactory.h"
#include <assert.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

//-------------------------------------------------------------------
//---- Boolean function formula (base for all boolean  formulas)
//-------------------------------------------------------------------

BooleanFormula::BooleanFormula ()
{
	m_FirstOperandFormula=NULL;
	m_SecondOperandFormula=NULL;
}

BooleanFormula::~BooleanFormula ()
{
	if (m_SecondOperandFormula!=NULL) delete m_SecondOperandFormula;
	if (m_FirstOperandFormula!=NULL) delete m_FirstOperandFormula;
}

bool BooleanFormula::IsZero(void)
{
	//---- check if both operands are constant for all simulation runs
	bool forCurrentRunOnly = false;

	if (!m_FirstOperandFormula->IsConstant(forCurrentRunOnly))
		return false;

	//(2nd operand is optional for some boolean functions)
	if (m_SecondOperandFormula && !m_SecondOperandFormula->IsConstant(forCurrentRunOnly))
		return false;

	return (DE_Compute(NULL, 0.0, USE_SCALEFACTOR) == 0.0);
}

void BooleanFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	//Get Nodes 
	XMLNode pFirstOperandNode = pNode.GetChildNode(FormulaConstants::FirstOperand);
	XMLNode pSecondOperandNode = pNode.GetChildNode(FormulaConstants::SecondOperand);
	
	//Set Formulas according to nodes
	m_FirstOperandFormula =  FormulaFactory::CreateFormula(pFirstOperandNode.GetFirstChild().GetNodeName());
	m_FirstOperandFormula->LoadFromXMLNode(pFirstOperandNode.GetFirstChild());
	
	//second operand is not mandatory (e.g. NOT Formula)
	if (!pSecondOperandNode.IsNull())
	{
		m_SecondOperandFormula =  FormulaFactory::CreateFormula(pSecondOperandNode.GetFirstChild().GetNodeName());
		m_SecondOperandFormula->LoadFromXMLNode(pSecondOperandNode.GetFirstChild());
	}
}

void BooleanFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{

	//Get Numerator and Denominator Nodes
	XMLNode pFirstOperandNode = pNode.GetChildNode(FormulaConstants::FirstOperand);
	XMLNode pSecondOperandNode = pNode.GetChildNode(FormulaConstants::SecondOperand);

	//Set Formulas according to nodes  
	m_FirstOperandFormula->XMLFinalizeInstance(pFirstOperandNode.GetFirstChild(), sim);
	
	//second operand is not mandatory (e.g. NOT Formula)
	if (!pSecondOperandNode.IsNull())
		m_SecondOperandFormula->XMLFinalizeInstance(pSecondOperandNode.GetFirstChild(), sim);
}

void BooleanFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	m_FirstOperandFormula->SetQuantityReference(quantityReference);
	
	//second operand is not mandatory (e.g. NOT Formula)
	if (m_SecondOperandFormula)
		m_SecondOperandFormula->SetQuantityReference(quantityReference);
}

void BooleanFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	m_FirstOperandFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);

	//second operand is not mandatory (e.g. NOT Formula)
	if (m_SecondOperandFormula)
		m_SecondOperandFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void BooleanFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	//no contribution to jacobian matrix by boolean functions
}

void BooleanFormula::Finalize()
{
	assert(m_FirstOperandFormula!=NULL);
	m_FirstOperandFormula->Finalize();

	if (m_SecondOperandFormula)
		m_SecondOperandFormula->Finalize();
}

//check if the formula has form "Time OP ConstFormula" or "ConstFormula OP Time",
// where OP is one of boolean operations ==, >=, ... and
//<ConstFormula> is a constant formula
vector <double> BooleanFormula::SwitchTimePointFromComparisonFormula()
{
	vector <double> switchTimePoints;

	Formula * formulaWithSwitchTimePoints = NULL;

	if (m_FirstOperandFormula->IsTime())
	{
		//comparison formula is "Time OP <Formula>", where OP is boolean operator
		//(e.g. "Time == P1+P2"). In this case, try to get potential
		//switch time point from the second operand
		formulaWithSwitchTimePoints = m_SecondOperandFormula;
	}
	else if (m_SecondOperandFormula->IsTime())
	{
		//comparison formula is "<Formula> OP Time", where OP is boolean operator
		//(e.g. "P1+P2 == TIME"). In this case, try to get potential
		//switch time point from the first operand
		formulaWithSwitchTimePoints = m_FirstOperandFormula;
	}

	if (formulaWithSwitchTimePoints != NULL)
	{
		//try to evaluate formula without variables. If formula is dependent
		//on the ODE variables it will fail and no switch time point will be added
		//otherwise, the value of the formula will be added as potential switch time point
		try
		{
			double switchTime = formulaWithSwitchTimePoints->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
			switchTimePoints.push_back(switchTime);
		}
		catch(...)
		{
			//formula is ODE-variable(s) dependent. Do nothing
		}
	}

	return switchTimePoints;
}

//returns (potential) switch time points for AND/OR/NOT-Rates
//will be overwritten by comparison rates (>, >=, ==, ...)
vector <double> BooleanFormula::SwitchTimePoints()
{
	vector <double> firstOpTimePoints, secondOpTimePoints;
	
	firstOpTimePoints = m_FirstOperandFormula->SwitchTimePoints();

	if (m_SecondOperandFormula)
		secondOpTimePoints = m_SecondOperandFormula->SwitchTimePoints();

	for(unsigned int i = 0; i<secondOpTimePoints.size(); i++)
		firstOpTimePoints.push_back(secondOpTimePoints[i]);

	return firstOpTimePoints;
}

void BooleanFormula::UpdateIndicesOfReferencedVariables()
{
	m_FirstOperandFormula->UpdateIndicesOfReferencedVariables();

	if (m_SecondOperandFormula)
		m_SecondOperandFormula->UpdateIndicesOfReferencedVariables();
}

//-------------------------------------------------------------------
//---- And (A&&B)
//-------------------------------------------------------------------

double AndFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert (m_SecondOperandFormula != NULL); //first operand is always set
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) && m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void AndFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" & ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

bool AndFormula::IsZero(void)
{
	bool forCurrentRunOnly = false;

	if (m_FirstOperandFormula->IsConstant(forCurrentRunOnly))
	{
		if (m_FirstOperandFormula->DE_Compute(NULL, 0.0, USE_SCALEFACTOR) == 0.0)
			return true;
	}

	if (m_SecondOperandFormula->IsConstant(forCurrentRunOnly))
	{
		if (m_SecondOperandFormula->DE_Compute(NULL, 0.0, USE_SCALEFACTOR) == 0.0)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------
//---- Equal (A==B)
//-------------------------------------------------------------------

double EqualFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) == m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void EqualFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" == ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

vector <double> EqualFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

//-------------------------------------------------------------------
//---- GreaterEqual (A>=B)
//-------------------------------------------------------------------

double GreaterEqualFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) >= m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void GreaterEqualFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" >= ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

vector <double> GreaterEqualFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

//-------------------------------------------------------------------
//---- Greater (A>B)
//-------------------------------------------------------------------

double GreaterFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) > m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void GreaterFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" > ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

vector <double> GreaterFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

//-------------------------------------------------------------------
//---- LessEqual (A<=B)
//-------------------------------------------------------------------

double LessEqualFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) <= m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void LessEqualFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" <= ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

vector <double> LessEqualFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

//-------------------------------------------------------------------
//---- Less (A<=B)
//-------------------------------------------------------------------

double LessFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) < m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void LessFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" < ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

vector <double> LessFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

//-------------------------------------------------------------------
//---- Not (!A)
//-------------------------------------------------------------------

double NotFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  !m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode);
}

void NotFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	mrOut<<"~";
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
}

//-------------------------------------------------------------------
//---- Or (A||B)
//-------------------------------------------------------------------

double OrFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert (m_SecondOperandFormula != NULL); //first operand is always set
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) || m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void OrFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" | ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

//-------------------------------------------------------------------
//---- Unequal (A<>B)
//-------------------------------------------------------------------

double UnequalFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return  (m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode) != m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode));
}

void UnequalFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" ~= ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

vector <double> UnequalFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

}//.. end "namespace SimModelNative"