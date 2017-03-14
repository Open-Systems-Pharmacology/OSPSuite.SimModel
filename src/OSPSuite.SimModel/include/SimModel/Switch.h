#ifndef _Switch_H_
#define _Switch_H_

#include "SimModel/ObjectBase.h"
#include "SimModel/TObjectVector.h"
#include "SimModel/FormulaChange.h"
#include <vector>
#include <set>

namespace SimModelNative
{

class Formula;

class Switch :
	public ObjectBase
{
protected:
	Formula * _conditionFormula;
	TObjectVector<FormulaChange> _formulaChangeVector;
	bool _oneTime; //should the switch fire only the first time its condition formula is satisfied
	bool _wasFired; //was switch already fired (relevant if oneTime=true)

public:
	Switch(void);
	virtual ~Switch(void);

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	void SimplifyFormulas(bool forCurrentRunOnly);
	void Finalize();

	bool PerformSwitchUpdate (double * y, double time);

	std::vector <double> SwitchTimePoints();

	void WriteMatlabCode (std::ostream & mrOut);
	void MarkQuantitiesDirectlyUsedBy(void); //required for Matlab code generation only

	void ResetState();

	void AppendUsedVariables(std::set<int> & usedVariblesIndices);

	//Update the index of the target species 
	void UpdateDEIndexOfTargetSpecies();
};

}//.. end "namespace SimModelNative"


#endif //_Switch_H_
