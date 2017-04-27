#include "SimModelManaged/VariableValues.h"
#include "SimModelManaged/Conversions.h"
#include "SimModelManaged/ExceptionHelper.h"
#include "SimModel/Species.h"

#pragma warning( disable : 4005)

namespace SimModelNET
{
	VariableValues::VariableValues(SimModelNative::Quantity * quantity) 
		: EntityProperties(CPPToNETConversions::MarshalString(quantity->GetEntityId()),
						   CPPToNETConversions::MarshalString(quantity->GetContainerPath()),
						   CPPToNETConversions::MarshalString(quantity->GetName()))
	{
		_quantity = quantity;
	}

	VariableTypes VariableValues::VariableType::get()
	{		
		VariableTypes variableType;

		try
		{
			if (dynamic_cast<SimModelNative::Species*>(_quantity) != NULL)
				variableType = VariableTypes::Species;
			else
				variableType = VariableTypes::Observer;
		}
		catch(ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch(System::Exception^ )
		{
			throw;
		}
		catch(...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}

		return variableType;
	}

	array<double>^ VariableValues::Values::get()
	{
		array<double>^ values;

		try
		{
			SimModelNative::Variable * var = dynamic_cast <SimModelNative::Variable *> (_quantity);
			if (var == NULL) // should never happen
				throw gcnew System::ArgumentException(gcnew System::String(EntityId::get() + " is available in model but is not an entity with values(species, observer, ...)"));

			//for constant quantities: create array of 1 element. Otherwise: create full array
			int sizeToFill = _quantity->IsConstant(false) ? 1 : var->GetValuesSize();

			values = CPPToNETConversions::DoubleArrayToArray(var->GetValues(), sizeToFill);
		}
		catch(ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch(System::Exception^ )
		{
			throw;
		}
		catch(...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}

		return values;
	}

	bool VariableValues::IsConstant::get()
	{
		bool forCurrentRunOnly = false;

		return _quantity->IsConstant(forCurrentRunOnly);
	}

	double VariableValues::ComparisonThreshold::get()
	{
		SimModelNative::Variable * var = dynamic_cast <SimModelNative::Variable *> (_quantity);
		if (var == NULL) // should never happen
			throw gcnew System::ArgumentException(gcnew System::String(EntityId::get() + " is available in model but is not an entity with values(species, observer, ...)"));

		return var->GetComparisonThreshold();
	}
}
