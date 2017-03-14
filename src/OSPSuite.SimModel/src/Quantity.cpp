#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "SimModel/Quantity.h"
#include "SimModel/Formula.h"
#include "SimModel/Simulation.h"
#include "SimModel/MathHelper.h"
#include "XMLWrapper/XMLHelper.h"

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

Quantity::Quantity(void)
{
	_valueFormula = NULL;
	_value = 0.0;
	_originalValueFormula = NULL;
	_originalValue = 0.0;

	_isPersistable = true;

	_isChangedBySwitch = false;
	_isFixed = true;

	_isUsedBySwitch = false;

	_originalFormulaID = INVALID_QUANTITY_ID;
}

Quantity::~Quantity(void)
{
	DeleteFormula();
}

void Quantity::DeleteFormula()
{
	_valueFormula = NULL; //_valueFormula is either NULL or points to the _originalValueFormula

	//check if the formula was created "on the fly" and doesn't come from the simulation XML
	//(e.g. after setting table points into parameter which was not defined as table originally)
	if (_originalValueFormula && (_originalFormulaID==INVALID_QUANTITY_ID))
	{
		delete _originalValueFormula;
	}

	_originalValueFormula=NULL;
}

string Quantity::GetName(void)
{
	return _name;
}

string Quantity::GetDescription(void)
{
	return _description;
}

string Quantity::GetContainerPath(void)
{
	return _containerPath;
}

string Quantity::GetUnit(void)
{
	return _unit;
}

bool Quantity::IsPersistable(void)
{
	return _isPersistable;
}

bool Quantity::IsChangedBySwitch(void)
{
	return _isChangedBySwitch;
}

void Quantity::SetIsChangedBySwitch(bool changedBySwitch)
{
	_isChangedBySwitch = changedBySwitch;
}

void Quantity::LoadFromXMLNode (const XMLNode & pNode)
{
	ObjectBase::LoadFromXMLNode(pNode);
	
	_name = pNode.GetAttribute(XMLConstants::Name);
	_description = pNode.GetAttribute(XMLConstants::Description);
	_containerPath = pNode.GetAttribute(XMLConstants::Path);
	_unit = pNode.GetAttribute(XMLConstants::Unit);
	_isPersistable = (pNode.GetAttribute(XMLConstants::Persistable, _isPersistable ? 1:0)==1);
}

void Quantity::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	const char * ERROR_SOURCE = "Quantity::XMLFinalizeInstance";

	ObjectBase::XMLFinalizeInstance(pNode, sim);

	string objectPathDelimiter = sim->GetObjectPathDelimiter();

	if(sim->GetXMLVersion() < 4)
		_fullName = _containerPath + objectPathDelimiter + _name;
	else
	{
		_fullName = _containerPath != "" ? _containerPath : _name;
	}

	setPathWithoutRoot(objectPathDelimiter);

	//---- make sure only one of {formulaID, value} attributes is present
	bool hasValueAttribute = pNode.HasAttribute(XMLConstants::Value);
	if (hasValueAttribute && pNode.HasAttribute(getFormulaXMLAttributeName()))
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Quantity "+_fullName+" has both formula and value attributes");

	//---- check if value is given directly instead of formula
	if (hasValueAttribute)
	{
		//---- double value is available: set it and return
		double value;
		
		string sValue=pNode.GetAttribute(XMLConstants::Value);

		try
		{
			//first, try to convert value to double directly
			value=XMLHelper::ToDouble(sValue);
		}
		catch(...)
		{
			//failed. try special values (Nan, Inf, ...)
			value = getDoubleFromSpecialValue(sValue, ERROR_SOURCE);
		}
		
		DeleteFormula();

		_value = value;
		_originalValue = value;

		return;
	}

	//---- value is not available: set formula
	long formulaId   = (long)pNode.GetAttribute(getFormulaXMLAttributeName(), INVALID_QUANTITY_ID);
	_valueFormula = sim->Formulas().GetObjectById(formulaId);

	if (_valueFormula == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Formula with id="+XMLHelper::ToString(formulaId)+" not found (in Quantity with id=" + _idAsString+")");

	//store original formula id (used during saving simulation to XML)
	_originalFormulaID = formulaId;

	//copy reference to original formula in order to restore it at the end of the simulation
	// (if changed by switches etc. during simulation run)
	_originalValueFormula = _valueFormula;

	//check if formula is a constant expression and
	//replace value with double scalar if so
	ReplaceRefIndependentFormula();
}

void Quantity::setPathWithoutRoot(const string & objectPathDelimiter)
{
	int firstdelimiterpos = _fullName.find_first_of(objectPathDelimiter);

	if (firstdelimiterpos == string::npos)
		_pathWithoutRoot = _fullName;
	else
		_pathWithoutRoot = _fullName.substr(firstdelimiterpos + 1);
}

std::string Quantity::GetPathWithoutRoot(void)
{
	return _pathWithoutRoot;
}

double Quantity::getDoubleFromSpecialValue(string & sValue, const char * ERROR_SOURCE)
{
	sValue = XMLHelper::ToUpper(sValue);
	
	if ((sValue == "NAN") || (sValue == "N.DEF.") || (sValue == "N. DEF."))
		return MathHelper::GetNaN();

	if ((sValue == "INFINITY") || (sValue == "INF"))
		return MathHelper::GetInf();

	if ((sValue == "-INFINITY") || (sValue == "-INF"))
		return MathHelper::GetNegInf();

	//invalid value
	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Quantity "+_fullName+" has invalid value "+sValue);
}

bool Quantity::Simplify (bool forCurrentRunOnly)
{
	if (_valueFormula == NULL)
		return false; //already simplified (scalar)
	
	//try to simplify formula first
	bool Simplified = _valueFormula->Simplify(forCurrentRunOnly);
	
	if (!Simplified) //some of used objects could not be simplified
		return false;

	_value = _valueFormula->DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
	_valueFormula = NULL; 

	//reset initial value also (only if not in CurrentRun-mode)
	if (!forCurrentRunOnly)
	{
		DeleteFormula();
		_originalValue = _value;
	}

	return true;
}

//reset formula/value state after simulation run is finished
// (e.g. if changed by switches during simulation)
void Quantity::ResetState(void)
{
	//either Value or ValueFormula is used - don't care, just reset both
	_value = _originalValue;
	_valueFormula = _originalValueFormula;
}


void Quantity::SetInitialValue(double value)
{
	DeleteFormula();

	_value = value;
	_originalValue = value;
}

void Quantity::SetConstantValue(double value)
{
	_valueFormula = NULL;
	_value = value;
}

bool Quantity::IsConstant(bool forCurrentRunOnly)
{
	if (forCurrentRunOnly)
		return (_valueFormula == NULL) && !_isChangedBySwitch;

	return (_valueFormula == NULL) && _isFixed && !_isChangedBySwitch;
}

void Quantity::ReplaceRefIndependentFormula(void)
{
	double value;
	if (_valueFormula->IsRefIndependent(value))
	{
		DeleteFormula();

		_originalValue = value;
		_value = value;
	}
}

HierarchicalFormulaObject * Quantity::GetHierarchicalFormulaObject(void)
{
	return dynamic_cast<HierarchicalFormulaObject *>(this);
}

string Quantity::GetFullName(void)
{
	return _fullName;
}

bool Quantity::IsFormulaEqualTo(Formula * formula)
{
	return (_valueFormula == formula);
}

void Quantity::SetFormula(Formula * formula)
{
	_valueFormula = formula;
}

bool Quantity::IsFixed(void)
{
	return _isFixed;
}

void Quantity::SetIsFixed(bool isFixed)
{
	_isFixed = isFixed;
}

void Quantity::SetIsUsedBySwitch(void)
{
	_isUsedBySwitch = true;
}

bool Quantity::IsUsedBySwitch(void)
{
	return _isUsedBySwitch;
}

//will be called between Load and Finalize of the parent simulation
//must set all properties of the quantity info (name, unit, id, ...)
//The quantity formula may not be evaluated at this time point
//The only exception are constant formulas, which were already replaced
//by values in Simulation::LoadFromXXX
void Quantity::InitialFillInfo(QuantityInfo & info)
{
	info.SetId(_id);
	info.SetEntityId(_entityId);
	info.SetFullName(_fullName);
	info.SetUnit(_unit);
	info.SetDescription(_description);

	//check if is formula or not
	if (_valueFormula)
	{
		if (_valueFormula->IsTable())
		{
			info.SetIsFormula(false);
			info.SetFormulaEquation("");
		}
		else
		{
			info.SetIsFormula(true);
			info.SetFormulaEquation(_valueFormula->Equation());
		}
		info.SetValue(MathHelper::GetNaN()); //cannot evaluate formula value at this timepoint
	}
	else
	{
		info.SetIsFormula(false);
		info.SetFormulaEquation("");
		info.SetValue(_value);
	}
}

//will be called after the finalize of the parent simulation
//only formula/value info must be updated
//Species initial values and sim. start time are required for the formula evaluation
void Quantity::FillInfo(QuantityInfo & info, 
						const double * speciesInitialValues,
						double simulationStartTime)
{
	if (_valueFormula)
	{
		if (_valueFormula->IsTable())
		{
			info.SetIsFormula(false);
			info.SetFormulaEquation("");
			info.SetValue(MathHelper::GetNaN());
		}
		else
		{
			info.SetIsFormula(true);
			info.SetFormulaEquation(_valueFormula->Equation());
			info.SetValue(_valueFormula->DE_Compute(speciesInitialValues,simulationStartTime, IGNORE_SCALEFACTOR)); 
		}
	}
	else
	{
		info.SetIsFormula(false);
		info.SetFormulaEquation("");
		info.SetValue(_value);
	}
}

long Quantity::GetFormulaId(void)
{
	if (_valueFormula == NULL)
		return INVALID_QUANTITY_ID;

	return _valueFormula->GetId();
}

//TODO this will NOT work properly for changed table formulas
// the function must be adjusted for table formulas
void Quantity::UpdateFormulaInXMLNode(XMLNode & pFormulaListNode, XMLNode & pQuantitiesListNode)
{
	const char * ERROR_SOURCE = "Quantity::UpdateFormulaInXMLNode";

	//only NOT FIXED parameters must be updated in XML
	//Other parameters cannot be changed by user (whether constant or not)
	if(_isFixed)
		return;

	//only replace formula value if the quantity is constant
	if(_originalValueFormula != NULL)
		return;

	//---- check if parameter was const in original XML.
	//     Then value-attribute must be set
	if (_originalFormulaID==INVALID_QUANTITY_ID)
	{
		for (XMLNode pQuantityNode = pQuantitiesListNode.GetFirstChild(); !pQuantityNode.IsNull();pQuantityNode = pQuantityNode.GetNextSibling())
		{
			long quantityId = (long)pQuantityNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);

			if (quantityId != _id)
				continue;

			//found quantity node: set value and exit
			if (MathHelper::IsNaN(_value))
				pQuantityNode.SetAttribute(XMLConstants::Value, "NaN");
			else
				pQuantityNode.SetAttribute(XMLConstants::Value, _value);

			return;
		}

		//quantity with the stored id not found - should never happen
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
			"Quantity with id "+_idAsString+" not found in the list");
	}

	for (XMLNode pFormulaNode = pFormulaListNode.GetFirstChild(); !pFormulaNode.IsNull();pFormulaNode = pFormulaNode.GetNextSibling())
	{
		long formulaId = (long)pFormulaNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);

		if (formulaId != _originalFormulaID)
			continue;

		//remove old formula node
		pFormulaListNode.RemoveChildNode(pFormulaNode);

		//---- create new explicit formula node with formula equation=<quantity value>
		XMLNode newFormulaNode = pFormulaListNode.CreateChildNode(XMLConstants::ExplicitFormula);

		//set formula id
		newFormulaNode.SetAttribute(XMLConstants::Id, formulaId);

		XMLNode equationNode = newFormulaNode.CreateChildNode(XMLConstants::Equation);
		equationNode.SetValue(_value);

		//add empty parameter- and variables lists (required by schema)
		newFormulaNode.CreateChildNode(XMLConstants::ParameterList);
		newFormulaNode.CreateChildNode(XMLConstants::VariableList);

		//exit function
		return;
	}

	//formula with the stored id not found - should never happen
	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
		"Formula with id "+XMLHelper::ToString(_originalFormulaID)+
		" not found (quantity id: "+XMLHelper::ToString(_id)+")");
}

void Quantity::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	if (_valueFormula == NULL)
		return; //nothing to do

	_valueFormula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void Quantity::UpdateIndicesOfReferencedVariables()
{
	if (_valueFormula == NULL)
		return; //nothing to do

	_valueFormula->UpdateIndicesOfReferencedVariables();
}


}//.. end "namespace SimModelNative"
