#ifndef _TableFormulaWithOffset_H_
#define _TableFormulaWithOffset_H_

#include "SimModel/Formula.h"
#include "SimModel/Quantity.h"
#include "SimModel/TableFormula.h"

#ifdef _WINDOWS
#pragma warning( disable : 4279)
#endif

namespace SimModelNative
{
class TableFormulaWithOffset :
	public Formula
{
private:
	//Id of referenced table object
	long _tableObjectId;

	//Id of referenced offset object
	long _offsetObjectId;

protected:
	void WriteFormulaMatlabCode (std::ostream & mrOut);
	void WriteFormulaCppCode (std::ostream & mrOut);

	//Referenced table object
	Quantity * _tableObject;

	//Referenced offset object
	Quantity * _offsetObject;
	
	//formula of referenced table formula object
	TableFormula * _tableFormula;

public:
	TableFormulaWithOffset(void);
	virtual ~TableFormulaWithOffset(void);

	virtual void LoadFromXMLNode (const XMLNode & pNode);
	virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	bool Simplify(bool forCurrentRunOnly);

	std::string Equation();

	double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
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

	virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
	virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

	virtual void UpdateIndicesOfReferencedVariables();
	//Update the value to ODEScaleFactor of the scale factor of the variable with the id referenced by this formula.
	virtual void UpdateScaleFactorOfReferencedVariable(const int quantity_id, const double ODEScaleFactor);
};

}//.. end "namespace SimModelNative"

#endif //_TableFormulaWithOffset_H_
