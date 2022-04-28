#ifndef _TableFormulaWithXArgument_H_
#define _TableFormulaWithXArgument_H_

#include "SimModel/Formula.h"
#include "SimModel/Quantity.h"
#include "SimModel/TableFormula.h"

#ifdef _WINDOWS
#pragma warning( disable : 4279)
#endif

namespace SimModelNative
{
class TableFormulaWithXArgument :
	public Formula
{
private:
	//Id of referenced table object
	long _tableObjectId;

	//Id of referenced X argument object
	long _XArgumentObjectId;

protected:
	void WriteFormulaMatlabCode (std::ostream & mrOut);
	void WriteFormulaCppCode (std::ostream & mrOut);

	//Referenced table object
	Quantity * _tableObject;

	//Referenced X argument object
	Quantity * _XArgumentObject;
	
	//formula of referenced table formula object
	TableFormula * _tableFormula;

public:
	TableFormulaWithXArgument(void);
	virtual ~TableFormulaWithXArgument(void);

	virtual void LoadFromXMLNode (const XMLNode & pNode);
	virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	bool Simplify(bool forCurrentRunOnly);

	std::string Equation();

	double Safe_DE_Compute(const double* y, const double time, ScaleFactorUsageMode scaleFactorMode);
	double DE_Compute(const double* y, const double time, ScaleFactorUsageMode scaleFactorMode);
	void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
	virtual Formula * DE_Jacobian(const int iEquation);
	virtual Formula * clone();
	virtual Formula * RecursiveSimplify();
	void SetQuantityReference (const QuantityReference & quantityReference);

	virtual bool IsZero(void);
	bool IsRefIndependent(double & value);
	std::vector < HierarchicalFormulaObject * > GetUsedHierarchicalFormulaObjects();
	void Finalize();

	std::vector <double> RestartTimePoints();

	virtual void AppendUsedVariables(std::set<int> & usedVariablesIndices, const std::set<int> & variablesIndicesUsedInSwitchAssignments);
	virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

	virtual void UpdateIndicesOfReferencedVariables();
};

}//.. end "namespace SimModelNative"

#endif //_TableFormulaWithXArgument_H_
