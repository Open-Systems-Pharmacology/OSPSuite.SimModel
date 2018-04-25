#ifdef _WINDOWS
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/TableFormula.h"
#include "SimModel/Quantity.h"
#include "SimModel/TableFormulaWithXArgument.h"
#include "SimModel/SimModelTypeDefs.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModelSpecs/ExplicitFormulaSpecsHelper.h"
#include "SimModelSpecs/TableFormulaSpecsHelper.h"

#include <math.h>

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework; 
	using namespace SimModelNET;

	//provide access to protected members via public inheritance
	class TableFormulaWithXArgumentExtender : public SimModelNative::TableFormulaWithXArgument
	{
	public:
		void SetTableObject(SimModelNative::Quantity * tableObject){_tableObject = tableObject;}
		void SetXArgumentObject(SimModelNative::Quantity * xargumentObject){ _XArgumentObject = xargumentObject;}
		void SetTableFormula(SimModelNative::TableFormula * tableFormula){_tableFormula = tableFormula;}
		SimModelNative::TableFormula * GetTableFormula() { return _tableFormula; }
	};

	ref class TableFormulaWithXArgumentWrapper
	{
	public:
		TableFormulaWithXArgumentExtender * Formula;
		TableFormulaExtender * TableFormula;
		
		TableFormulaWithXArgumentWrapper()
		{
			Formula=new TableFormulaWithXArgumentExtender();
			TableFormula = new TableFormulaExtender;
		}
		~TableFormulaWithXArgumentWrapper(){delete Formula; delete TableFormula;}
		double Calculate(double x)
		{
			return Formula->DE_Compute(NULL, x, SimModelNative::USE_SCALEFACTOR);
		}
		void UseDerivedValues(bool useDerivedValues) { Formula->GetTableFormula()->SetUseDerivedValues(useDerivedValues);}
	};

	public ref class concern_for_table_with_xargument_formula abstract : ContextSpecification<TableFormulaWithXArgumentWrapper^>
    {
    public:
		virtual void GlobalContext() override
		{
			try
			{
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
		}

	protected:
        virtual void Context() override
        {
            sut=gcnew TableFormulaWithXArgumentWrapper();
        }
    };

	public ref class when_creating_for_given_table_with_non_time_dependent_xargument : public concern_for_table_with_xargument_formula
    {
	protected:   
			ParameterExtender * _XArgumentObject;
			ParameterExtender * _tableObject;

		virtual void Because() override
        {
			SimModelNative::TObjectVector <SimModelNative::ValuePoint> & ValuePoints = sut->TableFormula->ValuePoints();
			ValuePoints.push_back(new SimModelNative::ValuePoint(0,0,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(1,2,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(5,3,true));
			ValuePoints.push_back(new SimModelNative::ValuePoint(20,6,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(40,6,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(41,8,false));

			sut->TableFormula->CallCacheValues();
			
			_XArgumentObject = new ParameterExtender();
			sut->Formula->SetXArgumentObject(_XArgumentObject);
			sut->Formula->SetTableFormula(sut->TableFormula);

			_tableObject = new ParameterExtender();
			_tableObject->SetFormula(sut->TableFormula);
			sut->Formula->SetTableObject(_tableObject);

			//sut->Formula->GetTableFormula()->SetUseDerivedValues(false);
        }

		void checkValue(double xargument, double expectedValue)
		{
			_XArgumentObject->SetConstantValue(xargument);

			double dummyTime = 0.0; //time does not matter for x argument which is not time dependent
			BDDExtensions::ShouldBeEqualTo(sut->Calculate(dummyTime), expectedValue);
		}

    public:
        [TestAttribute]
        void should_calculate_correct_values_in_direct_mode()
        {
			try
			{
				sut->UseDerivedValues(false);

				checkValue(-1, 0.0);
				checkValue(0, 0.0);
				checkValue(0.5, 1.0);
				checkValue(1, 2.0);
				checkValue(3, 2.5);
				checkValue(20, 6.0);
				checkValue(30, 6.0);
				checkValue(40.5, 7.0);
				checkValue(41, 8.0);
				checkValue(50, 8.0);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }

		[TestAttribute]
		void should_calculate_correct_values_in_derived_mode()
		{
			try
			{
				sut->UseDerivedValues(true);
				sut->TableFormula->CallCacheValues();

				checkValue(-1, 0.0);
				checkValue(0, 2.0);
				checkValue(0.5, 2.0);
				checkValue(1, 0.25);
				checkValue(3, 0.25);
				checkValue(20, 0.0);
				checkValue(30, 0.0);
				checkValue(40.5, 2.0);
				checkValue(41, 0.0);
				checkValue(50, 0.0);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
		}

		[TestAttribute]
        void should_return_empty_restart_timepoints_set()
		{
			std::vector<double> & restartTimePoints = sut->Formula->RestartTimePoints();
			BDDExtensions::ShouldBeTrue(restartTimePoints.size()==0);
		}
    };

}