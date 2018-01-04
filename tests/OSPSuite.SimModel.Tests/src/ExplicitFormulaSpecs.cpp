#ifdef _WINDOWS
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelSpecs/ExplicitFormulaSpecsHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/ExplicitFormula.h"
#include "SimModel/SimModelTypeDefs.h"
#include "XMLWrapper/XMLHelper.h"

#include <math.h>

namespace UnitTests
{
    using namespace OSPSuite::BDDHelper;
    using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework; 
	using namespace SimModelNET;

	ref class ExplicitFormulaWrapper
	{
	public:
		ExplicitFormulaExtender * Formula;
		ExplicitFormulaWrapper(){Formula=new ExplicitFormulaExtender();}
		~ExplicitFormulaWrapper(){delete Formula;}
	};

	public ref class concern_for_explicit_formula abstract : ContextSpecification<ExplicitFormulaWrapper^>
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
            sut=gcnew ExplicitFormulaWrapper();
        }
    };

	public ref class when_creating_for_references_independent_equation : public concern_for_explicit_formula
    {
	protected:   
		static const double pi=2*asin(1.0);

		double _value;

		virtual void Because() override
        {
			sut->Formula->SetEquation("sin(pi/2)*sqrt(4.0)");
			sut->Formula->Finalize();

			_value=sut->Formula->DE_Compute(NULL, 0.0, SimModelNative::USE_SCALEFACTOR);
        }

    public:
        [TestAttribute]
        void should_calculate_correct_value()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(_value, sin(pi/2)*sqrt(4.0), 1e-10);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };


	public ref class when_creating_for_parameter_and_species_references : public concern_for_explicit_formula
	{
	protected:
		double p1, p2, x, y, _value;
		SpeciesExtender * X, * Y;

		virtual void Because() override
        {
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;
				f->SetEquation("p1*x+p2*y");
				
				p1=2; p2=3; x=4; y=5;
				_value = p1*x+p2*y;

				f->AddParameterReference("p1", XMLHelper::ToString(p1));
				f->AddParameterReference("p2", XMLHelper::ToString(p2));

				X = f->AddSpeciesReference("x","-1"); //x value doesn't matter
				Y = f->AddSpeciesReference("y","-1"); //y value doesn't matter

				X->SetIsChangedBySwitch(true); //force using as variable, not as parameter
				Y->SetIsChangedBySwitch(true); //force using as variable, not as parameter

				X->SetODEIndex(0);
				Y->SetODEIndex(1);
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
	public:
        [TestAttribute]
        void should_calculate_correct_value_without_simplifying()
		{
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;
				f->Finalize();

				double yy[2] = {x, y};
				double value=f->DE_Compute(yy, 0.0, SimModelNative::USE_SCALEFACTOR);

				BDDExtensions::ShouldBeEqualTo(value, _value);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}			
		}

		[TestAttribute]
        void should_calculate_correct_value_with_simplifying()
		{
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;
				
				BDDExtensions::ShouldBeFalse(f->Simplify(false));

				f->Finalize();

				double yy[2] = {x, y};
				double value=f->DE_Compute(yy, 0.0, SimModelNative::USE_SCALEFACTOR);

				BDDExtensions::ShouldBeEqualTo(value, _value);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}			
		}
	};

	public ref class when_creating_for_parameter_species_and_time : public concern_for_explicit_formula
	{
	protected:
		double p2, x, y;
		SpeciesExtender * X, * Y;
		ExplicitFormulaExtender * p1Formula;

		// p1=Time+x
		// p2=const
		//f(p1, p2, x, y) = p1*x+p2*y 
		virtual void Because() override
        {
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;
				f->SetEquation("p1*x+p2*y");
				
				p2=3; x=4; y=5;

				X = f->AddSpeciesReference("x","-1"); //x value doesn't matter
				Y = f->AddSpeciesReference("y","-1"); //y value doesn't matter

				X->SetIsChangedBySwitch(true); //force using as variable, not as parameter
				Y->SetIsChangedBySwitch(true); //force using as variable, not as parameter

				X->SetODEIndex(0);
				Y->SetODEIndex(1);

				p1Formula = new ExplicitFormulaExtender();
				p1Formula->AddTimeReference();
				p1Formula->AddSpeciesReference(X); 
				p1Formula->SetEquation(SimModelNative::csTime+string("+x"));

				f->AddParameterReference("p1", p1Formula);
				f->AddParameterReference("p2", XMLHelper::ToString(p2));
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
	public:
        [TestAttribute]
        void should_calculate_correct_value_without_simplifying()
		{
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;

				p1Formula->Finalize();
				f->Finalize();

				double yy[2] = {x, y}, time = 17.5;

				double p1 = p1Formula->DE_Compute(yy, time, SimModelNative::USE_SCALEFACTOR);
				BDDExtensions::ShouldBeEqualTo(p1, time+x);

				double value=f->DE_Compute(yy, time, SimModelNative::USE_SCALEFACTOR);
				BDDExtensions::ShouldBeEqualTo(value, p1*x+p2*y);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}			
		}

		[TestAttribute]
        void should_calculate_correct_value_with_simplifying()
		{
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;

				BDDExtensions::ShouldBeFalse(p1Formula->Simplify(false));
				BDDExtensions::ShouldBeFalse(f->Simplify(false));

				p1Formula->Finalize();
				f->Finalize();

				double yy[2] = {x, y}, time = 17.5;

				double p1 = p1Formula->DE_Compute(yy, time, SimModelNative::USE_SCALEFACTOR);
				BDDExtensions::ShouldBeEqualTo(p1, time+x);

				double value=f->DE_Compute(yy, time, SimModelNative::USE_SCALEFACTOR);
				BDDExtensions::ShouldBeEqualTo(value, p1*x+p2*y);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}			
		}
	};

	public ref class when_creating_product_formula : public concern_for_explicit_formula
	{
	protected:
		double p1, p2, x, y, _value;
		SpeciesExtender * X, * Y;

		// p1=1+x-y
		// p2=const
		//f(p1, p2, x, y) = p1*x*p2*y (= p2*(1+x-y)*x*y )
		virtual void Because() override
        {
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;
				f->SetEquation("p1*x*p2*y");
				
				p2=3; x=4; y=5;

				X = f->AddSpeciesReference("x","-1"); //x value doesn't matter
				Y = f->AddSpeciesReference("y","-1"); //y value doesn't matter

				X->SetODEIndex(0);
				Y->SetODEIndex(1);
				X->SetIsChangedBySwitch(true); //force using as variable, not as parameter
				Y->SetIsChangedBySwitch(true); //force using as variable, not as parameter

				ExplicitFormulaExtender * p1Formula = new ExplicitFormulaExtender();
				p1Formula->AddSpeciesReference(X); 
				p1Formula->AddSpeciesReference(Y); 
				p1Formula->SetEquation("1+x-y");

				f->AddParameterReference("p1", p1Formula);
				f->AddParameterReference("p2", XMLHelper::ToString(p2));

				p1Formula->Finalize();
				f->Finalize();
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

	public:
	};

	public ref class when_getting_switch_timepoints : public concern_for_explicit_formula
	{
	protected:
		ExplicitFormulaExtender * p1Formula, * p2Formula;

		virtual void Because() override
        {
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;

				//Setup for returning time points 1,2,3,4,5,6,7
				f->SetEquation("((p1>=2) AND (Time>3)) OR ((4=Time) OR (Time !=2) OR (Time<>1)) AND NOT ((Time>=p1) OR (Time<p2) OR (Time<=5) OR (Time<=x))");
				f->AddTimeReference();

				SpeciesExtender * X = f->AddSpeciesReference("x","-1"); //x value doesn't matter
				X->SetIsChangedBySwitch(true); //force using as variable, not as parameter

				p1Formula = new ExplicitFormulaExtender();
				p1Formula->SetEquation("6");

				f->AddParameterReference("p1", p1Formula, true);

				p2Formula = new ExplicitFormulaExtender();
				p2Formula->SetEquation("7");
				f->AddParameterReference("p2", p2Formula, true);
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
	public:
        [TestAttribute]
        void should_return_correct_switch_timepoints()
		{
			try
			{
				ExplicitFormulaExtender * f = sut->Formula;
				p1Formula->Simplify(false);
				p2Formula->Simplify(false);
				f->Simplify(false);

				p1Formula->Finalize();
				p2Formula->Finalize();
				f->Finalize();

				vector<double> switchTimePoints=f->SwitchTimePoints();

				BDDExtensions::ShouldBeEqualTo(switchTimePoints.size(), (size_t)7);

				DoubleQueue dq;
				
				unsigned int i;
				for (i=0; i<switchTimePoints.size(); i++)
					dq.push(switchTimePoints[i]);

				for (i=1; i<=7; i++)
				{
					BDDExtensions::ShouldBeEqualTo(dq.top(), (double)i);
					dq.pop();
				}
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}			
		}

	};

}