#ifdef _WINDOWS
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/TableFormula.h"
#include "SimModel/SimModelTypeDefs.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModelSpecs/TableFormulaSpecsHelper.h"

#include <math.h>

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework; 
	using namespace SimModelNET;

	ref class TableFormulaWrapper
	{
	public:
		TableFormulaExtender * Formula;
		TableFormulaWrapper()
		{
			Formula=new TableFormulaExtender();
		}
		~TableFormulaWrapper(){delete Formula;}
		double Calculate(double x)
		{
			return Formula->DE_Compute(NULL, x, SimModelNative::USE_SCALEFACTOR);
		}
	};

	public ref class concern_for_table_formula abstract : ContextSpecification<TableFormulaWrapper^>
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
            sut=gcnew TableFormulaWrapper();
        }
    };

	public ref class when_creating_for_given_table : public concern_for_table_formula
    {
	protected:   

		virtual void Because() override
        {
			SimModelNative::TObjectVector <SimModelNative::ValuePoint> & ValuePoints = sut->Formula->ValuePoints();
			ValuePoints.push_back(new SimModelNative::ValuePoint(0,0,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(1,2,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(5,3,true));
			ValuePoints.push_back(new SimModelNative::ValuePoint(20,6,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(40,6,false));
			ValuePoints.push_back(new SimModelNative::ValuePoint(41,8,false));

			sut->Formula->CallCacheValues();
        }

    public:
        [TestAttribute]
        void should_calculate_correct_value()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(-1), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0.5), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(1), 0.25);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(3), 0.25);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(5), 0.2);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(10), 0.2);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(20), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(30), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(40), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(40.5), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(42), 0.0);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
		
		[TestAttribute]
        void should_set_restart_timepoints()
		{
			std::vector<double> & restartTimePoints = sut->Formula->RestartTimePoints();

			BDDExtensions::ShouldBeTrue(restartTimePoints.size()==3);

			BDDExtensions::ShouldBeEqualTo(restartTimePoints[0], 0.0);
			BDDExtensions::ShouldBeEqualTo(restartTimePoints[1], 5.0);
			BDDExtensions::ShouldBeEqualTo(restartTimePoints[2], 40.0);
		}
    };

}