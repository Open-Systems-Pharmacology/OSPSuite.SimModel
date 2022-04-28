#ifndef _Formula_H_
#define _Formula_H_

#include <string>
#include <vector>
#include "SimModel/QuantityReference.h"
#include "SimModel/ObjectBase.h"
#include "SimModel/XMLLoader.h"
#include <set>
#include <map>
#include <sstream>

// difference is if quantitiy is "_isFixed" or not
const bool CONSTANT_CURRENT_RUN = false; //TODO

#ifdef linux
#include <fstream>
#endif

#define MATRIX_ELEM(A,i,j) (A[j][i])

namespace SimModelNative
{

class ValuePoint
{
public:
	ValuePoint();
	SIM_EXPORT ValuePoint(double x, double y, bool restartSolver);
	SIM_EXPORT ValuePoint(const ValuePoint & valuePoint);

	double X;
	double Y;
	bool RestartSolver;
};

class Formula :
	public ObjectBase
{
protected:
	std::string FormulaInfoForErrorMessage();
	const double mu_for_safe_computation = 0.01;

public:
	Formula(void);
	virtual ~Formula(void);

	virtual double Safe_DE_Compute(const double* y, const double time, ScaleFactorUsageMode scaleFactorMode) = 0;
	virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode) = 0;
	virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)=0;
	virtual void SetQuantityReference (const QuantityReference & quantityReference) = 0;
	
	//////////////////////////////////
	// functions used by CppODE export
	//
	// symbolic Jacobian calculation, automatic differentiation would be
	// preferable in theory, but sparsity pattern is harder to exploit
	//
	// supply species index for derivative w.r.t. species and
	// negative parameter id for derivatives w.r.t. certain paramters
	virtual Formula * DE_Jacobian(const int iEquation) = 0;
	// deep copy of formula
	virtual Formula * clone() = 0;
	// recursive simplification for symbolic jacobian equations
	// to keep code size and readability in in check
	virtual Formula * RecursiveSimplify() = 0;
	//////////////////////////////////

	//returns true for formulas like "2.5" or "2*sin(pi/3)" and calculates
	//formula value for this kind of formulas
	virtual bool IsRefIndependent(double & value);

	virtual bool Simplify(bool forCurrentRunOnly);

	virtual std::vector < HierarchicalFormulaObject * > GetUsedHierarchicalFormulaObjects();

	virtual bool IsZero(void) = 0;

	virtual void Finalize();

	virtual void WriteMatlabCode (std::ostream & mrOut);
	virtual void WriteCppCode (std::ostream & mrOut);

	virtual std::vector <double> SwitchTimePoints();

	virtual bool IsTime();

	virtual bool IsConstant(bool forCurrentRunOnly);

	virtual std::string Equation();

	virtual bool IsTable(void);

	//for table formula: returns table points. For any other formula: returns empty vector
	virtual std::vector <ValuePoint> GetTablePoints();

	//for table formula: set new table points. For any other formula: ERROR
	virtual void SetTablePoints(const std::vector <ValuePoint> & valuePoints);

	//append all DE variables used in the formula into the set
	virtual void AppendUsedVariables(std::set<int> & usedVariablesIndices, const std::set<int> & variablesIndicesUsedInSwitchAssignments) = 0;

	//append all parameters used in the formula into the set
	virtual void AppendUsedParameters(std::set<int> & usedParameterIDs) = 0;

	//Change indices of referenced variables according to the given indices permutation
	virtual void UpdateIndicesOfReferencedVariables() = 0;

protected:
	virtual bool UseBracketsForODESystemGeneration ();
	virtual void WriteFormulaMatlabCode (std::ostream & mrOut) = 0;
	virtual void WriteFormulaCppCode(std::ostream & mrOut) = 0;
};

}//.. end "namespace SimModelNative"


#endif //_Formula_H_

