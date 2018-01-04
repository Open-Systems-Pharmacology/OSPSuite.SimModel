#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/Formula.h"
#include <ErrorData.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

	using namespace std;

	Formula::Formula(void)
	{
	}

	Formula::~Formula(void)
	{
	}

	string Formula::FormulaInfoForErrorMessage()
	{
		string formulaInfo = "\n"+string("FormulaId = ")+_idAsString;
		return formulaInfo;
	}

	bool Formula::IsRefIndependent(double & value)
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::IsRefIndependent", "Function Formula::IsRefIndependent may not be called");
	}

	bool Formula::Simplify(bool forCurrentRunOnly)
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::Simplify", "Function Formula::Simplify may not be called");
	}

	std::vector < HierarchicalFormulaObject * > Formula::GetUsedHierarchicalFormulaObjects()
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::GetUsedHierarchicalFormulaObjects", "Function Formula::GetUsedHierarchicalFormulaObjects may not be called");
	}

	void Formula::Finalize()
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::Finalize", "Function Formula::Finalize may not be called");
	}

	std::string Formula::Equation()
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::Equation", "Function Formula::Equation may not be called");
	}

	bool Formula::UseBracketsForODESystemGeneration ()
	{
		return true; //per default, use brackets for the formula
	}

	void Formula::WriteMatlabCode (std::ostream & mrOut)
	{
		bool bracketRate = UseBracketsForODESystemGeneration();
		
		if (bracketRate)
			mrOut<<"(";
			
		WriteFormulaMatlabCode(mrOut);
		
		if (bracketRate)
			mrOut<<")";
	}

	bool Formula::IsTime()
	{
		return false;
	}

	bool Formula::IsConstant(bool forCurrentRunOnly)
	{
		return false;
	}

	vector <double> Formula::SwitchTimePoints()
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::SwitchTimePoints", 
						"Switch conditions seems to be setup incorrectly");
	}

	bool Formula::IsTable(void)
	{
		return false;
	}

	vector <ValuePoint> Formula::GetTablePoints()
	{
		vector <ValuePoint> emptyVec;
		return emptyVec;
	}

	void Formula::SetTablePoints(const std::vector <ValuePoint> & valuePoints)
	{
		throw ErrorData(ErrorData::ED_ERROR, "Formula::SetTablePoints", 
						"SetTablePoints may only be called for table formula");
	}

ValuePoint::ValuePoint()
{
	RestartSolver = false;
	X = 0.0;
	Y = 0.0;
}

ValuePoint::ValuePoint(double x, double y, bool restartSolver)
{
	RestartSolver = restartSolver;
	X = x;
	Y = y;
}

ValuePoint::ValuePoint(const ValuePoint & valuePoint)
{
	RestartSolver = valuePoint.RestartSolver;
	X = valuePoint.X;
	Y = valuePoint.Y;
}

}//.. end "namespace SimModelNative"
