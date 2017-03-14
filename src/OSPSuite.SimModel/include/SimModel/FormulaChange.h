#ifndef _FormulaChange_H_
#define _FormulaChange_H_

#include "SimModel/XMLLoader.h"
#include <string>
#include <set>

namespace SimModelNative
{

class Quantity;
class Formula;

class FormulaChange : 
	public XMLLoader
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

	bool PerformSwitchUpdate (double * y, double time);

	void WriteMatlabCode (std::ostream & mrOut);
	void MarkQuantitiesDirectlyUsedBy(void); //required for Matlab code generation only

	//used variables are only added into the list if UseAsValue=false
	void AppendVariablesUsedInNewFormula(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);

	//update the index of the target species (if any)
	void UpdateDEIndexOfTargetSpecies();
};

}//.. end "namespace SimModelNative"


#endif //_FormulaChange_H_
