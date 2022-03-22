#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#ifdef linux
#include <algorithm>
#endif

#include "SimModel/FormulaChange.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/Simulation.h"
#include "XMLWrapper/XMLHelper.h"
#include <sstream>

namespace SimModelNative
{

using namespace std;

FormulaChange::FormulaChange(void)
{
	_quantity = NULL;
	_newFormula = NULL;
	_speciesDEIndex = DE_INVALID_INDEX;
	_useAsValue = false; //per default, use formula and not its value if the parent switch fires
	_speciesScaleFactor = 1.0;
}

FormulaChange::~FormulaChange(void)
{
}

void FormulaChange::LoadFromXMLNode (const XMLNode & pNode)
{
	//load UseAsValue-Flag
	_useAsValue = (pNode.GetAttribute(XMLConstants::UseAsValue,_useAsValue ? 1:0)==1);
}

void FormulaChange::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	const char * ERROR_SOURCE = "FormulaChange::XMLFinalizeInstance";

	long quantityToChangeId = (long)pNode.GetAttribute(XMLConstants::ObjectId, INVALID_QUANTITY_ID);
	long newFormulaId       = (long)pNode.GetAttribute(XMLConstants::NewFormulaId, INVALID_QUANTITY_ID);

	_quantity = sim->AllQuantities().GetObjectById(quantityToChangeId);

	if (_quantity == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, _parentSwitchInfo+": object with id="+XMLHelper::ToString(quantityToChangeId)+" not found");

	_newFormula = sim->Formulas().GetObjectById(newFormulaId);

	if (_newFormula == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, _parentSwitchInfo+": formula with id="+XMLHelper::ToString(newFormulaId)+" not found");

	//mark quantity as one (potentially) changed by switch(es)
	_quantity->SetIsChangedBySwitch(true);

	//If the quantity is a species, add this formula change to the list of entities that cache its scale factor (used for updating scale factors)
	Species * species = dynamic_cast<Species*>(_quantity);
	if (species == NULL) //changed quantity is not a species
		return;
	species->AddEntityWithCachedScaleFactor(this);
}

void FormulaChange::SetParentSwitchInfo(const std::string & switchInfo)
{
	_parentSwitchInfo = switchInfo;
}

Formula * FormulaChange::GetNewFormula(void)
{
	return _newFormula;
}

bool FormulaChange::PerformSwitchUpdate(double * y, double time, set<int>& changedVariablesIndicesUsedInParameters) const
{
	if(_speciesDEIndex != DE_INVALID_INDEX)
	{
		//object to change is a species
		double newValue = _newFormula->DE_Compute(y, time, USE_SCALEFACTOR);
		newValue /= _speciesScaleFactor;

		if(newValue == y[_speciesDEIndex])
			return false; //new formula produced the same value as before - no change

		y[_speciesDEIndex] = newValue;

		return true; //simulation conditions changed by switch (solver restart needed)
	}

	//---- object to change is not a species - set new formula or value
	if (_useAsValue)
	{
		//add variable indices used by the original quantity formula
		_quantity->AppendUsedVariables(changedVariablesIndicesUsedInParameters);

		//set formula VALUE of the new formula
		_quantity->SetConstantValue(_newFormula->DE_Compute(y, time, USE_SCALEFACTOR));
		return true;
	}

	if (_quantity->IsFormulaEqualTo(_newFormula))
		return false; //same formula already set - no change

	//---- really new formula - set new FORMULA

	//first, add state variables used by the old formula to the set of changed RHS variables
	_quantity->AppendUsedVariables(changedVariablesIndicesUsedInParameters);

	//update formula
	_quantity->SetFormula(_newFormula);

	//now add state variables used by the NEW formula to the set of changed RHS variables
	_quantity->AppendUsedVariables(changedVariablesIndicesUsedInParameters);

	return true; //simulation conditions changed by switch (solver restart needed)
}

void FormulaChange::Finalize()
{
	const char * ERROR_SOURCE = "FormulaChange::Finalize";

	assert(_quantity != NULL);

	//---- if non-constant species: cache species index for performing switch update
	Species * species = dynamic_cast<Species*>(_quantity);
	if(species)
	{
		if(species->IsConstantDuringCalculation()) //should never happen
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, _parentSwitchInfo+": species changed by switch is marked as constant");

		_speciesDEIndex = species->GetODEIndex();
		assert(_speciesDEIndex != DE_INVALID_INDEX);

		_speciesScaleFactor = species->GetODEScaleFactor();
	}

}

void FormulaChange::WriteMatlabCode (std::ostream & mrOut)
{

	if(_speciesDEIndex != DE_INVALID_INDEX)
	{
		//object to change is a species
		mrOut<<"        newValue = ";
		_newFormula->WriteMatlabCode(mrOut);
		mrOut<<";"<<endl;

		mrOut<<"        if newValue ~= y("<<_speciesDEIndex+1<<")"<<endl;
		mrOut<<"            yOut("<<_speciesDEIndex+1<<") = newValue;"<<endl;
		mrOut<<"            switchUpdate = true;"<<endl;
		mrOut<<"        end"<<endl<<endl;
	}

	else
	{
		//check if parameter; only parameters and species are supported by matlab code
		Parameter * param = dynamic_cast <Parameter *>(_quantity);

		if(param == NULL)
			return;

		string paramName = param->GetShortUniqueName();

		//---- write new parameter function handle into a string
		ostringstream newFormulaStream;
		newFormulaStream<<"@(Time,y) ";
		_newFormula->WriteMatlabCode(newFormulaStream);
		string newFormula = newFormulaStream.str();
		
		mrOut<<"        newFormula = "<<newFormula<<";"<<endl;
		mrOut<<"        if isnumeric("<<paramName<<")"<<endl;
		mrOut<<"            "<<paramName<<" = newFormula;"<<endl;
		mrOut<<"            switchUpdate = true;"<<endl;
		mrOut<<"        else"<<endl;
		mrOut<<"            if ~strcmp(func2str("<<paramName<<"), func2str(newFormula))"<<endl;
		mrOut<<"                "<<paramName<<" = newFormula;"<<endl;
		mrOut<<"                switchUpdate = true;"<<endl;
		mrOut<<"            end"<<endl;
		mrOut<<"        end"<<endl<<endl;
	}
}

void FormulaChange::WriteCppCode(const std::map<int, formulaParameterInfo > & formulaParameterIDs, const set<int> & usedIDs, std::ostream & mrOut)
{
	if (_speciesDEIndex != DE_INVALID_INDEX)
	{
		//object to change is a species
//		mrOut << "        oldValue = y[" << _speciesDEIndex << "];" << endl;
		mrOut << "        y[" << _speciesDEIndex << "] = ";
		_newFormula->WriteCppCode(mrOut);
		mrOut << ";" << endl;
		//mrOut << ")/" << _speciesScaleFactor << ";" << endl;
//		mrOut << "        switchUpdate = switchUpdate || (oldValue!=y[" << _speciesDEIndex << "]);" << endl;
	}

	else
	{
		//check if parameter; only parameters and species are supported by C++ code
		Parameter * param = dynamic_cast <Parameter *>(_quantity);

		if (param == NULL)
			throw ErrorData(ErrorData::ED_ERROR, "Switch", "Unsupported switch type for C++ export.");

		// TODO: add warning if new formula depends on y regarding jacobian
		map<int, formulaParameterInfo>::const_iterator iter = formulaParameterIDs.find(param->GetId());
		if (iter == formulaParameterIDs.end())
		{
			string paramName = param->GetShortUniqueName();

			//---- write new parameter function handle into a string
			//mrOut << "        oldValue = " << paramName << ";" << endl;

			mrOut << "        " << paramName << " = ";
			_newFormula->WriteCppCode(mrOut);
			mrOut << ";" << std::endl;
		}
		else
		{
			if (_useAsValue || _newFormula->IsConstant(CONSTANT_CURRENT_RUN))
			{
				mrOut << "        S[" << iter->second.switchIndex << "] = 0;" << endl;
				mrOut << "        P[" << iter->second.valueIndex << "] = ";
				_newFormula->WriteCppCode(mrOut);
				mrOut << ";" << endl;
			}
			else
			{
				vector<Formula*>::const_iterator viter = find(iter->second.vecFormulas.begin(), iter->second.vecFormulas.end(), _newFormula);
				if (viter == iter->second.vecFormulas.end())
					throw ErrorData(ErrorData::ED_ERROR, "Switch", "Formula not found.");
				size_t caseIndex = viter - iter->second.vecFormulas.begin() + 1;
				mrOut << "        S[" << iter->second.switchIndex << "] = " << caseIndex << ";" << endl;
				// if param is used in switch functions, update value immediatly
				if (usedIDs.find(param->GetId()) != usedIDs.end())
				{
					mrOut << "        " << param->GetShortUniqueName() << " = ";
					_newFormula->WriteCppCode(mrOut);
					mrOut << ";" << endl;
				}
			}
		}

		//mrOut << ";" << endl;
		//mrOut << "        switchUpdate = switchUpdate || (oldValue!=" << paramName << ");" << endl;
	}
}

//required for Matlab code generation only
void FormulaChange::MarkQuantitiesDirectlyUsedBy(void) 
{
	//mark quantities used by condition formula
	vector <HierarchicalFormulaObject *> usedObjects = _newFormula->GetUsedHierarchicalFormulaObjects();
	for (unsigned int i=0; i<usedObjects.size(); i++)
		usedObjects[i]->SetIsUsedBySwitch();
}


void FormulaChange::AppendVariablesUsedInNewFormula(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
{
	if (_useAsValue)
		return; 

	_newFormula->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
}

void FormulaChange::AppendUsedParameters(std::set<int> & usedParameterIDs, bool alwaysAppend)
{
	// if NOT usedAsValue it is contained in switch statement
	if (!_useAsValue && !alwaysAppend)
		return;

	_newFormula->AppendUsedParameters(usedParameterIDs);
}

void FormulaChange::AppendFormulaParameters(std::map<int, formulaParameterInfo > & formulaParameterIDs)
{
	if (_useAsValue)
		return;

	Parameter * param = dynamic_cast <Parameter *>(_quantity);
	if (param == NULL)
		return;

	if (_newFormula->IsConstant(CONSTANT_CURRENT_RUN))
		return;

	//if (param->IsFormulaEqualTo(NULL))
	//	return;

	map<int, formulaParameterInfo >::iterator iter = formulaParameterIDs.find(param->GetId());
	if (iter==formulaParameterIDs.end())
		formulaParameterIDs.insert(pair<int, formulaParameterInfo >(param->GetId(), formulaParameterInfo(_newFormula)));
	else
		iter->second.vecFormulas.push_back(_newFormula);
}

void FormulaChange::UpdateDEIndexOfTargetSpecies()
{
	Finalize();
}

void FormulaChange::UpdateScaleFactorOfReferencedVariable(const int odeIndex, const double ODEScaleFactor)
{
	if (_speciesDEIndex == odeIndex)
		_speciesScaleFactor = ODEScaleFactor;
}

}//.. end "namespace SimModelNative"
