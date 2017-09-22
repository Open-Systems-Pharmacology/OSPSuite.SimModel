#ifdef WIN32
#pragma warning( disable : 4691)
#pragma warning( disable : 4244)
#endif

#include "SimModelManaged/ManagedSimulation.h"
#include "SimModel/Simulation.h"
#include "SimModel/SwitchTask.h"
#include "SimModelManaged/XMLSchemaCache.h"
#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/SimModelException.h"
#include "SimModelManaged/ManagedOutputSchema.h"
#include "SimModelSpecs/SpecsHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/MathHelper.h"
#include "SimModel/MatlabODEExporter.h"
#include "XMLWrapper/XMLHelper.h"
#include <fstream>

namespace SimulationTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace SimModelNET;
	using namespace UnitTests;

	public ref class concern_for_simulation abstract : ContextSpecification<SimModelNET::Simulation^>
    {
    public:
		virtual void GlobalContext() override
		{
			try
			{
				CoInitialize(NULL);
				XMLSchemaCache::InitializeFromFile(SpecsHelper::SchemaPath());
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
            sut=gcnew Simulation();
        }

		void CheckSolverOutputTimes()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();

			double * solverTimes = sim->GetTimeValues();
			DoubleQueue timeSchema = sut->GetNativeSimulation()->GetOutputSchema().AllTimePoints<float>();

			BDDExtensions::ShouldBeEqualTo<int>(timeSchema.size(), noOfOutputtimePoints);

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				BDDExtensions::ShouldBeEqualTo(solverTimes[i], timeSchema.top());
				timeSchema.pop();
			}
		}

		void EnableBandLinearSolver()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			sim->SetUseBandLinearSolver(true);
		}

		void CheckBandLinearSolverEnabled()
		{
			BDDExtensions::ShouldBeTrue(sut->GetNativeSimulation()->UseBandLinearSolver());
		}

		void DisableBandLinearSolver()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			sim->SetUseBandLinearSolver(false);
		}

		void CheckBandLinearSolverDisabled()
		{
			BDDExtensions::ShouldBeFalse(sut->GetNativeSimulation()->UseBandLinearSolver());
		}

		IParameterProperties^ GetParameterByPath(IList<IParameterProperties^>^ parameterProperties, System::String^ path)
        {
			for each (IParameterProperties^ param in parameterProperties)
			{
				if (param->Path == path)
					return param;
			}

			return nullptr;
        }

		ISpeciesProperties^ GetSpeciesByPath(IList<ISpeciesProperties^>^ speciesProperties, System::String^ path)
		{
			for each (ISpeciesProperties^ species in speciesProperties)
			{
				if (species->Path == path)
					return species;
			}

			return nullptr;
		}
    };

    
	public ref class when_loading_from_file : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_be_loaded_from_file()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput05.xml"));
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
    };

    
	public ref class when_loading_from_string : public concern_for_simulation
    {
	protected: 
		System::String^ _string2Load;

		virtual void Because() override
        {
			XMLDocument xmlDoc = XMLDocument::FromFile(NETToCPPConversions::MarshalString(SpecsHelper::TestFileFrom("SimModel4_ExampleInput05.xml")));
			_string2Load = CPPToNETConversions::MarshalString(xmlDoc.ToString());
        }

    public:
        [TestAttribute]
        void should_be_loaded_from_file()
        {
			try
			{
				sut->LoadFromXMLString(_string2Load);
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
    };

    
	public ref class when_loading_and_finalizing_from_file : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_be_loaded_and_finalized_from_file()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput05.xml"));
				sut->FinalizeSimulation();
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
    };


    
	public ref class when_loading_finalizing_and_running : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput05.xml"));
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
    };

    
	public ref class when_loading_finalizing_and_running_band_solver : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput05.xml"));
				EnableBandLinearSolver();
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
    };

	public ref class when_running_system_with_all_constant_species_base abstract : public concern_for_simulation
	{
	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput04.xml"));
				OptionalTasksBeforeFinalize();
				sut->FinalizeSimulation();
				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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

		virtual void OptionalTasksBeforeFinalize()
		 {
		 }

		void checkSpeciesValues()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			SimModelNative::TObjectList<SimModelNative::Species> allSpecies = sim->SpeciesList();

			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();

			for(int speciesIdx=0; speciesIdx<allSpecies.size(); speciesIdx++)
			{
				SimModelNative::Species * species = allSpecies[speciesIdx];
				double initialValue = species->GetValue(NULL, sim->GetStartTime(), SimModelNative::IGNORE_SCALEFACTOR);

				double * speciesValues = species->GetValues();

				BDDExtensions::ShouldBeEqualTo(species->GetValuesSize(), 1);

				for(int timePointIdx=0; timePointIdx<species->GetValuesSize(); timePointIdx++)
					BDDExtensions::ShouldBeEqualTo(speciesValues[timePointIdx],
					                               initialValue);
			}
		}
	};

    
	public ref class when_running_system_with_all_constant_species_dense : public when_running_system_with_all_constant_species_base
    {
	protected:
		virtual void OptionalTasksBeforeFinalize() override
		{
			DisableBandLinearSolver();
		}

    public:
        [TestAttribute]
        void solver_output_times_should_be_equal_to_output_schema()
        {
			CheckSolverOutputTimes();
        }

		[TestAttribute]
        void species_values_should_be_equal_to_initial_value()
		{
			checkSpeciesValues();
			CheckBandLinearSolverDisabled();
		}
    };

    
	public ref class when_running_system_with_all_constant_species_band : public when_running_system_with_all_constant_species_base
    {
	protected:
		virtual void OptionalTasksBeforeFinalize() override
		{
			EnableBandLinearSolver();
		}

    public:
        [TestAttribute]
        void solver_output_times_should_be_equal_to_output_schema()
        {
			CheckSolverOutputTimes();
        }

		[TestAttribute]
        void species_values_should_be_equal_to_initial_value()
		{
			CheckBandLinearSolverEnabled();
			checkSpeciesValues();
		}
    };

	
	public ref class when_running_testsystem_06 abstract : public concern_for_simulation
	{
		//---- solving the system:
		//
		// y1' = (P1+P2)*y2 + (P3-Time)*y1 + (y3-2)
		// y2' = y1 + P4
		// y3' = 0 + 0
		//
		// y1(0) = 2
		// y2(0) = P1 + P2 -1
		// y3(0) = y1
		//
		// P1 = sin(y3)^2
		// P2 = cos(y3)^2
		// P3 = Time
		// P4 = y3 - 2
		//
		// System is equivalent to:
		//
		// y1' = y2   y1(0) = 2
		// y2' = y1   y2(0) = 0
		// y3' = 0    y3(0) = 2
		//
		// Analytical solution is:
		//
		// y1 = exp(Time) + exp(-Time)
		// y2 = exp(Time) - exp(-Time)
		// y3 = 2
		//
		// Additionaly, an Observer Obs1 is defined as Obs1 = 2*y1
	protected:
		void TestResult()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();
			DoubleQueue timeSchema = sut->GetNativeSimulation()->GetOutputSchema().AllTimePoints<float>();

			BDDExtensions::ShouldBeEqualTo<int>(timeSchema.size(), noOfOutputtimePoints);

			SimModelNative::Species * y1, * y2, * y3;
			y1 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y1");
			y2 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y2");
			y3 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y3");

			BDDExtensions::ShouldBeEqualTo(y1->GetValuesSize(), noOfOutputtimePoints);
			BDDExtensions::ShouldBeEqualTo(y2->GetValuesSize(), noOfOutputtimePoints);

			double * y1_Values = y1->GetValues();
			double * y2_Values = y2->GetValues();

			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			double * y3_Values = y3->GetValues();
			BDDExtensions::ShouldBeTrue(y3->IsConstantDuringCalculation());
			BDDExtensions::ShouldBeEqualTo(y3->GetValuesSize(), 1);
			BDDExtensions::ShouldBeEqualTo(y3_Values[0], 2.0, relTol);

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				double time = solverTimes[i];

				BDDExtensions::ShouldBeEqualTo(time, timeSchema.top());
				timeSchema.pop();

				BDDExtensions::ShouldBeEqualTo(y1_Values[i], exp(time)+exp(-time), relTol);
				BDDExtensions::ShouldBeEqualTo(y2_Values[i], exp(time)-exp(-time), relTol);
			}
		}
	};

	public ref class when_getting_all_parameter_values_and_all_initial_values: public concern_for_simulation
	{

	public:
		[TestAttribute]
		void should_return_correct_value_for_dependent_parameters_and_initial_value_before_and_after_changing_of_basis_parameter()
		{
			sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("TestAllParametersInitialValues.xml"));

			//---- set P1 and P2 as variable
			IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

			variableParams->Add(GetParameterByPath(sut->ParameterProperties, "P1"));
			variableParams->Add(GetParameterByPath(sut->ParameterProperties, "P2"));
			sut->VariableParameters = variableParams;

			sut->FinalizeSimulation();

			//value of P10 should be equal P1+P2, which is initially 1
			BDDExtensions::ShouldBeEqualTo(GetParameterByPath(sut->ParameterProperties, "P10")->Value, 1.0, 1e-5);

			//initial value of y2 should be equal P1+P2-1, which is initially 0
			BDDExtensions::ShouldBeEqualTo(GetSpeciesByPath(sut->SpeciesProperties, "y2")->Value, 0.0, 1e-5);

			//update variable parameters
			variableParams = sut->VariableParameters;
			GetParameterByPath(variableParams, "P1")->Value = 3;
			GetParameterByPath(variableParams, "P2")->Value = 4;

			//---- set new parameter values
			sut->SetParameterValues(variableParams);

			//value of P10 should be equal P1+P2, which is now 7
			BDDExtensions::ShouldBeEqualTo(GetParameterByPath(sut->ParameterProperties, "P10")->Value, 7.0, 1e-5);

			//initial value of y2 should be equal P1+P2-1, which is now 6
			BDDExtensions::ShouldBeEqualTo(GetSpeciesByPath(sut->SpeciesProperties, "y2")->Value, 6.0, 1e-5);
		}
	};

	public ref class when_running_testsystem_06_without_scalefactor_dense : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml"));
				
				DisableBandLinearSolver();
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
			CheckBandLinearSolverDisabled();
        }

		[TestAttribute]
		void should_calculate_comparison_threshold()
        {
			//get absolute tolerance used for calculation (might differ from input absolute tolerance)
			const double AbsTol = sut->UsedAbsoluteTolerance;

			//expected threshold for ode variables
			const double threshold = 10.0 * AbsTol;

			IList<IValues^>^ allValues = sut->AllValues;
			for each(IValues^ values in allValues)
			{
				if (values->VariableType == VariableTypes::Observer)
					//the (only) observer is defined as 2*y1 and thus must retrieve the threshold 2*Threshold(y1)
					BDDExtensions::ShouldBeEqualTo(values->ComparisonThreshold, 2.0*threshold); 
				else
					BDDExtensions::ShouldBeEqualTo(values->ComparisonThreshold, threshold);
			}
        }
    };

   
	public ref class when_running_testsystem_06_without_scalefactor_band : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml"));
				EnableBandLinearSolver();
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			CheckBandLinearSolverEnabled();
			TestResult();
        }
    };



   
	public ref class when_running_testsystem_06_with_scalefactor_dense : public when_running_testsystem_06
    {

	protected:   
		 SimModelNative::Species * _y1, *_y2, *_y3;
		 double _scaleFactor = 10.0;
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml"));
				
				DisableBandLinearSolver();

				_y1 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y1");
				_y2 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y2");
				_y3 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y3");

				_y1->SetODEScaleFactor(_scaleFactor);
				_y2->SetODEScaleFactor(_scaleFactor);
				_y3->SetODEScaleFactor(_scaleFactor);
				
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
			CheckBandLinearSolverDisabled();
        }

		[TestAttribute]
		void should_calculate_comparison_threshold()
		{
			//get absolute tolerance used for calculation (might differ from input absolute tolerance)
			const double AbsTol = sut->UsedAbsoluteTolerance;

			//expected threshold for ode variables
			const double threshold = 10.0 * AbsTol;

			BDDExtensions::ShouldBeEqualTo(_y1->GetComparisonThreshold(), threshold * _scaleFactor);
			BDDExtensions::ShouldBeEqualTo(_y2->GetComparisonThreshold(), threshold * _scaleFactor);

			//scale factor is ignored for constant variables!
			BDDExtensions::ShouldBeEqualTo(_y3->GetComparisonThreshold(), threshold);

			//the (only) observer is defined as 2*y1 and thus must retrieve the threshold 2*Threshold(y1)
			SimModelNative::Observer * obs1 = sut->GetNativeSimulation()->Observers().GetObjectByEntityId("Obs1");
			BDDExtensions::ShouldBeEqualTo(obs1->GetComparisonThreshold(), 2.0 * _y1->GetComparisonThreshold());
		}
    };

   
	public ref class when_running_testsystem_06_with_scalefactor_band : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml"));

				SimModelNative::Species * y1, * y2, * y3;
				y1 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y1");
				y2 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y2");
				y3 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y3");

				y1->SetODEScaleFactor(10.0);
				y2->SetODEScaleFactor(10.0);
				y3->SetODEScaleFactor(10.0);
				
				EnableBandLinearSolver();
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			CheckBandLinearSolverEnabled();
			TestResult();
        }
    };

   
	public ref class when_running_testsystem_06_setting_all_parameters_as_variable_dense : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml"));
				DisableBandLinearSolver();

				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				
				//---- set all parameters as variable
				std::vector<SimModelNative::ParameterInfo> params;
				sim->FillParameterProperties(params);
				sim->SetVariableParameters(params);

				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
			CheckBandLinearSolverDisabled();
        }
    };

   
	public ref class when_running_testsystem_06_setting_all_parameters_as_variable_band : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml"));
				
				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				
				//---- set all parameters as variable
				std::vector<SimModelNative::ParameterInfo> params;
				sim->FillParameterProperties(params);
				sim->SetVariableParameters(params);

				EnableBandLinearSolver();
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			CheckBandLinearSolverEnabled();
			TestResult();
        }
    };

   
	public ref class when_running_testsystem_06_new_schema_without_scalefactor : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_NewSchema.xml"));
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
        }
    };

   
	public ref class when_running_testsystem_06_new_schema_with_scalefactor : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_NewSchema.xml"));

				SimModelNative::Species * y1, * y2, * y3;
				y1 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y1");
				y2 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y2");
				y3 = sut->GetNativeSimulation()->SpeciesList().GetObjectByEntityId("y3");

				y1->SetODEScaleFactor(10.0);
				y2->SetODEScaleFactor(10.0);
				y3->SetODEScaleFactor(10.0);
				
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
        }
    };

   
	public ref class when_running_testsystem_06_modified_setting_parameter_values_and_initial_values : public when_running_testsystem_06
    {
		//Modifications made in the system (compared to the test system) in XML
		// y2(0) changed from P1 + P2 -1 (=0) to 10
		// P1 changed from sin(y3)^2
		// P2 changed from cos(y3)^2
		//
		// In order to get the original system, one must set:
		// y2(0) = 0
		// P1+P2=1
	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_Modified.xml"));
				
				//---- set P1 and P2 as variable
				IList<IParameterProperties^>^ paramProps = sut->ParameterProperties;
				IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

				variableParams->Add(GetParameterByPath(paramProps, "Subcontainer1/P1"));
				variableParams->Add(GetParameterByPath(paramProps, "Subcontainer1/P2"));
				sut->VariableParameters=variableParams;

				//---- set y2 initial value as variable
				IList<ISpeciesProperties^>^ speciesProps = sut->SpeciesProperties;
				IList<ISpeciesProperties^>^ variableSpecies = gcnew System::Collections::Generic::List<ISpeciesProperties^>();

				variableSpecies->Add(GetSpeciesByPath(speciesProps, "Subcontainer1/y2"));
				sut->VariableSpecies=variableSpecies;

				sut->FinalizeSimulation();

				//update variable parameters/species lists
				variableParams = sut->VariableParameters;
				variableSpecies = sut->VariableSpecies;

				//---- set y2(0)=0 and P1+P2=1
				GetParameterByPath(variableParams, "Subcontainer1/P1")->Value = 0.3;
				GetParameterByPath(variableParams, "Subcontainer1/P2")->Value = 0.7;
				GetSpeciesByPath(variableSpecies,  "Subcontainer1/y2")->Value = 0;
				
				//---- set new parameter values and species initial values
				sut->SetParameterValues(variableParams);
				sut->SetSpeciesProperties(variableSpecies);

				//---- run simulation with new values
				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
        }
    };

   
	public ref class when_running_testsystem_06_modified_setting_table_parameter_values : public when_running_testsystem_06
    {
		//in the model, y4 is defined as following:
		//
		// y4(0) = 33
		// y4'   = P5
		// P5 is table parameter const 0
	protected:
		array<double>^ _y4_without_change;
		array<double>^ _y4_with_change;
		 virtual void Because() override
        {
			try
			{
				String^ file = SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_Modified.xml");

				//---- run without parameter modification
				sut->LoadFromXMLFile(file);
				sut->FinalizeSimulation();
				sut->RunSimulation();

				_y4_without_change=sut->ValuesFor("y4")->Values;

				//---- run with table parameter modification
				sut->LoadFromXMLFile(file);
				
				//---- set P5 as variable
				IList<IParameterProperties^>^ paramProps = sut->ParameterProperties;
				IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

				variableParams->Add(GetParameterByPath(paramProps, "Subcontainer1/P5"));
				sut->VariableParameters=variableParams;

				sut->FinalizeSimulation();

				//update variable parameters/species lists
				variableParams = sut->VariableParameters;

				//---- set P5 as table:
				//   0,  0
				//   5,  10
				//   10, 10
				//After this changes:
				//
				//y4'(t) = 2   for 0<=t<=5
				//y4'(t) = 0   for t>5
				//
				//=> y4(t)=y4(0)+2*t for 0<=t<=5
				//   y4(t)=y4(5)=y4(0)+10 for t>=5
				IList <IValuePoint^ >^ tablePoints=GetParameterByPath(variableParams, "Subcontainer1/P5")->TablePoints;
				tablePoints->Clear();
				tablePoints->Add(gcnew ValuePoint(0,0,false));
				tablePoints->Add(gcnew ValuePoint(5,10,false));
				tablePoints->Add(gcnew ValuePoint(10,10,false));
				
				//---- set new parameter values and species initial values
				sut->SetParameterValues(variableParams);

				//---- run simulation with new values
				sut->RunSimulation();

				_y4_with_change=sut->ValuesFor("y4")->Values;

			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result_without_changing_table_parameter()
        {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();

			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			for(int i=0; i<noOfOutputtimePoints; i++)
				BDDExtensions::ShouldBeEqualTo(_y4_without_change[i], _y4_without_change[0], relTol);
        }

		[TestAttribute]
        void should_produce_correct_result_with_changing_table_parameter()
        {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();

			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				double time = solverTimes[i];

				double expectedValue = _y4_with_change[0]+ (time <=5 ? 2*time : 10);
				BDDExtensions::ShouldBeEqualTo(_y4_with_change[i], expectedValue, relTol);
			}			
        }
    };


   
	public ref class when_running_testsystem_06_V4_modified_setting_parameter_values_and_initial_values : public when_running_testsystem_06
    {
		//Modifications made in the system (compared to the test system) in XML
		// y2(0) changed from P1 + P2 -1 (=0) to 10
		// P1 changed from sin(y3)^2
		// P2 changed from cos(y3)^2
		//
		// In order to get the original system, one must set:
		// y2(0) = 0
		// P1+P2=1
	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_Modified_V4.xml"));
				
				//---- set P1 and P2 as variable
				IList<IParameterProperties^>^ paramProps = sut->ParameterProperties;
				IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

				variableParams->Add(GetParameterByPath(paramProps, "Subcontainer1/P1"));
				variableParams->Add(GetParameterByPath(paramProps, "Subcontainer1/P2"));
				sut->VariableParameters=variableParams;

				//---- set y2 initial value as variable
				IList<ISpeciesProperties^>^ speciesProps = sut->SpeciesProperties;
				IList<ISpeciesProperties^>^ variableSpecies = gcnew System::Collections::Generic::List<ISpeciesProperties^>();

				variableSpecies->Add(GetSpeciesByPath(speciesProps, "Subcontainer1/y2"));
				sut->VariableSpecies=variableSpecies;

				sut->FinalizeSimulation();

				//update variable parameters/species lists
				variableParams = sut->VariableParameters;
				variableSpecies = sut->VariableSpecies;

				//---- set y2(0)=0 and P1+P2=1
				GetParameterByPath(variableParams, "Subcontainer1/P1")->Value = 0.3;
				GetParameterByPath(variableParams, "Subcontainer1/P2")->Value = 0.7;
				GetSpeciesByPath(variableSpecies,  "Subcontainer1/y2")->Value = 0;
				
				//---- set new parameter values and species initial values
				sut->SetParameterValues(variableParams);
				sut->SetSpeciesProperties(variableSpecies);

				//---- run simulation with new values
				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
        }
    };

   
	public ref class when_running_testsystem_06_V4_modified_setting_table_parameter_values : public when_running_testsystem_06
    {
		//in the model, y4 is defined as following:
		//
		// y4(0) = 33
		// y4'   = P5
		// P5 is table parameter const 0
	protected:
		array<double>^ _y4_without_change;
		array<double>^ _y4_with_change;
		 virtual void Because() override
        {
			try
			{
				String^ file = SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_Modified_V4.xml");

				//---- run without parameter modification
				sut->LoadFromXMLFile(file);
				sut->FinalizeSimulation();
				sut->RunSimulation();

				_y4_without_change=sut->ValuesFor("y4")->Values;

				//---- run with table parameter modification
				sut->LoadFromXMLFile(file);
				
				//---- set P5 as variable
				IList<IParameterProperties^>^ paramProps = sut->ParameterProperties;
				IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

				variableParams->Add(GetParameterByPath(paramProps, "Subcontainer1/P5"));
				sut->VariableParameters=variableParams;

				sut->FinalizeSimulation();

				//update variable parameters/species lists
				variableParams = sut->VariableParameters;

				//---- set P5 as table:
				//   0,  0
				//   5,  10
				//   10, 10
				//After this changes:
				//
				//y4'(t) = 2   for 0<=t<=5
				//y4'(t) = 0   for t>5
				//
				//=> y4(t)=y4(0)+2*t for 0<=t<=5
				//   y4(t)=y4(5)=y4(0)+10 for t>=5
				IList <IValuePoint^ >^ tablePoints = GetParameterByPath(variableParams, "Subcontainer1/P5")->TablePoints;
				tablePoints->Clear();
				tablePoints->Add(gcnew ValuePoint(0,0,false));
				tablePoints->Add(gcnew ValuePoint(5,10,false));
				tablePoints->Add(gcnew ValuePoint(10,10,false));
				
				//---- set new parameter values and species initial values
				sut->SetParameterValues(variableParams);

				//---- run simulation with new values
				sut->RunSimulation();

				_y4_with_change=sut->ValuesFor("y4")->Values;
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result_without_changing_table_parameter()
        {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();

			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			for(int i=0; i<noOfOutputtimePoints; i++)
				BDDExtensions::ShouldBeEqualTo(_y4_without_change[i], _y4_without_change[0], relTol);
        }

		[TestAttribute]
        void should_produce_correct_result_with_changing_table_parameter()
        {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();

			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				double time = solverTimes[i];

				double expectedValue = _y4_with_change[0]+ (time <=5 ? 2*time : 10);
				BDDExtensions::ShouldBeEqualTo(_y4_with_change[i], expectedValue, relTol);
			}			
        }
    };



   
	public ref class when_running_testsystem_06_new_schema_setting_all_parameters_as_variable : public when_running_testsystem_06
    {

	protected:   
		 virtual void Because() override
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_NewSchema.xml"));
				
				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				
				//---- set all parameters as variable
				std::vector<SimModelNative::ParameterInfo> params;
				sim->FillParameterProperties(params);
				sim->SetVariableParameters(params);

				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
        void should_produce_correct_result()
        {
			TestResult();
        }
    };


	public ref class when_running_pksim_input abstract : public concern_for_simulation
	{
	protected:   

		String^ _inputFile;
		String^ _venPlsId;
		String^ _debugOutputFile;
		String^ _matlabCodeOutputFolder;
		bool   _writeDebugFile;
		bool   _writeMatlabCode;
		bool   _matlabCodeFullMode;

		when_running_pksim_input()
		{
			_writeDebugFile = false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
		}

		 virtual void Because() override
        {
        }

		 virtual void OptionalTasksBeforeFinalize()
		 {
		 }

		 SimModelNative::Variable * GetVenousBloodPlasma()
		 {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();

			SimModelNative::Variable * ven_pls = 
				sim->SpeciesList().GetObjectByEntityId(NETToCPPConversions::MarshalString(_venPlsId));
			if (ven_pls == NULL)
				ven_pls = sim->Observers().GetObjectByEntityId(NETToCPPConversions::MarshalString(_venPlsId));
			BDDExtensions::ShouldBeTrue(ven_pls!=NULL);

			return ven_pls;
		 }

		 bool VariableHasNegativeValues(System::String^ entityId)
		 {
			 SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			 SimModelNative::Variable * var = sim->SpeciesList().GetObjectByEntityId(NETToCPPConversions::MarshalString(entityId));

			 double * values = var->GetValues();
			 int valuesize = var->GetValuesSize();

			 double absTol = sut->GetNativeSimulation()->GetSolver().GetSolverProperties().GetAbsTol();

			 for (int i = 0; i < valuesize; i++)
			 {
				 if (values[i] < -absTol)
					 return true;
			 }

			 return false;
		 }

		void SimpleRunTestResult()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom(_inputFile));

				OptionalTasksBeforeFinalize();

				if (_writeMatlabCode)
				{
					//will finalize simulation
					SimModelNative::MatlabODEExporter odeExporter;
					odeExporter.WriteMatlabCode(sut->GetNativeSimulation(), 
						NETToCPPConversions::MarshalString(_matlabCodeOutputFolder), 
						_matlabCodeFullMode);
				}
				else
					sut->FinalizeSimulation();

				double absTol = sut->GetNativeSimulation()->GetSolver().GetSolverProperties().GetAbsTol();
				double relTol = sut->GetNativeSimulation()->GetSolver().GetSolverProperties().GetRelTol();

				sut->RunSimulation();

				if(!sut->ToleranceWasReduced)
				{
					BDDExtensions::ShouldBeEqualTo(sut->UsedAbsoluteTolerance, absTol);
					BDDExtensions::ShouldBeEqualTo(sut->UsedRelativeTolerance, relTol);
				}

				//assure output time schema is correct
				CheckSolverOutputTimes();

				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
	
				SimModelNative::Variable * ven_pls = GetVenousBloodPlasma();

				BDDExtensions::ShouldBeEqualTo(ven_pls->GetValuesSize(), noOfOutputtimePoints);
				double * venPlsValues = ven_pls->GetValues();

				if (_writeDebugFile)
				{
					std::ofstream outfile;
					outfile.open(NETToCPPConversions::MarshalString(_debugOutputFile).c_str());

					for (int i = 0; i < noOfOutputtimePoints; i++)
					{
						outfile<<XMLHelper::ToString(venPlsValues[i]) << std::endl;
					}
					outfile.close();
				}

			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
	};

	
	public ref class when_running_pksim_input_01 : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKSim_Input_01.xml";
			_venPlsId = "727457644";
			_debugOutputFile = "D:\\Out\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();
		}
	};




	
	public ref class when_running_pksim_input_MultiApp_2_dense : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKSim_Input_04_MultiApp.xml";
			_venPlsId = "25cee37d-434a-4dd0-a91a-96e0c8952339";
			_debugOutputFile = "C:\\VSS\\SimModel\\branches\\4.0\\Test\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }
		
		virtual void OptionalTasksBeforeFinalize() override
		{
			DisableBandLinearSolver();
		}

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();
			CheckBandLinearSolverDisabled();
        }

	};

	
	public ref class when_running_pksim_input_MultiApp_2_band : public when_running_pksim_input
	{
	protected:   
		virtual void OptionalTasksBeforeFinalize() override
		 {
			 EnableBandLinearSolver();
		 }

		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKSim_Input_04_MultiApp.xml";
			_venPlsId = "25cee37d-434a-4dd0-a91a-96e0c8952339";
			_debugOutputFile = "C:\\VSS\\SimModel\\branches\\4.0\\Test\\SimModelResult_VenPls_band.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();
			CheckBandLinearSolverEnabled();
        }

	};

	
	public ref class when_running_pkmodelcore_case_study_01 : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKModelCoreCaseStudy_01.xml";
			_venPlsId = "66d0f1d8-b644-4cd6-874c-a2a9b079d702";
			_debugOutputFile = "D:\\Out\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;

			SimpleRunTestResult();
        }

		 virtual void OptionalTasksBeforeFinalize() override
		 {
			 DisableBandLinearSolver();
		 }

    public:
        [TestAttribute]
        void mass_balance_at_t0_should_be_ok()
        {
			double amount = 0.0;

			for (int i=0; i<sut->GetNativeSimulation()->SpeciesList().size(); i++)
			{
				amount += sut->GetNativeSimulation()->SpeciesList()[i]->GetValues()[0];
			}

			BDDExtensions::ShouldBeEqualTo(amount, 26.5, 1e-10);
        }

		[TestAttribute]
        void mass_balance_at_tEnd_should_be_ok()
        {
			double amount = 0.0;

			int noOfTimePoints=sut->GetNativeSimulation()->GetNumberOfTimePoints();

			for (int i=0; i<sut->GetNativeSimulation()->SpeciesList().size(); i++)
			{
				SimModelNative::Species * species = sut->GetNativeSimulation()->SpeciesList()[i];
				
				int lastValueIdx;
				if (species->IsConstantDuringCalculation())
					lastValueIdx = 0;
				else
					lastValueIdx = noOfTimePoints-1;

				amount += species->GetValues()[lastValueIdx];
			}

			BDDExtensions::ShouldBeEqualTo(amount, 36.5, 1e-10);
        }

		[TestAttribute]
		void amount_observer_for_arterial_blood_plasma_A_should_return_correct_values()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();

			SimModelNative::Species  * Art_pls_A = sim->SpeciesList().GetObjectById(3);
			SimModelNative::Observer * Art_pls_A_Obs1 = sim->Observers().GetObjectById(7);

			DoubleQueue timeSchema = sut->GetNativeSimulation()->GetOutputSchema().AllTimePoints<float>();
			BDDExtensions::ShouldBeEqualTo<int>(timeSchema.size(), noOfOutputtimePoints);

			BDDExtensions::ShouldBeEqualTo(Art_pls_A->GetValuesSize(), noOfOutputtimePoints);
			BDDExtensions::ShouldBeEqualTo(Art_pls_A_Obs1->GetValuesSize(), noOfOutputtimePoints);

			double * variable_values = Art_pls_A->GetValues();
			double * observer_values = Art_pls_A_Obs1->GetValues();

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				double var_value = variable_values[i], obs_value = observer_values[i];
				BDDExtensions::ShouldBeEqualTo(obs_value, var_value/2.0, 1e-5);
			}
		}
	};

	
	public ref class when_running_pkmodelcore_case_study_01_with_scalefactor : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKModelCoreCaseStudy_01.xml";
			_venPlsId = "66d0f1d8-b644-4cd6-874c-a2a9b079d702";
			_debugOutputFile = "D:\\Out\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;

			SimpleRunTestResult();
        }

		virtual void OptionalTasksBeforeFinalize() override
		{
			SimModelNative::Species * y1;
			y1 = sut->GetNativeSimulation()->SpeciesList().GetObjectById(3);
			y1->SetODEScaleFactor(10.0);

			DisableBandLinearSolver(); //TODO delete as soon as band solver issue is solved
		}

    public:
        [TestAttribute]
        void mass_balance_at_t0_and_at_tEnd_should_be_ok()
        {
			int noOfTimePoints=sut->GetNativeSimulation()->GetNumberOfTimePoints();
			double amount_t0 = 0.0, amount_tEnd = 0.0;

			for (int i=0; i<sut->GetNativeSimulation()->SpeciesList().size(); i++)
			{
				SimModelNative::Species * species = sut->GetNativeSimulation()->SpeciesList()[i];
				amount_t0 += species->GetValues()[0];

				int lastValueIdx;
				if (species->IsConstantDuringCalculation())
					lastValueIdx = 0;
				else
					lastValueIdx = noOfTimePoints-1;

				amount_tEnd += species->GetValues()[lastValueIdx];
			}

			BDDExtensions::ShouldBeEqualTo(sut->GetNativeSimulation()->SolverWarnings().size(), 0);

			BDDExtensions::ShouldBeEqualTo(amount_t0, 26.5, 1e-10);
			BDDExtensions::ShouldBeEqualTo(amount_tEnd, 36.5, 1e-10);
        }

		[TestAttribute]
		void amount_observer_for_arterial_blood_plasma_A_should_return_correct_values()
		{
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			double * solverTimes = sim->GetTimeValues();

			SimModelNative::Species  * Art_pls_A = sim->SpeciesList().GetObjectById(3);
			SimModelNative::Observer * Art_pls_A_Obs1 = sim->Observers().GetObjectById(7);

			DoubleQueue timeSchema = sut->GetNativeSimulation()->GetOutputSchema().AllTimePoints<float>();
			BDDExtensions::ShouldBeEqualTo<int>(timeSchema.size(), noOfOutputtimePoints);

			BDDExtensions::ShouldBeEqualTo(Art_pls_A->GetValuesSize(), noOfOutputtimePoints);
			BDDExtensions::ShouldBeEqualTo(Art_pls_A_Obs1->GetValuesSize(), noOfOutputtimePoints);

			double * variable_values = Art_pls_A->GetValues();
			double * observer_values = Art_pls_A_Obs1->GetValues();

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				double var_value = variable_values[i], obs_value = observer_values[i];
				BDDExtensions::ShouldBeEqualTo(obs_value, var_value/2.0, 1e-5);
			}
		}
	};


	public ref class when_running_pkmodelcore_case_study_02 : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKModelCoreCaseStudy_02.xml";
			_venPlsId = "e6bc43ed-5acd-4457-bc22-560c8f44ab38";
			_debugOutputFile = "D:\\Out\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;

			SimpleRunTestResult();
        }

		virtual void OptionalTasksBeforeFinalize() override
		{
			DisableBandLinearSolver();
		}

    public:
        [TestAttribute]
        void mass_balance_at_t0_and_at_tEnd_should_be_ok()
        {
			int noOfTimePoints=sut->GetNativeSimulation()->GetNumberOfTimePoints();
			double amount_t0 = 0.0, amount_tEnd = 0.0;

			for (int i=0; i<sut->GetNativeSimulation()->SpeciesList().size(); i++)
			{
				SimModelNative::Species * species = sut->GetNativeSimulation()->SpeciesList()[i];
				amount_t0 += species->GetValues()[0];

				int lastValueIdx;
				if (species->IsConstantDuringCalculation())
					lastValueIdx = 0;
				else
					lastValueIdx = noOfTimePoints-1;

				amount_tEnd += species->GetValues()[lastValueIdx];
			}

			BDDExtensions::ShouldBeEqualTo(sut->GetNativeSimulation()->SolverWarnings().size(), 0);

			BDDExtensions::ShouldBeEqualTo(amount_t0, 26.5,1e-10);
			BDDExtensions::ShouldBeEqualTo(amount_tEnd, 36.5,1e-10);
        }

	};


	public ref class when_running_test4model_reduced02 : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "Test4Model_Reduced03.xml";
			_venPlsId = "0beab802-84f5-4bed-ae57-a4afeb96737c";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\ML_TestOutput\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\ML_TestOutput";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true; 
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();
		}


		[TestAttribute]
		void should_redim_variables_and_observers_according_to_is_persistant_flag()
        {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();

			//check that all persistable species have the full size and non-persistable size of 1
			SimModelNative::TObjectList<SimModelNative::Species> allSpecies = sim->SpeciesList();
			for (int speciesIdx = 0; speciesIdx<allSpecies.size(); speciesIdx++)
			{
				SimModelNative::Species * species = allSpecies[speciesIdx];
				BDDExtensions::ShouldBeEqualTo(species->GetValuesSize(), species->IsPersistable() ? noOfOutputtimePoints : 1);
			}

			//same check for observers
			SimModelNative::TObjectList<SimModelNative::Observer> allObservers = sim->Observers();
			for (int observerIdx = 0; observerIdx<allObservers.size(); observerIdx++)
			{
				SimModelNative::Observer * observer = allObservers[observerIdx];
				BDDExtensions::ShouldBeEqualTo(observer->GetValuesSize(), observer->IsPersistable() ? noOfOutputtimePoints : 1);
			}
        }

	};
    
	public ref class when_loading_from_file_new_schema : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_be_loaded_from_file()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("PKSim_Input_NewSchema_01.xml"));
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
    };

   
	public ref class when_running_below_abstol_test : public when_running_pksim_input
    {
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PKSim_Input_BelowAbsTol.xml";
			_venPlsId = "XRhXIrsshU2zWFavqRn7yQ";
			_debugOutputFile = SpecsHelper::TestFileFrom("SimModelResult_VenPls.txt");
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;

			SimpleRunTestResult();
        }

    public:
        
		[TestAttribute]
		void all_variable_values_below_abstol_should_be_zero()
        {
			SimModelNative::Simulation * sim = sut->GetNativeSimulation();
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
			
			double absTol = sim->GetSolver().GetSolverProperties().GetAbsTol();

			for(int i=0; i<sim->SpeciesList().size(); i++)
			{
				SimModelNative::Species * species = sim->SpeciesList()[i];

				double * values = species->GetValues();

				for (int i = 0; i < species->GetValuesSize(); i++)
				{
					if ((values[i] < 0.0) && (values[i] > -absTol))
						ExceptionHelper::ThrowExceptionFrom("Species "+species->GetFullName()+" has values in [-AbsTol..AbsTol]");
				}
			}
        }
    };

	
	public ref class when_running_AdultPopulation_sim : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "AdultPopulation.xml";
			_venPlsId = "o4iFZUU4dUSypxgqm3y7HQ";
			_debugOutputFile = "D:\\Out\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();
		}
	};

	
	public ref class when_running_oral_table_01 : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "OralTable01.xml";
			_venPlsId = "qpogYIvjOE-lRts7NtmaJw";
			_debugOutputFile = "D:\\Out\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "D:\\Out\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();
		}
	};
    
	public ref class when_loading_from_file_with_infinity_values : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_be_loaded_from_file()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("InfinityTest.xml"));
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
    };

	
	public ref class when_running_IV_EHC : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "Theophyline_2.xml";
			_venPlsId = "n0k31nUqU0CIQJSGWuBzUw";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void start_of_gallbladder_refilling_should_be_in_the_switch_startpoints_list()
        {
			SimpleRunTestResult();
			
			DoubleQueue switchTimePoints = SimModelNative::SwitchTask::SwitchTimePoints(sut->GetNativeSimulation()->Switches());
			double lastSwitchTimePoint = 0;

			while (!switchTimePoints.empty())
			{
				lastSwitchTimePoint = switchTimePoints.top();
				switchTimePoints.pop();
			}

			BDDExtensions::ShouldBeEqualTo(lastSwitchTimePoint,126, 1e-5);
		}
	};

	
	public ref class when_running_Growing_const_tables : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "GrowConst.xml";
			_venPlsId = "fnpcgAdXBES9jRiYGarWMQ";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run()
        {
			SimpleRunTestResult();			
		}
	};
	
	public ref class when_running_simulation_with_persistable_parameters : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "PersistableParams.xml";
			_venPlsId = "74fe8982-69cd-41be-9dc6-7fd8020f2ed4";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
        void should_perform_simulation_run_and_return_persistable_parameters_as_observers()
        {
			SimpleRunTestResult();

			SimModelNative::Simulation * sim = sut->GetNativeSimulation();

			BDDExtensions::ShouldBeEqualTo(sim->Observers().size(),2);

			SimModelNative::Observer * timeParamObserver = sim->Observers()[0];
			SimModelNative::Observer * inverseTimeParamObserver = sim->Observers()[1];

			BDDExtensions::ShouldBeTrue(timeParamObserver->GetEntityId()=="74fe8982-69cd-41be-9dc6-7fd8020f2ed4");
			int noOfOutputtimePoints = sim->GetNumberOfTimePoints();

			double * solverTimes = sim->GetTimeValues();
			double * obsValues = timeParamObserver->GetValues();
			double * inverseObsValues = inverseTimeParamObserver->GetValues();

			for(int i=0; i<noOfOutputtimePoints; i++)
			{
				BDDExtensions::ShouldBeEqualTo(obsValues[i], solverTimes[i], 1e-5);

				if (i==0)
					BDDExtensions::ShouldBeEqualTo(inverseObsValues[i], SimModelNative::MathHelper::GetInf());
				else
					BDDExtensions::ShouldBeEqualTo(inverseObsValues[i], 1.0/solverTimes[i], 1e-5);
			}

			IList<IValues^>^ values = sut->AllValues;

			BDDExtensions::ShouldBeFalse(values[0]->IsConstant);
			BDDExtensions::ShouldBeFalse(values[1]->IsConstant);
		}
	};

    
	public ref class when_loading_from_file_with_table_formulas : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_return_table_formula_infos()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("TableParametersViaDCI_Test01.xml"));

				SimModelNative::Simulation * sim = sut->GetNativeSimulation();

				std::vector <SimModelNative::ParameterInfo> paramInfos;
				sim->FillParameterProperties(paramInfos);

				int paramsWithTableFormulaCount = 0;
				for(size_t i=0; i<paramInfos.size(); i++)
				{
					if (paramInfos[i].IsTable())
						paramsWithTableFormulaCount++;
				}

				BDDExtensions::ShouldBeTrue(paramsWithTableFormulaCount>0);
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
    };
	
	public ref class when_loading_solver_error_testmodel : public when_running_pksim_input
	{
	protected:   
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "SolverError01.xml";
			_venPlsId = "d8040e14-fd37-4130-b897-548d73312911";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

    public:
        [TestAttribute]
		void should_load_and_finalize_simulation()
        {
			sut->LoadFromXMLFile(SpecsHelper::TestFileFrom(_inputFile));
			sut->FinalizeSimulation();

			//simulation run not performed because it takes a lot of time
			//after removing constant species ODE system still contains 1243 variables
//			SimpleRunTestResult();			
		}
	};
	
	public ref class when_changing_ehc_start_time : public when_running_pksim_input
	{
	protected:   
		SimModelNative::Parameter * startTime;
		 virtual void Because() override
        {
			when_running_pksim_input::Because();

			_inputFile = "POP_EHC_StartTime.xml";
			_venPlsId = "7260856f-e266-4db3-90b6-bda1ac1ba6d2";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile =false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
        }

		virtual void OptionalTasksBeforeFinalize() override
		{
			startTime = sut->GetNativeSimulation()->Parameters().GetObjectById(4127);

			//make sure we've got the right parameter
			BDDExtensions::ShouldBeTrue(startTime->GetFullName()=="POP|Events|EHC|EHC_1|Start time");

			startTime->SetIsFixed(false);
		}

		double getSumVenPls()
		{
			double value = 0.0;
			
			SimModelNative::Variable * ven_pls = GetVenousBloodPlasma();
			double * values = ven_pls->GetValues();

			for(int i=0;i<ven_pls->GetValuesSize();i++)
			{
				value += values[i];
			}

			return value;
		}

    public:
        [TestAttribute]
		void changing_ehc_start_time_should_alter_simulation_results()
        {
			SimpleRunTestResult();

			double value1 = getSumVenPls();
			
			startTime->SetInitialValue(1000000); //EHC beyond simulation time
			sut->RunSimulation();

			double value2 = getSumVenPls();

			//Difference should be > 5%
			double diff = fabs(value1-value2)/min(value1,value2);

			BDDExtensions::ShouldBeTrue(diff>0.05);
		}
	};
    
	public ref class when_loading_from_old_xml_file : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
		void xml_version_should_be_set_to_the_old_version()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("PersistableParams.xml"));

				double version = (double)sut->GetNativeSimulation()->GetXMLVersion();
				BDDExtensions::ShouldBeEqualTo(version, (double)SimModelNative::OLD_SIMMODEL_XML_VERSION);
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
    };

    
	public ref class when_loading_from_new_xml_file : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
		void xml_version_should_be_set_to_the_new_version()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("VersionTest.xml"));
				
				double version = (double)sut->GetNativeSimulation()->GetXMLVersion();
				
				BDDExtensions::ShouldNotBeEqualTo(version, (double)SimModelNative::OLD_SIMMODEL_XML_VERSION);
				BDDExtensions::ShouldBeEqualTo(version, 3.0);
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
    };

    
	public ref class when_changing_species_initial_values : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
		[Ignore("This test should be investigated and completed after the bugfix")]
        void should_update_species_initial_values_in_the_simulation_xml_node()
        {
			//initial system is:
			//
			//  y1(0) = 2;  y1' = 0
			//  y2(0) = y1; y2' = 0
			//
			try
			{
				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				sim->Options().SetKeepXMLNodeAsString(true);

				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("Test4SavingSpeciesInitialValues.xml"));

				SimModelNative::Species * y1, * y2;

				y1 = sim->SpeciesList().GetObjectByEntityId("y1");
				y2 = sim->SpeciesList().GetObjectByEntityId("y2");

				y1->SetIsFixed(false);
				y2->SetIsFixed(false);

				sut->FinalizeSimulation();

				sut->RunSimulation();

				double y1_startvalue = y1->GetValues()[0];
				double y2_startvalue = y2->GetValues()[0];

				//after running initial system, y1(0)=2, y2(0)=2
				BDDExtensions::ShouldBeEqualTo(y1_startvalue, 2.0);
				BDDExtensions::ShouldBeEqualTo(y2_startvalue, 2.0);

				//---- change species initial values and scaling factors
				y1->SetInitialValue(3.0);
				y2->SetInitialValue(4.0);

				y1->SetODEScaleFactor(5.0);
				y2->SetODEScaleFactor(6.0);

				//---- get xml string of the changed simulation
				std::string simXMLString = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + 
				                           sim->GetSimulationXMLString();

				//---- create new simulation and load it from  string
				sut=gcnew Simulation();
				sut->LoadFromXMLString(CPPToNETConversions::MarshalString(simXMLString));

				//rerun new simulation
				sut->FinalizeSimulation();
				sut->RunSimulation();

				sim = sut->GetNativeSimulation();

				y1 = sim->SpeciesList().GetObjectByEntityId("y1");
				y2 = sim->SpeciesList().GetObjectByEntityId("y2");

				y1_startvalue = y1->GetValues()[0];
				y2_startvalue = y2->GetValues()[0];

				//check new initial values
				BDDExtensions::ShouldBeEqualTo(y1_startvalue, 3.0);
				BDDExtensions::ShouldBeEqualTo(y2_startvalue, 4.0);

				//check new scale factor //TODO enable test as soon as saving scale factor bug is fixed
				//BDDExtensions::ShouldBeEqualTo(y1->GetODEScaleFactor(), 5.0);
				//BDDExtensions::ShouldBeEqualTo(y2->GetODEScaleFactor(), 6.0);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
    };


    
	public ref class when_exporting_bcm_platelet_to_matlab : public concern_for_simulation
    {
	protected:   
		 virtual void Because() override
        {
        }

    public:
        [TestAttribute]
        void should_export_to_matlab()
        {
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("BCM_Platelet6Lit.xml"));
				
				SimModelNative::MatlabODEExporter odeExporter;
				odeExporter.WriteMatlabCode(sut->GetNativeSimulation(), "C:\\Temp\\", true);
			
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
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
    };
	
	public ref class when_loading_simualtion_which_is_not_schema_conform : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_throw_a_BTS_exception()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("InvalidSimmodelXML.xml"));

				ExceptionHelper::ThrowExceptionFrom("Loading invalid file did not throw any exception");
			}
			catch (OSPSuite::Utility::Exceptions::OSPSuiteException^ )
			{
				//nothing to do, that's the expected behavior
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFrom("Loading invalid file did not throw a BTS exception");
			}
		}
	};

	
	public ref class when_running_simulation_with_if_formula_in_event_condition : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_perform_simulation_run()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("IfFormulaInSwitchCondition.xml"));
				sut->FinalizeSimulation();

				sut->RunSimulation();
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};

	
	public ref class when_running_simulation_returning_not_allowed_negative_values : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_perform_simulation_run_without_negative_values_check_and_throw_an_exception_with_negative_values_check()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("NegativeValuesTestSimple.xml"));
				
				sut->FinalizeSimulation();

				//first perform without negative values check
				sut->CheckForNegativeValues::set(false);
				try
				{
					sut->RunSimulation();
				}
				catch (...)
				{
					throw "Exception was thrown but negative values check was deactivated";
				}

				sut->CheckForNegativeValues::set(true);
				try
				{
					sut->RunSimulation();
				}
				catch (SimModelException^ ED)
				{
					//check the error message contains "negative"
					BDDExtensions::ShouldBeTrue(ED->Message->Contains("negative"));

					//expected behavior. Leave the test case
					return;
				}

				//failed (no exception with negative values check)
				throw "No exception was thrown with negative values check";
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};

	
	public ref class when_running_simulation_with_events_simultaneously_increasing_a_variable : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_perform_all_events()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("TestSimultanEvents.xml"));
				sut->FinalizeSimulation();

				sut->RunSimulation();

				//simulation has only 1 constant variable with start value 0, which is modified by 2 events:
				//    event #1: M=M+10
				//    event #2: M=M+20
				// both events fire at t=10; simulation output interval is [0..30]
				//
				//thus at the end it must be: Variable[0]=0 and Variable[lastIndex]=30

				SimModelNative::Simulation * sim = sut->GetNativeSimulation();

				SimModelNative::Variable * variable = sim->SpeciesList().GetObjectById(2);

				BDDExtensions::ShouldBeEqualTo(variable->GetValues()[0], 0.0, 1e-5);
				BDDExtensions::ShouldBeEqualTo(variable->GetValues()[variable->GetValuesSize() - 1], 30.0, 1e-5);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};


	
	public ref class when_solving_cvsRoberts_FSA_dns_with_sensitivity_Sensitivity_RHS_function_not_set : public concern_for_simulation
	{
	protected:
		array<double, 3>^ _expectedSensitivities;
		const unsigned int _numberOfTimesteps = 2;
		const unsigned int _numberOfUnknowns = 3;
		const unsigned int _numberOfSensitivityParameters = 3;

		virtual void Because() override
		{
			_expectedSensitivities = FillExpectedSensitivities();

			sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("cvsRoberts_FSA_dns.xml"));

			IList<IParameterProperties^>^ params = sut->ParameterProperties;
			IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

			for each (IParameterProperties^ param in params)
			{
				if ((param->EntityId == "P1") ||
					(param->EntityId == "P2") ||
					(param->EntityId == "P3"))
				{
					param->CalculateSensitivity = true;
					variableParams->Add(param);
				}
			}

			sut->VariableParameters = variableParams;
			sut->FinalizeSimulation();
			sut->RunSimulation();

		}

		//values produced via direct usage of CVODES
		array<double, 3>^ FillExpectedSensitivities()
		{

			array<double, 3>^ sens = gcnew array<double, 3>(_numberOfTimesteps, _numberOfUnknowns, _numberOfSensitivityParameters)
			{
				{ //time step #1
					{ -3.5611e-001, 9.4831e-008, -1.5733e-011},  // {dy1/dp1, dy1/dp2, dy1/dp3}
					{ 3.9023e-004, -2.1325e-010, -5.2897e-013 }, // {dy2/dp1, dy2/dp2, dy2/dp3}
					{ 3.5572e-001, -9.4618e-008, 1.6262e-011 }   // {dy3/dp1, dy3/dp2, dy3/dp3}
				},

				{ //time step #2
					{ -1.8761e+000, 2.9612e-006, -4.9330e-010 }, // {dy1/dp1, dy1/dp2, dy1/dp3}
					{ 1.7922e-004, -5.8308e-010, -2.7624e-013 }, // {dy2/dp1, dy2/dp2, dy2/dp3}
					{ 1.8760e+000, -2.9606e-006, 4.9357e-010 }   // {dy3/dp1, dy3/dp2, dy3/dp3}
				}
			};

			return sens;
		}

		void checkSensitivities(array<double> ^dy1_dp1, array<double> ^dy1_dp2, array<double> ^dy1_dp3, 
			                    array<double> ^dy2_dp1, array<double> ^dy2_dp2, array<double> ^dy2_dp3,
								array<double> ^dy3_dp1, array<double> ^dy3_dp2, array<double> ^dy3_dp3, 
								array<double> ^dObs1_dp1, array<double> ^dObs1_dp2, array<double> ^dObs1_dp3)
		{
			array<double, 3>^ sensitivities;
			sensitivities = gcnew array<double, 3>(_numberOfTimesteps, _numberOfUnknowns, _numberOfSensitivityParameters)
			{
				{ //time step #1
					{ dy1_dp1[1], dy1_dp2[1], dy1_dp3[1] }, // {dy1/dp1, dy1/dp2, dy1/dp3}
					{ dy2_dp1[1], dy2_dp2[1], dy2_dp3[1] }, // {dy2/dp1, dy2/dp2, dy2/dp3}
					{ dy3_dp1[1], dy3_dp2[1], dy3_dp3[1] }  // {dy3/dp1, dy3/dp2, dy3/dp3}
				},

				{ //time step #2
					{ dy1_dp1[2], dy1_dp2[2], dy1_dp3[2] }, // {dy1/dp1, dy1/dp2, dy1/dp3}
					{ dy2_dp1[2], dy2_dp2[2], dy2_dp3[2] }, // {dy2/dp1, dy2/dp2, dy2/dp3}
					{ dy3_dp1[2], dy3_dp2[2], dy3_dp3[2] }  // {dy3/dp1, dy3/dp2, dy3/dp3}
				}
			};

			//---- TODO ------------------------------------------------------
			//Test passes with relTol 1e-2 but fails already with relTol 1e-3
			//This should be investigated further! Deviation seems too high for me
			//
			//Test output with relTol=1e-3:
			//    Timestep: 1 Variable: 1 Parameter: 2 Expected sensitivity: 9,4831E-08 Returned sensitivity: 9,54238142897576E-08
			//    9,54238142897576E-08 and 9,4831E-08 are not equal within relative tolerance 0,001
			//----------------------------------------------------------------
			const double relTol = 1e-2; //max. allowed relative deviation 1%

			for (unsigned int i = 0; i < _numberOfTimesteps; i++)
			{
				for (unsigned int j = 0; j < _numberOfUnknowns; j++)
				{
					for (unsigned int k = 0; k < _numberOfSensitivityParameters; k++)
					{
						System::String^ msg = System::String::Format("Timestep: {0}\nVariable: {1}\nParameter: {2}\nExpected sensitivity: {3}\nReturned sensitivity: {4}\n", i + 1, j + 1, k + 1, _expectedSensitivities[i, j, k], sensitivities[i, j, k]);
						BDDExtensions::ShouldBeEqualTo(sensitivities[i, j, k], _expectedSensitivities[i, j, k], relTol, msg);
					}
				}
			}

			//check observer sensitivity values. 
			//Observer is defined as y1+2*y2+3*y3
			//the same must apply for all parameter sensitivities

			array<double, 2>^ observerSensitivities;

			observerSensitivities = gcnew array<double, 2>(_numberOfTimesteps, _numberOfSensitivityParameters)
			{
				{ //time step #1
					dObs1_dp1[1], dObs1_dp2[1], dObs1_dp3[1]
				},

				{ //time step #2
					dObs1_dp1[2], dObs1_dp2[2], dObs1_dp3[2]
				}
			};


			for (unsigned int i = 0; i < _numberOfTimesteps; i++)
			{
				for (unsigned int k = 0; k < _numberOfSensitivityParameters; k++)
				{
					double expectedSensitivity = sensitivities[i, 0, k] + 2 * sensitivities[i, 1, k] + 3 * sensitivities[i, 2, k];
					System::String^ msg = System::String::Format("Timestep: {0}\nParameter: {1}\nExpected sensitivity: {2}\nReturned sensitivity: {3}\n", i + 1, k + 1, expectedSensitivity, observerSensitivities[i, k]);
					BDDExtensions::ShouldBeEqualTo(observerSensitivities[i, k], expectedSensitivity, relTol, msg);
				}
			}

		}

	public:
		[TestAttribute]
		void should_solve_example_system_and_return_correct_sensitivity_values()
		{
			try
			{
				array<double> ^dy1_dp1, ^dy1_dp2, ^dy1_dp3, ^dy2_dp1, ^dy2_dp2, ^dy2_dp3, 
					          ^dy3_dp1, ^dy3_dp2, ^dy3_dp3, ^dObs1_dp1, ^dObs1_dp2, ^dObs1_dp3;

				dy1_dp1 = sut->SensitivityValuesFor("y1", "P1");
				dy1_dp2 = sut->SensitivityValuesFor("y1", "P2");
				dy1_dp3 = sut->SensitivityValuesFor("y1", "P3");

				dy2_dp1 = sut->SensitivityValuesFor("y2", "P1");
				dy2_dp2 = sut->SensitivityValuesFor("y2", "P2");
				dy2_dp3 = sut->SensitivityValuesFor("y2", "P3");

				dy3_dp1 = sut->SensitivityValuesFor("y3", "P1");
				dy3_dp2 = sut->SensitivityValuesFor("y3", "P2");
				dy3_dp3 = sut->SensitivityValuesFor("y3", "P3");

				dObs1_dp1 = sut->SensitivityValuesFor("Obs1", "P1");
				dObs1_dp2 = sut->SensitivityValuesFor("Obs1", "P2");
				dObs1_dp3 = sut->SensitivityValuesFor("Obs1", "P3");

				checkSensitivities(dy1_dp1, dy1_dp2, dy1_dp3,
					               dy2_dp1, dy2_dp2, dy2_dp3,
					               dy3_dp1, dy3_dp2, dy3_dp3,
					               dObs1_dp1, dObs1_dp2, dObs1_dp3);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}

		[TestAttribute]
		void should_solve_example_system_and_return_correct_sensitivity_values_by_entity_path()
		{
			try
			{
				array<double> ^dy1_dp1, ^dy1_dp2, ^dy1_dp3, ^dy2_dp1, ^dy2_dp2, ^dy2_dp3,
					^dy3_dp1, ^dy3_dp2, ^dy3_dp3, ^dObs1_dp1, ^dObs1_dp2, ^dObs1_dp3;

				dy1_dp1 = sut->SensitivityValuesByPathFor("SubContainer/y1", "SubContainer/P1");
				dy1_dp2 = sut->SensitivityValuesByPathFor("SubContainer/y1", "SubContainer/P2");
				dy1_dp3 = sut->SensitivityValuesByPathFor("SubContainer/y1", "SubContainer/P3");

				dy2_dp1 = sut->SensitivityValuesByPathFor("SubContainer/y2", "SubContainer/P1");
				dy2_dp2 = sut->SensitivityValuesByPathFor("SubContainer/y2", "SubContainer/P2");
				dy2_dp3 = sut->SensitivityValuesByPathFor("SubContainer/y2", "SubContainer/P3");

				dy3_dp1 = sut->SensitivityValuesByPathFor("SubContainer/y3", "SubContainer/P1");
				dy3_dp2 = sut->SensitivityValuesByPathFor("SubContainer/y3", "SubContainer/P2");
				dy3_dp3 = sut->SensitivityValuesByPathFor("SubContainer/y3", "SubContainer/P3");

				dObs1_dp1 = sut->SensitivityValuesByPathFor("SubContainer/Obs1", "SubContainer/P1");
				dObs1_dp2 = sut->SensitivityValuesByPathFor("SubContainer/Obs1", "SubContainer/P2");
				dObs1_dp3 = sut->SensitivityValuesByPathFor("SubContainer/Obs1", "SubContainer/P3");

				checkSensitivities(dy1_dp1, dy1_dp2, dy1_dp3,
					               dy2_dp1, dy2_dp2, dy2_dp3,
					               dy3_dp1, dy3_dp2, dy3_dp3,
					               dObs1_dp1, dObs1_dp2, dObs1_dp3);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};

	
	public ref class when_running_simulation_with_almost_equal_output_times : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_remove_duplicate_time_points_according_to_comparison()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("NonMonotoneBasegridTest.xml"));
				sut->FinalizeSimulation();

				//first, run simulation using (default) float comparison for user output points
				sut->RunSimulation();

				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				int noOfOutputtimePointsFloat = sim->GetNumberOfTimePoints();

				double * solverTimes = sim->GetTimeValues();

				for (int i = 0; i<noOfOutputtimePointsFloat-1; i++)
				{
					BDDExtensions::ShouldBeTrue(fabs(solverTimes[i] - solverTimes[i + 1]) >= 1e-8);
				}

				//now switch float comparison off (use double) and rerun simulation
				sim->Options().SetUseFloatComparisonInUserOutputTimePoints(false);
				sut->RunSimulation();

				int noOfOutputtimePointsDouble = sim->GetNumberOfTimePoints();
				BDDExtensions::ShouldBeTrue(noOfOutputtimePointsDouble > noOfOutputtimePointsFloat);

				//check that we have some points with deviation less than float precision
				bool found = false;
				solverTimes = sim->GetTimeValues();
				for (int i = 0; i<noOfOutputtimePointsDouble - 1; i++)
				{
					double timeDeviation = fabs(solverTimes[i] - solverTimes[i + 1]);

					if (timeDeviation < 1e-8)
					{
						BDDExtensions::ShouldBeTrue(timeDeviation>0.0); //still must be unique

						found = true; break;
					}
				}

				BDDExtensions::ShouldBeTrue(found);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};

	
	public ref class when_getting_output_schema : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_retrieve_correct_output_schema()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_Modified.xml"));
				sut->FinalizeSimulation();

				IOutputSchema^ outputSchema = sut->OutputSchema;
				SimModelNative::OutputSchema & nativeOutputSchema = sut->GetNativeSimulation()->GetOutputSchema();

				BDDExtensions::ShouldBeEqualTo(outputSchema->OutputIntervals->Count, nativeOutputSchema.OutputIntervals().size());

				for (int i = 0; i < outputSchema->OutputIntervals->Count; i++)
				{
					IOutputInterval^ interval1 = outputSchema->OutputIntervals[i];
					SimModelNative::OutputInterval * interval2 = nativeOutputSchema.OutputIntervals()[i];

					BDDExtensions::ShouldBeEqualTo(interval1->StartTime, interval2->StartTime());
					BDDExtensions::ShouldBeEqualTo(interval1->EndTime, interval2->EndTime());
					BDDExtensions::ShouldBeEqualTo(interval1->NumberOfTimePoints, interval2->NumberOfTimePoints());

				}
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};

	
	public ref class when_setting_output_schema : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_retrieve_output_time_vector_matching_the_new_schema()
		{
			try
			{
				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SimModel4_ExampleInput06_Modified.xml"));
				sut->FinalizeSimulation();

				IOutputSchema^ outputSchema = gcnew OutputSchema();
				IOutputInterval^ interval1 = gcnew OutputInterval(0, 0, 1); //{0}
				IOutputInterval^ interval2 = gcnew OutputInterval(5, 7, 3); //{5,6,7}
				IOutputInterval^ interval3 = gcnew OutputInterval(7, 9, 3); //{7,8,9}

				outputSchema->AddInterval(interval1);
				outputSchema->AddInterval(interval2);
				outputSchema->AddInterval(interval3);

				sut->OutputSchema = outputSchema;

				sut->RunSimulation();
				double * times = sut->GetNativeSimulation()->GetTimeValues();
				int numberOfTimeValues = sut->GetNativeSimulation()->GetNumberOfTimePoints();

				//expected array: {0, 5, 6, 7, 8, 9}
				double expectedTimeValues[6] = { 0, 5, 6, 7, 8, 9 };
				BDDExtensions::ShouldBeEqualTo(numberOfTimeValues, 6);

				for (int i = 0; i < 6; i++)
					BDDExtensions::ShouldBeEqualTo(times[i], expectedTimeValues[i]);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};

	
	public ref class when_solving_A_exp_minus_kT_with_sensitivity : public concern_for_simulation
	{
	protected:
		array<double, 3>^ _expectedSensitivities;
		const unsigned int _numberOfTimesteps = 21;
		const unsigned int _numberOfUnknowns = 1;
		const unsigned int _numberOfSensitivityParameters = 2;

		virtual void Because() override
		{
			sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("S3_reduced.xml"));

			IList<IParameterProperties^>^ params = sut->ParameterProperties;
			IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

			for each (IParameterProperties^ param in params)
			{
				if ((param->EntityId == "A0") ||
					(param->EntityId == "k"))
				{
					param->CalculateSensitivity = true;
					variableParams->Add(param);
				}
			}

			sut->VariableParameters = variableParams;
			sut->FinalizeSimulation();
			sut->RunSimulation();

		}

	//The (only) variable of the system d(C1)/dt=-k*C1; C1(0)=A0
	//Solution: C1(t)=A0*exp(-kt)
	//Sensitivities: d(C1)/d(A0) = exp(-kt)
	//               d(C1)/d(k)  = -A0*t*exp(-kt)
	public:
		[TestAttribute]
		void should_solve_example_system_and_return_correct_sensitivity_values()
		{
			try
			{
				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				
				int noOfOutputtimePoints = sim->GetNumberOfTimePoints();
				double * solverTimes = sim->GetTimeValues();
				double A0 = sim->Parameters().GetObjectByEntityId("A0")->GetValue(NULL, 0.0, SimModelNative::ScaleFactorUsageMode::IGNORE_SCALEFACTOR);
				double k = sim->Parameters().GetObjectByEntityId("k")->GetValue(NULL, 0.0, SimModelNative::ScaleFactorUsageMode::IGNORE_SCALEFACTOR);
				double *C1 = sim->SpeciesList().GetObjectByEntityId("C1")->GetValues();

				array<double> ^dC1_dA0 = sut->SensitivityValuesFor("C1", "A0");
				array<double> ^dC1_dk = sut->SensitivityValuesFor("C1", "k");

				const double relTol = 1e-3;
				for (int i = 1; i < noOfOutputtimePoints; i++)
				{
					double t = solverTimes[i];
					BDDExtensions::ShouldBeEqualTo(C1[i], A0*exp(-k*t), relTol);

					//TODO remove comment as soon as 47-8264 is fixed
//					BDDExtensions::ShouldBeEqualTo(dC1_dA0[i], exp(-k*t), relTol);
					
					BDDExtensions::ShouldBeEqualTo(dC1_dk[i], -A0*t*exp(-k*t), relTol);
				}
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}

	};

	
	public ref class when_calculating_sensitivity_of_persistable_parameter : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
			sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("SensitivityOfPersistableParameter.xml"));

			IList<IParameterProperties^>^ params = sut->ParameterProperties;
			IList<IParameterProperties^>^ variableParams = gcnew System::Collections::Generic::List<IParameterProperties^>();

			for each (IParameterProperties^ param in params)
			{
				if (param->EntityId == "Volume")
				{
					param->CalculateSensitivity = true;
					variableParams->Add(param);
				}
			}

			sut->VariableParameters = variableParams;
			sut->FinalizeSimulation();
			sut->RunSimulation();
		}

	public:
		[TestAttribute]
		void should_calculate_sensitivity_values_of_persistable_parameter()
		{
			try
			{
				SimModelNative::Simulation * sim = sut->GetNativeSimulation();

				array<double> ^dP1_dV = sut->SensitivityValuesByPathFor("Organism|P1", "Organism|Volume");
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (const char * message)
			{
				ExceptionHelper::ThrowExceptionFrom(message);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}

	};

	public ref class when_running_GIM_Table_Infusion_starting_at_1000_dense : public when_running_pksim_input
	{
	protected:
		virtual void Because() override
		{
			when_running_pksim_input::Because();

			_inputFile = "GIM_Table_01.xml";
			_venPlsId = "84457059-8660-4b5e-8810-9c6fbefcdd2d";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\GIM_Table_01_VenPlsInsulin.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile = false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
		}

		virtual void OptionalTasksBeforeFinalize() override
		{
			DisableBandLinearSolver();
		}

	public:
		[TestAttribute]
		void should_perform_simulation_run()
		{
			SimpleRunTestResult();
			CheckBandLinearSolverDisabled();
		}
	};

	public ref class when_running_GIM_Table_Infusion_starting_at_1000_band : public when_running_pksim_input
	{
	protected:

		virtual void OptionalTasksBeforeFinalize() override
		{
			EnableBandLinearSolver();
		}

		virtual void Because() override
		{
			when_running_pksim_input::Because();

			_inputFile = "GIM_Table_01.xml";
			_venPlsId = "84457059-8660-4b5e-8810-9c6fbefcdd2d";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\GIM_Table_01_VenPlsInsulin.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile = false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
		}

	public:
		[TestAttribute]
		void should_perform_simulation_run()
		{
			SimpleRunTestResult();
			CheckBandLinearSolverEnabled();
		}
	};

	public ref class when_running_GIM_model_with_negative_Mealtime_offset : public when_running_pksim_input
	{
	protected:
		virtual void Because() override
		{
			when_running_pksim_input::Because();

			_inputFile = "Neg_t_TimeSinceMeal.xml";
			_venPlsId = "815e3581-54b3-4c1c-a6b6-72a4e4709510";
			_debugOutputFile = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4\\SimModelResult_VenPls.txt";
			_matlabCodeOutputFolder = "C:\\VSS\\SimModel\\trunk\\Test\\TestData\\SM4";
			_writeDebugFile = false;
			_writeMatlabCode = false;
			_matlabCodeFullMode = true;
		}

	public:
		[TestAttribute]
		[Ignore("Enable test in case negative meal offset will be allowed")]
		void should_perform_simulation_run()
		{
			sut->LoadFromXMLFile(SpecsHelper::TestFileFrom(_inputFile));
			sut->FinalizeSimulation();
			SimpleRunTestResult();
		}
	};

	public ref class when_changing_output_schema : public concern_for_simulation
	{
	protected:
		virtual void Because() override
		{
		}

	public:
		[TestAttribute]
		void should_update_output_schema_in_the_simulation_xml_string()
		{
			try
			{
				SimModelNative::Simulation * sim = sut->GetNativeSimulation();
				sim->Options().SetKeepXMLNodeAsString(true);

				sut->LoadFromXMLFile(SpecsHelper::TestFileFrom("OutputSchemaSaveToXml.xml"));

				SimModelNative::OutputSchema & timeSchema = sim->GetOutputSchema();

				//---- set new output time schema
				timeSchema.Clear();
				timeSchema.OutputIntervals().push_back(new SimModelNative::OutputInterval(0, 10, 3, SimModelNative::OutputIntervalDistribution::Equidistant));
				timeSchema.OutputIntervals().push_back(new SimModelNative::OutputInterval(10, 100, 3, SimModelNative::OutputIntervalDistribution::Equidistant));
				timeSchema.OutputIntervals().push_back(new SimModelNative::OutputInterval(100, 1000, 3, SimModelNative::OutputIntervalDistribution::Equidistant));

				std::string simXMLString = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + sim->GetSimulationXMLString();

				//---- load new simulation from the xml string of the old one
				SimModelNative::Simulation * newSim = new SimModelNative::Simulation();
				newSim->LoadFromXMLString(simXMLString);

				timeSchema = newSim->GetOutputSchema();

				//---- check that output time schema in the new simulation is the same as in the old one
				BDDExtensions::ShouldBeEqualTo(timeSchema.OutputIntervals().size(), 3);
				SimModelNative::OutputInterval * interval;

				interval = timeSchema.OutputIntervals()[0];
				BDDExtensions::ShouldBeEqualTo(interval->StartTime(), 0.0);
				BDDExtensions::ShouldBeEqualTo(interval->EndTime(), 10.0);
				BDDExtensions::ShouldBeEqualTo(interval->NumberOfTimePoints(), 3);

				interval = timeSchema.OutputIntervals()[1];
				BDDExtensions::ShouldBeEqualTo(interval->StartTime(), 10.0);
				BDDExtensions::ShouldBeEqualTo(interval->EndTime(), 100.0);
				BDDExtensions::ShouldBeEqualTo(interval->NumberOfTimePoints(), 3);

				interval = timeSchema.OutputIntervals()[2];
				BDDExtensions::ShouldBeEqualTo(interval->StartTime(), 100.0);
				BDDExtensions::ShouldBeEqualTo(interval->EndTime(), 1000.0);
				BDDExtensions::ShouldBeEqualTo(interval->NumberOfTimePoints(), 3);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch (System::Exception^)
			{
				throw;
			}
			catch (...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}
	};
}