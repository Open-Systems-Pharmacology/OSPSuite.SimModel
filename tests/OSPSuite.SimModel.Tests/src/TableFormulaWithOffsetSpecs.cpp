#ifdef _WINDOWS
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/TableFormula.h"
#include "SimModel/Quantity.h"
#include "SimModel/TableFormulaWithOffset.h"
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
	class TableFormulaWithOffsetExtender : public SimModelNative::TableFormulaWithOffset
	{
	public:
		void SetTableObject(SimModelNative::Quantity * tableObject){_tableObject = tableObject;}
		void SetOffsetObject(SimModelNative::Quantity * offsetObject){_offsetObject = offsetObject;}
		void SetTableFormula(SimModelNative::TableFormula * tableFormula){_tableFormula = tableFormula;}
	};

	ref class TableFormulaWithOffsetWrapper
	{
	public:
		TableFormulaWithOffsetExtender * Formula;
		TableFormulaExtender * TableFormula;
		
		TableFormulaWithOffsetWrapper()
		{
			Formula=new TableFormulaWithOffsetExtender();
			TableFormula = new TableFormulaExtender;
		}
		~TableFormulaWithOffsetWrapper(){delete Formula; delete TableFormula;}
		double Calculate(double x)
		{
			return Formula->DE_Compute(NULL, x, SimModelNative::USE_SCALEFACTOR);
		}
	};

	public ref class concern_for_table_with_offset_formula abstract : ContextSpecification<TableFormulaWithOffsetWrapper^>
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
            sut=gcnew TableFormulaWithOffsetWrapper();
        }
    };

	public ref class when_creating_for_given_table_with_offset : public concern_for_table_with_offset_formula
    {
	protected:   
			double _offset;
			ParameterExtender * _offsetObject;
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
			
			_offset = 300.0;

			_offsetObject = new ParameterExtender();
			_offsetObject->SetConstantValue(_offset);
			sut->Formula->SetOffsetObject(_offsetObject);
			sut->Formula->SetTableFormula(sut->TableFormula);

			_tableObject = new ParameterExtender();
			_tableObject->SetFormula(sut->TableFormula);
			sut->Formula->SetTableObject(_tableObject);
        }

    public:
        [TestAttribute]
        void should_calculate_correct_value()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(-1+_offset), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0+_offset), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(0.5+_offset), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(1+_offset), 0.25);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(3+_offset), 0.25);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(5+_offset), 0.2);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(10+_offset), 0.2);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(20+_offset), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(30+_offset), 0.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(40+_offset), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(40.5+_offset), 2.0);
				BDDExtensions::ShouldBeEqualTo(sut->Calculate(42+_offset), 0.0);
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

			BDDExtensions::ShouldBeEqualTo(restartTimePoints[0], 0.0+_offset);
			BDDExtensions::ShouldBeEqualTo(restartTimePoints[1], 5.0+_offset);
			BDDExtensions::ShouldBeEqualTo(restartTimePoints[2], 40.0+_offset);
		}
    };

}