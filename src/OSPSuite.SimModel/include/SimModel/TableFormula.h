#ifndef _TableFormula_H_
#define _TableFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class TableFormula :
	public Formula
{
private:
	//Times to Restart the solver
	std::vector <double> _restartTimes;

	//should formula values be used directly (e.g. as value of some time dependent parameter)
	// or should the values be derived (e.g. in some rate definition)
	bool _useDerivedValues;
	
	//cache some stuff for speed up
	long _numberOfValuePoints;
	double * _X_values;
	double * _Y_values;
	
	//derived values are filled only if the formula has _useDerivedValues=true
	// In this case, the i-th derived value is calculated for the interval [x_i; x_i+1)
	// Thus the number of derived values is one less than number of x-points
	double * _derivedValues;
protected:
	TObjectVector <ValuePoint> _valuePoints;

	void CacheValues(void);
	void WriteFormulaMatlabCode (std::ostream & mrOut);
	void WriteFormulaCppCode (std::ostream & mrOut);

	bool UseBracketsForODESystemGeneration ();

public:
	TableFormula(void);
	virtual ~TableFormula(void);

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

	bool IsTable(void);
	std::vector <ValuePoint> GetTablePoints();
	void SetTablePoints(const std::vector <ValuePoint> & valuePoints);

	virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
	virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

	virtual void UpdateIndicesOfReferencedVariables();
	//Update the value to ODEScaleFactor of the scale factor of the variable with the id referenced by this formula.
	virtual void UpdateScaleFactorOfReferencedVariable(const int quantity_id, const double ODEScaleFactor);

	void SetUseDerivedValues(bool useDerivedValues);
	bool UseDerivedValues();

	//get table formula value for the argument passed
	double GetValue(double argument);
};

}//.. end "namespace SimModelNative"

#endif //_TableFormula_H_
