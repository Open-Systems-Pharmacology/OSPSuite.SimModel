#include "SimModelManaged/AdjustableEntityProperties.h"

namespace SimModelNET
{
	AdjustableEntityProperties::AdjustableEntityProperties(long id, System::String^ entityId, System::String^ path, 
		                               System::String^ name, System::String^ description,
									   System::String^ unit, bool isFormula,
									   System::String^ formulaEquation, double value) 
	    : EntityProperties(entityId, path, name)

	{
		_id   = id;
		_description = description;
		_value = value;
		_unit = unit;
		_isFormula = isFormula;
		_formulaEquation = formulaEquation;
	}
    
	long AdjustableEntityProperties::Id::get()
	{
		return _id;
	}

	System::String^ AdjustableEntityProperties::Description::get()
	{
		return _description;
	}

	double AdjustableEntityProperties::Value::get()
	{
		return _value;
	}

	void AdjustableEntityProperties::Value::set(double value)
	{
		_value = value;
	}

	System::String^ AdjustableEntityProperties::Unit::get()
	{
		return _unit;
	}

	bool AdjustableEntityProperties::IsFormula::get()
	{
		return _isFormula;
	}

	System::String^ AdjustableEntityProperties::FormulaEquation::get()
	{
		return _formulaEquation;
	}

}
