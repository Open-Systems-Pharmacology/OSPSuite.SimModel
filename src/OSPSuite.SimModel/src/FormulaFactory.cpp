#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/FormulaFactory.h"
#include "SimModel/ExplicitFormula.h"
#include "SimModel/ParameterFormula.h"
#include "SimModel/VariableFormula.h"
#include "SimModel/SumFormula.h"
#include "SimModel/ProductFormula.h"
#include "SimModel/ConstantFormula.h"
#include "SimModel/UnaryFunctionFormula.h"
#include "SimModel/BooleanFormula.h"

#include "SimModel/DiffFormula.h"
#include "SimModel/DivFormula.h"
#include "SimModel/IfFormula.h"
#include "SimModel/MaxFormula.h"
#include "SimModel/MinFormula.h"
#include "SimModel/PowerFormula.h"
#include "SimModel/SimpleProductFormula.h"
#include "SimModel/TableFormula.h"
#include "SimModel/TableFormulaWithOffset.h"
#include "SimModel/TableFormulaWithXArgument.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

FormulaFactory::FormulaFactory(void)
{
}

FormulaFactory::~FormulaFactory(void)
{
}

Formula * FormulaFactory::CreateFormula(std::string formulaName)
{
	if (formulaName == FormulaName::ExplicitFormula)
		return new ExplicitFormula();

	if (formulaName == FormulaName::Parameter)
		return new ParameterFormula();

	if (formulaName == FormulaName::Variable)
		return new VariableFormula();

	if (formulaName == FormulaName::Sum)
		return new SumFormula();

	if (formulaName == FormulaName::Constant)
		return new ConstantFormula();

	if (formulaName == FormulaName::Product)
		return new ProductFormula();

	if (formulaName == FormulaName::Acos)
		return new AcosFormula();

	if (formulaName == FormulaName::Asin)
		return new AsinFormula();

	if (formulaName == FormulaName::Atan)
		return new AtanFormula();

	if (formulaName == FormulaName::Cosh)
		return new CoshFormula();

	if (formulaName == FormulaName::Cos)
		return new CosFormula();

	if (formulaName == FormulaName::Exp)
		return new ExpFormula();

	if ((formulaName == FormulaName::Ln) || (formulaName == FormulaName::Log))
		return new LnFormula(formulaName); //both "ln" and "log" accepted

	if (formulaName == FormulaName::Log10)
		return new Log10Formula();

	if (formulaName == FormulaName::Sinh)
		return new SinhFormula();

	if (formulaName == FormulaName::Sin)
		return new SinFormula();

	if (formulaName == FormulaName::Sqrt)
		return new SqrtFormula();

	if (formulaName == FormulaName::Tanh)
		return new TanhFormula();

	if (formulaName == FormulaName::Tan)
		return new TanFormula();

	if (formulaName == FormulaName::And)
		return new AndFormula();

	if (formulaName == FormulaName::Equal)
		return new EqualFormula();

	if (formulaName == FormulaName::GreaterEqual)
		return new GreaterEqualFormula();

	if (formulaName == FormulaName::Greater)
		return new GreaterFormula();

	if (formulaName == FormulaName::LessEqual)
		return new LessEqualFormula();

	if (formulaName == FormulaName::Less)
		return new LessFormula();

	if (formulaName == FormulaName::Not)
		return new NotFormula();

	if (formulaName == FormulaName::Or)
		return new OrFormula();

	if (formulaName == FormulaName::Unequal)
		return new UnequalFormula();

	if (formulaName == FormulaName::Diff)
		return new DiffFormula();

	if (formulaName == FormulaName::Div)
		return new DivFormula();

	if (formulaName == FormulaName::IF)
		return new IfFormula();

	if (formulaName == FormulaName::Max)
		return new MaxFormula();

	if (formulaName == FormulaName::Min)
		return new MinFormula();

	if (formulaName == FormulaName::Power)
		return new PowerFormula();

	if (formulaName == FormulaName::SimpleProduct)
		return new SimpleProductFormula();

	if (formulaName == FormulaName::TableFormula)
		return new TableFormula();
	
	if (formulaName == FormulaName::TableFormulaWithOffset)
		return new TableFormulaWithOffset();

	if (formulaName == FormulaName::TableFormulaWithXArgument)
		return new TableFormulaWithXArgument();

	//Formula not found
	throw ErrorData(ErrorData::ED_ERROR, "FormulaFactory::CreateFormula", "Formula type '"+ formulaName+"' is unknown");
}

}//.. end "namespace SimModelNative"
