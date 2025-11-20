#ifndef _FormulaChange_H_
#define _FormulaChange_H_

#include "SimModel/XMLLoader.h"
#include "SimModel/EntityWithCachedScaleFactor.h"
#include <string>
#include <set>
#include <map>
#include <vector>

namespace SimModelNative
{

class Quantity;
class Formula;

struct formulaParameterInfo {
	int switchIndex;
	int valueIndex;
	int initialIndex;
	std::vector<Formula *> vecFormulas;
	formulaParameterInfo(Formula * f, int _init=0) : switchIndex(-1), valueIndex(-1), initialIndex(_init), vecFormulas(1, f) {}
};

class FormulaChange : 
	public XMLLoader,
	public EntityWithCachedScaleFactor
{
protected:
	std::string _parentSwitchInfo;

	Quantity * _quantity;
	Formula  * _newFormula;

	int _speciesDEIndex;
	double _speciesScaleFactor;
	bool _useAsValue;

public:
	FormulaChange(void);
	virtual ~FormulaChange(void);

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	void SetParentSwitchInfo(const std::string & switchInfo);
	void Finalize();

	Formula * GetNewFormula(void);

	bool PerformSwitchUpdate (double * y, double time, bool & switchJacobians);

	void WriteMatlabCode (std::ostream & mrOut);
	void WriteCppCode(const std::map<int, formulaParameterInfo > & formulaParameterIDs, const std::set<int> & usedIDs, std::ostream & mrOut);

	void MarkQuantitiesDirectlyUsedBy(void); //required for Matlab code generation only

	//used variables are only added into the list if UseAsValue=false
	void AppendVariablesUsedInNewFormula(std::set<int> & usedVariablesIndices, const std::set<int> & variablesIndicesUsedInSwitchAssignments);
	void AppendUsedParameters(std::set<int> & usedParameterIDs, bool alwaysAppend);
	void AppendFormulaParameters(std::map<int, formulaParameterInfo > & formulaParameterIDs);

	//update the index of the target species (if any)
	void UpdateDEIndexOfTargetSpecies();

	virtual void UpdateScaleFactorOfReferencedVariable(const int odeIndex, const double ODEScaleFactor);
};

}//.. end "namespace SimModelNative"


#endif //_FormulaChange_H_
