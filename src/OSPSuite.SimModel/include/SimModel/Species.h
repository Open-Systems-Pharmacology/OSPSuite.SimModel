#ifndef _Species_H_
#define _Species_H_
 
#include "SimModel/Quantity.h"
#include "SimModel/Variable.h"
#include "SimModel/HierarchicalFormulaObject.h"
#include "SimModel/TObjectList.h"
#include "SimModel/SpeciesInfo.h"
#include "SimModel/VariableWithParameterSensitivity.h"
#include <set>
#include <map>

namespace SimModelNative
{

class Species :
	public HierarchicalFormulaObject,
	public VariableWithParameterSensitivity
{
protected:
	TObjectList<Formula> _rhsFormulaList;
	double m_ODEScaleFactor;
	double _DEScaleFactorInv; //inverse of scale factor
	int m_ODEIndex;
	int _rhsFormulaListSize;
	double _simulationStartTime; //used just for check during getting species value
	
	std::string getFormulaXMLAttributeName();
	
	//number of DE-Variables used in the RHS of given variable
	int _RHS_noOfUsedVariables;

	//indices of DE-variables used in RHS
	int * _RHS_UsedVariablesIndices;

	bool _negativeValuesAllowed;

public:
	Species(void);
	virtual ~Species(void);

	double GetODEScaleFactor () const;
	void SetODEScaleFactor (double p_ODEScaleFactor);

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	std::vector < HierarchicalFormulaObject * > GetUsedHierarchicalFormulaObjects ();
	
	bool IsConstant(bool forCurrentRunOnly);
	bool IsConstantDuringCalculation();

	double GetInitialValue (const double * y, double time);
	double GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode);
	virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);

	//simplify right hand side of ODE equations
	bool SimplifyRHSList();

	virtual bool Simplify (bool forCurrentRunOnly);

	int GetODEIndex () const;
	void DE_SetSpeciesIndex (int & iEquationNumber);

	void DE_Rhs (double * ydot, const double * y, const double time);
	void DE_Jacobian (double * * jacobian, const double * y, const double time);

	//set all species values = species initial value
	//(for species constant during simulation)
	void FillWithInitialValue(const double * speciesInitialValuesScaled);

	//rescale back (division by DE scale factor)
	void RescaleValues ();

	//set values in interval [-AbsTol..AbsTol] to zero
	void SetValuesBelowAbsTolLevelToZero(double absTol);

	void WriteMatlabCode (std::ostream & mrOut);

	//will be called between Load and Finalize of the parent simulation
	void InitialFillInfo(SpeciesInfo & info);

	//will be called after the finalize of the parent simulation
	void FillInfo(SpeciesInfo & info,const double * speciesInitialValues, double simulationStartTime);

	//caches all variables used in the RHS formula
	//also appends all variables used in ANY switch assignment formula with UseAsValue=false
	void CacheRHSUsedVariables(const std::set<int> & DEVariblesUsedInSwitchAssignments);

	//checks if RHS of the current variable depends on the DE Variable with given index
	//(using cached info)
	bool RHSDependsOn(int DE_VariableIndex);

	//Return dependency info of the RHS of the given variable
	std::vector<bool> RHSDependencyVector(int numberOfVariables);

	void SetODEIndex(int newIndex);

	//Change indices of variables used in the RHS formula according to the given indices permutation
	void ChangeIndicesOfRHSUsedVariables(std::map<unsigned int, unsigned int> & indexMap);

	//Return lower and upper band range of the used RHS variables
	//The half-bandwidths are set such that the nonzero locations (i, j) 
	//in the banded Jacobian satisfy 
    // -lowerHalfBandWidth <= j-i <= upperHalfBandWidth
	// (where i is the ODE index of the current variable)
	void GetRHSUsedBandRange(int & upperHalfBandWidth, int & lowerHalfBandWidth);

	bool NegativeValuesAllowed(void);
};

}//.. end "namespace SimModelNative"


#endif //_Species_H_
