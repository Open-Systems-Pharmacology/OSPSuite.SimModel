#include "SimModel/QuantityInfo.h"
#include "SimModel/MathHelper.h"

namespace SimModelNative
{

using namespace std;

QuantityInfo::QuantityInfo(void)
{
	_value = MathHelper::GetNaN();
	_isFormula=true;
	_id = INVALID_QUANTITY_ID;
}

long QuantityInfo::GetId(void) const
{
	return _id;
}

void QuantityInfo::SetId(long id)
{
	_id = id;
}

string QuantityInfo::GetEntityId(void) const
{
	return _entityId;
}

void QuantityInfo::SetEntityId(string entityId)
{
	_entityId = entityId;
}


string QuantityInfo::GetFullName(void) const
{
	return _fullName;
}

void QuantityInfo::SetFullName(const string & fullName)
{
	_fullName = fullName;
}

string QuantityInfo::GetDescription(void) const
{
	return _description;
}

void QuantityInfo::SetDescription(const std::string & description)
{
	_description = description;
}

double QuantityInfo::GetValue() const
{
	return _value;
}

void QuantityInfo::SetValue(double value)
{
	_value = value;
}

string QuantityInfo::GetUnit() const
{
	return _unit;
}

void QuantityInfo::SetUnit(const string & unit)
{
	_unit = unit;
}

bool QuantityInfo::IsFormula() const
{
	return _isFormula;
}

void QuantityInfo::SetIsFormula(bool isFormula)
{
	_isFormula = isFormula;
}

string QuantityInfo::GetFormulaEquation() const
{
	return _formula;
}

void QuantityInfo::SetFormulaEquation(const string & equation)
{
	_formula = equation;
}

string QuantityInfo::PathWithoutRoot(const string & objectPathDelimiter) const
{
	size_t firstdelimiterpos = _fullName.find_first_of(objectPathDelimiter);

	if (firstdelimiterpos==string::npos)
		return _fullName;

	return _fullName.substr(firstdelimiterpos+1);
}



}//.. end "namespace SimModelNative"
