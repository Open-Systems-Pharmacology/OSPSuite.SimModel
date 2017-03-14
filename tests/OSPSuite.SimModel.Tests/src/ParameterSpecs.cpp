#ifdef WIN32
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/Parameter.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/ExplicitFormula.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/TableFormula.h"

#include <math.h>

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework; 
	using namespace SimModelNET;

	ref class ParameterWrapper
	{
	public:
		SimModelNative::Parameter * Parameter;
		ParameterWrapper()
		{
			Parameter=new SimModelNative::Parameter();
		}
		~ParameterWrapper(){delete Parameter;}
		double Calculate(double time)
		{
			return Parameter->GetValue(NULL, time, SimModelNative::USE_SCALEFACTOR);
		}

		void SetFormula(SimModelNative::Formula * formula){Parameter->SetFormula(formula);}
		void SetTablePoints(const std::vector <SimModelNative::ValuePoint> & valuePoints){Parameter->SetTablePoints(valuePoints);}
		void SetInitialValue(double value){Parameter->SetInitialValue(value);}
		bool IsTable(){return Parameter->IsTable();}
	};

	public ref class concern_for_parameter abstract : ContextSpecification<ParameterWrapper^>
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
            sut=gcnew ParameterWrapper();
        }
    };

	public ref class when_setting_table_points_into_nontable_parameter : public concern_for_parameter
    {
	protected:   

		virtual void Because() override
        {
			sut->SetFormula(new SimModelNative::ExplicitFormula());

			std::vector <SimModelNative::ValuePoint> ValuePoints;

			ValuePoints.push_back(SimModelNative::ValuePoint(0,0,false));
			ValuePoints.push_back(SimModelNative::ValuePoint(1,2,false));
			ValuePoints.push_back(SimModelNative::ValuePoint(5,3,true));
			ValuePoints.push_back(SimModelNative::ValuePoint(20,6,false));
			ValuePoints.push_back(SimModelNative::ValuePoint(40,6,false));
			ValuePoints.push_back(SimModelNative::ValuePoint(41,8,false));

			sut->SetTablePoints(ValuePoints);
        }

    public:

		[TestAttribute]
		void parameter_should_be_table()
		{
			BDDExtensions::ShouldBeTrue(sut->IsTable());
		}

        [TestAttribute]
        void should_calculate_correct_value()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(-1), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0.5), 1.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(1), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(3), 2.5);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(5), 3.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(10), 4.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(20), 6.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(30), 6.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(40), 6.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(40.5), 7.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(42), 8.0);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
		
    };

	public ref class when_setting_initial_value_into_table_parameter : public concern_for_parameter
    {
	protected:   

		static const double initialValue = 999;

		virtual void Because() override
        {
			sut->SetFormula(new SimModelNative::TableFormula());

			std::vector <SimModelNative::ValuePoint> ValuePoints;

			ValuePoints.push_back(SimModelNative::ValuePoint(0,0,false));
			ValuePoints.push_back(SimModelNative::ValuePoint(1,2,false));

			sut->SetTablePoints(ValuePoints);

			sut->SetInitialValue(initialValue);
        }

    public:

		[TestAttribute]
		void parameter_should_not_be_table()
		{
			BDDExtensions::ShouldBeFalse(sut->IsTable());
		}

        [TestAttribute]
        void should_calculate_correct_value()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(-1), initialValue);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0), initialValue);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(2), initialValue);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
		
    };

}