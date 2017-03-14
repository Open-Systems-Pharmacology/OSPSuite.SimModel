#ifndef _QuantityInfo_H_
#define _QuantityInfo_H_

#include "SimModel/GlobalConstants.h"
#include <string>

namespace SimModelNative
{

class QuantityInfo
{
protected:
	//object id
	long _id;
	std::string _entityId;

	//path+name
	std::string _fullName; 

	double _value;
	std::string _unit;
	bool _isFormula;
	std::string _formula;
	std::string _description;

public:
	virtual ~QuantityInfo()
	{
	}

	QuantityInfo(void);

	SIM_EXPORT long GetId(void) const;
	void SetId(long id);

	SIM_EXPORT std::string GetEntityId(void) const;
	void SetEntityId(std::string id);

	SIM_EXPORT std::string GetFullName(void) const;
	void SetFullName(const std::string & fullName);

	SIM_EXPORT std::string GetDescription(void) const;
	void SetDescription(const std::string & description);

	SIM_EXPORT double GetValue() const;
	SIM_EXPORT virtual void SetValue(double value);

	SIM_EXPORT std::string GetUnit() const;
	void SetUnit(const std::string & unit);

	SIM_EXPORT bool IsFormula() const;
	void SetIsFormula(bool isFormula);

	SIM_EXPORT std::string GetFormulaEquation() const;
	void SetFormulaEquation(const std::string & equation);

	std::string PathWithoutRoot(const std::string & objectPathDelimiter) const;
};

}//.. end "namespace SimModelNative"

#endif //_QuantityInfo_H_

