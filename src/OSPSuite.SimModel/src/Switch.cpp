#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/Switch.h"
#include "SimModel/SimModelXMLHelper.h"
#include "SimModel/FormulaChange.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/Simulation.h"
#include "SimModel/BooleanFormula.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

Switch::Switch(void)
{
	_conditionFormula = NULL;
	_oneTime = false;
	_wasFired = false;
}

Switch::~Switch(void)
{
	_formulaChangeVector.clear();
}

void Switch::LoadFromXMLNode (const XMLNode & pNode)
{
	ObjectBase::LoadFromXMLNode(pNode);

	//load OneTime-Flag
	_oneTime = (pNode.GetAttribute(XMLConstants::OneTime,_oneTime ? 1:0)==1);

	//load formula change list
	SimModelXMLHelper<FormulaChange>::ObjectVectorLoadFromXMLNode(_formulaChangeVector, pNode.GetChildNode(XMLConstants::ChangeList));

	//set parent switch info
	for(int i=0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->SetParentSwitchInfo("Switch id="+_idAsString);
}

void Switch::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	const char * ERROR_SOURCE = "Switch::XMLFinalizeInstance";

	ObjectBase::XMLFinalizeInstance(pNode, sim);

	//formula change list
	SimModelXMLHelper<FormulaChange>::ObjectVectorXMLFinalizeInstance(
		_formulaChangeVector, pNode.GetChildNode(XMLConstants::ChangeList), sim);

	//condition formula
	long conditionFormulaId = (long)pNode.GetAttribute(XMLConstants::ConditionFormulaId,INVALID_QUANTITY_ID);
	_conditionFormula = sim->Formulas().GetObjectById(conditionFormulaId);

	if (_conditionFormula == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Condition formula in switch id=" + _idAsString + " not found");
}

void Switch::SimplifyFormulas(bool forCurrentRunOnly)
{
	assert(_conditionFormula != NULL);

	//condition formula
	_conditionFormula->Simplify(forCurrentRunOnly);

	//new formulas
	for(int i=0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->GetNewFormula()->Simplify(forCurrentRunOnly);
}

bool Switch::PerformSwitchUpdate (double * y, double time)
{
	//In OneTime-mode: check if switch was already fired. nothing to do if so
	if (_oneTime && _wasFired)
		return false;

	//evaluate switch condition formula
	bool switchConditionApplies = (_conditionFormula->DE_Compute(y, time, USE_SCALEFACTOR) == 1);

	if (!switchConditionApplies)
		return false; //switch not active by now

	//update was-fired flag
	_wasFired = true;

	bool switchUpdate = false;
	
	for(int i=0; i<_formulaChangeVector.size(); i++)
		switchUpdate |= _formulaChangeVector[i]->PerformSwitchUpdate(y, time);

	return switchUpdate;
}

void Switch::Finalize()
{
	for(int i=0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->Finalize();
}

vector <double> Switch::SwitchTimePoints()
{
	return _conditionFormula->SwitchTimePoints();
}

void Switch::WriteMatlabCode (std::ostream & mrOut)
{
	mrOut<<"    switchConditionApplies = ";
	_conditionFormula->WriteMatlabCode(mrOut);
	mrOut<<";"<<endl;

	mrOut<<"    if switchConditionApplies && isempty(intersect(SwitchUpdateTimePoints, Time))"<<endl<<endl;

	for(int i=0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->WriteMatlabCode(mrOut);

	mrOut<<"    end"<<endl<<endl;
}

void Switch::WriteCppCode(int switchIndex, const std::map<int, formulaParameterInfo > & formulaParameterIDs, const set<int> & usedIDs, std::ostream & mrOut)
{
	// TODO: check usage in combination with switchIndex
	//if (_conditionFormula->IsZero())
	//	return; //switch will never fire

	mrOut << "    if(";
	if (_oneTime)
		mrOut << "S[" << switchIndex << "] && ";
	_conditionFormula->WriteCppCode(mrOut);
	mrOut << ") {" << endl;
	
	for (int i = 0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->WriteCppCode(formulaParameterIDs, usedIDs, mrOut);

	if (_oneTime)
		mrOut << "        S[" << switchIndex << "] = 0;" << endl;

	mrOut << "        switchUpdate = true;" << endl;
	//mrOut << "        writelog(Time," << switchIndex << ");" << endl;
	mrOut << "    }" << endl << endl;
}

void Switch::MarkQuantitiesDirectlyUsedBy(void) //required for Matlab code generation only
{
	//mark quantities used by condition formula
	vector <HierarchicalFormulaObject *> conditionObjects = _conditionFormula->GetUsedHierarchicalFormulaObjects();
	for (unsigned int i=0; i<conditionObjects.size(); i++)
		conditionObjects[i]->SetIsUsedBySwitch();

	//mark quantities used by formula changes
	for(int j=0; j<_formulaChangeVector.size(); j++)
		_formulaChangeVector[j]->MarkQuantitiesDirectlyUsedBy();
}

void Switch::ResetState(void)
{
	_wasFired = false;
}

void Switch::AppendUsedVariables(set<int> & usedVariblesIndices)
{
	if (_conditionFormula->IsZero())
		return; //switch will never fire

	const set<int> emptySet;

	for(int i=0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->AppendVariablesUsedInNewFormula(usedVariblesIndices, emptySet);
}

void Switch::AppendUsedParameters(std::set<int> & usedParameterIDs, bool alwaysAppendInFormulaChange)
{
	if (_conditionFormula->IsZero())
		return; //switch will never fire

	_conditionFormula->AppendUsedParameters(usedParameterIDs);
	for (int i = 0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->AppendUsedParameters(usedParameterIDs, alwaysAppendInFormulaChange);
}

void Switch::AppendFormulaParameters(std::map<int, formulaParameterInfo > & formulaParameterIDs)
{
	if (_conditionFormula->IsZero())
		return; //switch will never fire

	for (int i = 0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->AppendFormulaParameters(formulaParameterIDs);
}

void Switch::SwitchFormulaFromComparisonFormula(std::vector<Formula*> &vecExplicit, std::vector<Formula*> &vecImplicit)
{
	BooleanFormula * f = dynamic_cast<BooleanFormula*>(_conditionFormula);
	if(f)
		SwitchFormulaFromComparisonFormula(vecExplicit, vecImplicit);
}

void Switch::UpdateDEIndexOfTargetSpecies()
{
	for(int i=0; i<_formulaChangeVector.size(); i++)
		_formulaChangeVector[i]->UpdateDEIndexOfTargetSpecies();
}

}//.. end "namespace SimModelNative"
