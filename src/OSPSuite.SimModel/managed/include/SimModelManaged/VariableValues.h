#ifndef _VariableValues_H_
#define _VariableValues_H_

#include "SimModelManaged/EntityProperties.h"
#include "SimModelManaged/EntityProperties.h"
#include "SimModel/Variable.h"
#include "SimModel/Quantity.h"

namespace SimModelNET
{
	public enum class VariableTypes
    {
        Species  = 1,
        Observer = 2
	};

	//public interface
	public interface class IValues : IEntityProperties
    {
		property VariableTypes VariableType
		{
			VariableTypes get();
		}

		property array<double>^ Values
		{
			array<double>^ get();
		}

		property bool IsConstant
		{
			bool get();
		}
    };

	ref class VariableValues : 
		public EntityProperties,
		public IValues
	{
	private:
		SimModelNative::Quantity * _quantity;
	internal:
		VariableValues(SimModelNative::Quantity * quantity);
	public:
		property VariableTypes VariableType
		{
			virtual VariableTypes get();
		}

		property array<double>^ Values
		{
			virtual array<double>^ get();
		}

		property bool IsConstant
		{
			virtual bool get();
		}
	};
}

#endif //_VariableValues_H_

