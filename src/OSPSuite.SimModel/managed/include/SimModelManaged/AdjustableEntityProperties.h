#ifndef _AdjustableEntityProperties_H_
#define _AdjustableEntityProperties_H_

#include "SimModelManaged/EntityProperties.h"

namespace SimModelNET
{
	//public interface
	public interface class IAdjustableEntityProperties : public IEntityProperties
    {
		property long Id
		{
			long get();
		}

		property System::String^ Description
		{
			System::String^ get();
		}

		property double Value
		{
			double get();
			void set(double Value);
		}

		property System::String^ Unit
		{
			System::String^ get();
		}

		property bool IsFormula
		{
			bool get();
		}

		property System::String^ FormulaEquation
		{
			System::String^ get();
		}

    };

	ref class AdjustableEntityProperties : 
		public EntityProperties,
		public IAdjustableEntityProperties
	{
	protected:
		long _id;
		System::String^ _description;
		double _value;
		System::String^ _unit;
		bool   _isFormula;
		System::String^ _formulaEquation;

	internal:
		AdjustableEntityProperties(long id, System::String^ entityId, System::String^ path, 
			                       System::String^ name, System::String^ description, 
								   System::String^ unit, bool isFormula, System::String^ formulaEquation,
			                       double value);
	public:
		property long Id
		{
			virtual long get();
		}

		property System::String^ Description
		{
			virtual System::String^ get();
		}

		property double Value
		{
			virtual double get();
			virtual void set(double value);
		}

		property System::String^ Unit
		{
			virtual System::String^ get();
		}

		property bool IsFormula
		{
			virtual bool get();
		}

		property System::String^ FormulaEquation
		{
			virtual System::String^ get();
		}
	};
}


#endif //_ParameterProperties_H_

