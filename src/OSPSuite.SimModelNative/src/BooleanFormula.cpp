#include "SimModel/BooleanFormula.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/ConstantFormula.h"
#include "SimModel/ParameterFormula.h"
#include <assert.h>

#include "SimModel/MathHelper.h"

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

void BooleanFormula::AppendUsedVariables(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
{
	m_FirstOperandFormula->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);

	//second operand is not mandatory (e.g. NOT Formula)
	if (m_SecondOperandFormula)
		m_SecondOperandFormula->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
}

void BooleanFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	m_FirstOperandFormula->AppendUsedParameters(usedParameterIDs);

	//second operand is not mandatory (e.g. NOT Formula)
	if (m_SecondOperandFormula)
		m_SecondOperandFormula->AppendUsedParameters(usedParameterIDs);
}

void BooleanFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	//no contribution to jacobian matrix by boolean functions
}

Formula* BooleanFormula::DE_Jacobian(const int iEquation)
{
	// should not occur in ODE RHS
	throw ErrorData(ErrorData::ED_ERROR, "BooleanFormula::DE_Jacobian", "Boolean formulas should not occur in ODE RHS.");
}

Formula * BooleanFormula::RecursiveSimplify()
{
	m_FirstOperandFormula = m_FirstOperandFormula->RecursiveSimplify();
	if (m_SecondOperandFormula != NULL) m_SecondOperandFormula = m_SecondOperandFormula->RecursiveSimplify();
	if (m_FirstOperandFormula->IsConstant(CONSTANT_CURRENT_RUN)
		&& (m_SecondOperandFormula == NULL || m_SecondOperandFormula->IsConstant(CONSTANT_CURRENT_RUN)))
	{
		auto f = new ConstantFormula( DE_Compute(NULL, 0.0, USE_SCALEFACTOR) );
		delete this;
		return f;
	}

	return this;
}

void BooleanFormula::setFormula(Formula* FirstOperandFormula, Formula* SecondOperandFormula)
{
	m_FirstOperandFormula = FirstOperandFormula;
	m_SecondOperandFormula = SecondOperandFormula;
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
vector <double> BooleanFormula::SwitchTimePointFromComparisonFormula() const
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

double BooleanFormula::calculate_binaryOperation(const double* y, const double time,
   ScaleFactorUsageMode scaleFactorMode, const function<double(double, double)> & logicalOperation) const
{
	auto firstOperand = m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode);
	if (isnan(firstOperand))
		return MathHelper::GetNaN();

	auto secondOperand = m_SecondOperandFormula->DE_Compute(y, time, scaleFactorMode);
	if (isnan(secondOperand))
		return MathHelper::GetNaN();

	return logicalOperation(firstOperand, secondOperand);
}

double BooleanFormula::calculate_unaryOperation(const double* y, const double time,
   ScaleFactorUsageMode scaleFactorMode, const std::function<double(double)>& logicalOperation) const
{
	auto firstOperand = m_FirstOperandFormula->DE_Compute(y, time, scaleFactorMode);
	if (isnan(firstOperand))
		return MathHelper::GetNaN();

	return logicalOperation(firstOperand);
}

//returns (potential) switch time points for AND/OR/NOT-Rates
//will be overwritten by comparison rates (>, >=, ==, ...)
vector <double> BooleanFormula::SwitchTimePoints()
{
	vector <double> firstOpTimePoints, secondOpTimePoints;
	
	firstOpTimePoints = m_FirstOperandFormula->SwitchTimePoints();

	if (m_SecondOperandFormula)
		secondOpTimePoints = m_SecondOperandFormula->SwitchTimePoints();

	for (auto secondOpTimePoint : secondOpTimePoints)
      firstOpTimePoints.push_back(secondOpTimePoint);

	return firstOpTimePoints;
}

// check if the formula has form "Time OP Formula" or "Formula OP Time",
// where OP is one of boolean operations ==, >=, ...
// afterwards try to differentiate between explicit and implicit switches
// and store formula pointers in corresponding vectors
void BooleanFormula::SwitchFormulaFromComparisonFormula(std::vector<Formula*> &vecExplicit, std::vector<Formula*> &vecImplicit) const
{
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
			formulaWithSwitchTimePoints->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
			vecExplicit.push_back(formulaWithSwitchTimePoints);
		}
		catch (...)
		{
			//formula is ODE-variable(s) dependent.
			vecImplicit.push_back(formulaWithSwitchTimePoints);
		}
	}
	else
	{
		// too complicated to identify as explicit switch
		vecImplicit.push_back(formulaWithSwitchTimePoints);
	}
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
	return calculate_binaryOperation(y, time, scaleFactorMode, logical_and<>{});
}

Formula* AndFormula::clone()
{
   auto f = new AndFormula();

   f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
   f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

   return f;
}

void AndFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" & ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void AndFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " && ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
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
	return calculate_binaryOperation(y, time, scaleFactorMode, equal_to<>{});
}

Formula* EqualFormula::clone()
{
	auto f = new EqualFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void EqualFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" == ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void EqualFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	// potentially unsafe equality comparison for floating point numbers, but in line with direct evaluation and Matlab export
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " == ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
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
	return calculate_binaryOperation(y, time, scaleFactorMode, greater_equal<>{});
}

Formula* GreaterEqualFormula::clone()
{
	auto f = new GreaterEqualFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void GreaterEqualFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" >= ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void GreaterEqualFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " >= ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
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
	return calculate_binaryOperation(y, time, scaleFactorMode, greater<>{});
}

Formula* GreaterFormula::clone()
{
	auto f = new GreaterFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void GreaterFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" > ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void GreaterFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " > ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
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
	return calculate_binaryOperation(y, time, scaleFactorMode, less_equal<>{});
}

Formula* LessEqualFormula::clone()
{
	auto f = new LessEqualFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void LessEqualFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" <= ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void LessEqualFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " <= ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
}

vector <double> LessEqualFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

//-------------------------------------------------------------------
//---- Less (A<B)
//-------------------------------------------------------------------

double LessFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return calculate_binaryOperation(y, time, scaleFactorMode, less<>{});
}

Formula* LessFormula::clone()
{
	auto f = new LessFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void LessFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" < ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void LessFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " < ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
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
	return calculate_unaryOperation(y, time, scaleFactorMode, logical_not<>{});
}

Formula* NotFormula::clone()
{
	auto f = new NotFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();

	return f;
}

void NotFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	mrOut<<"~";
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
}

void NotFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	mrOut << "!";
	m_FirstOperandFormula->WriteCppCode(mrOut);
}

//-------------------------------------------------------------------
//---- Or (A||B)
//-------------------------------------------------------------------

double OrFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return calculate_binaryOperation(y, time, scaleFactorMode, logical_or<>{});
}

Formula* OrFormula::clone()
{
	auto f = new OrFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void OrFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" | ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void OrFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " || ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
}

//-------------------------------------------------------------------
//---- Unequal (A<>B)
//-------------------------------------------------------------------

double UnequalFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return calculate_binaryOperation(y, time, scaleFactorMode, not_equal_to<>{});
}

Formula* UnequalFormula::clone()
{
	auto f = new UnequalFormula();

	f->m_FirstOperandFormula = m_FirstOperandFormula->clone();
	f->m_SecondOperandFormula = m_SecondOperandFormula->clone();

	return f;
}

void UnequalFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	m_FirstOperandFormula->WriteMatlabCode(mrOut);
	mrOut<<" ~= ";
	m_SecondOperandFormula->WriteMatlabCode(mrOut);
}

void UnequalFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	// potentially unsafe inequality comparison for floating point numbers, but in line with direct evaluation and Matlab export
	m_FirstOperandFormula->WriteCppCode(mrOut);
	mrOut << " != ";
	m_SecondOperandFormula->WriteCppCode(mrOut);
}

vector <double> UnequalFormula::SwitchTimePoints()
{
	return SwitchTimePointFromComparisonFormula();
}

}//.. end "namespace SimModelNative"