#ifdef WIN32
#pragma warning( disable : 4691)
#endif


#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelSpecs/SpecsHelper.h"
#include "SimModelManaged/Conversions.h"

#include "SimModelSpecs/SimModelCompSpecsHelper.h"
#include "XMLWrapper/XMLHelper.h"
#include <string>
#include "SimModelComp/SimModelComp.h"

namespace SimModelCompTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace SimModelNET;
	using namespace UnitTests;

	ref class SimModelCompWrapper
	{
	public:
		SimModelCompSpecsHelper * SMCSpecsHelper;
		SimModelCompWrapper(String^ simModelCompConfigPath)
		{
			SMCSpecsHelper=new SimModelCompSpecsHelper(NETToCPPConversions::MarshalString(simModelCompConfigPath).c_str());
		}
		~SimModelCompWrapper(){delete SMCSpecsHelper;}
		String^ DCILastError() {return CPPToNETConversions::MarshalString(SMCSpecsHelper->DCILastError());}
		void ConfigureFrom(String^ simulationFilePath)
		{
			SMCSpecsHelper->ConfigureFrom(
					NETToCPPConversions::MarshalString(SpecsHelper::SchemaPath()).c_str(),
					NETToCPPConversions::MarshalString(simulationFilePath).c_str());
		}
	};

	public ref class concern_for_simmodelcomp abstract : ContextSpecification<SimModelCompWrapper^>
    {
	protected:
		String^ _simulationFilePath;
		::SimModelComp * _simModelComp;

    public:
		virtual void GlobalContext() override
		{
			try
			{
				CoInitialize(NULL);
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
			try
			{
				Environment::CurrentDirectory = SpecsHelper::BaseDirectory();
				sut=gcnew SimModelCompWrapper(SpecsHelper::SimModelCompConfigFilePath());
				
				_simModelComp = sut->SMCSpecsHelper->GetInstance();
				sut->ConfigureFrom(_simulationFilePath);
			}
			catch(const std::string & errMsg)
			{
				ExceptionHelper::ThrowExceptionFrom(errMsg);
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

	public ref class when_getting_output_variables_and_observers : public concern_for_simmodelcomp
	{
	protected:
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath = SpecsHelper::TestFileFrom("PersistableOutputsTest.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
		{
			try
			{
				dciRetVal = _simModelComp->ProcessMetaData() && _simModelComp->ProcessData();
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

	public:
		[TestAttribute]
		void should_save_only_persistable_variables_and_observers_into_output_table()
		{
			try
			{
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());

				//get values Table
				DCI::ITableHandle hTab = _simModelComp->GetOutputPorts()->Item(2)->GetTable();

				//test model contains 1 pers. and 1. non-pers. observer and 1 pers. and 1. non-pers. variable
				BDDExtensions::ShouldBeEqualTo((int)hTab->GetColumns()->GetCount(), 2);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
		}
	};

	public ref class when_setting_all_output_variables_and_observers_as_persistable : public concern_for_simmodelcomp
	{
	protected:
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath = SpecsHelper::TestFileFrom("PersistableOutputsTest.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
		{
			try
			{
				dciRetVal = _simModelComp->Invoke("SetAllOutputsPersistable","") && 
					        _simModelComp->ProcessMetaData() && 
					        _simModelComp->ProcessData();
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

	public:
		[TestAttribute]
		void should_save_all_variables_and_observers_into_output_table()
		{
			try
			{
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());

				//get values Table
				DCI::ITableHandle hTab = _simModelComp->GetOutputPorts()->Item(2)->GetTable();

				//test model contains 1 pers. and 1. non-pers. observer and 1 pers. and 1. non-pers. variable
				//setting them all to persistable should retrieve 4 variables
				BDDExtensions::ShouldBeEqualTo((int)hTab->GetColumns()->GetCount(), 4);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
		}
	};

	public ref class when_performing_process_simulation_twice : public concern_for_simmodelcomp
    {
	protected:   
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("PKSim_Input_04_MultiApp.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
				dciRetVal = helper->ProcessMetaData() && 
							helper->ProcessData()     &&
							helper->ProcessData();
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
        void should_perform_two_simulation_runs()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };

	public ref class when_saving_simulation_to_file : public concern_for_simmodelcomp
	{
	protected:
		bool dciRetVal;
		array<double> ^ timeOrig, ^ timeReloaded;
		array<double> ^ y1Orig, ^ y1Reloaded, ^y2Orig, ^y2Reloaded, ^y3Orig, ^y3Reloaded;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("SimModel4_ExampleInput06.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
				
				//mark all constant parameters as variable
				helper->SetAllConstantParametersAsVariable();
				dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
				if(!dciRetVal)
					ExceptionHelper::ThrowExceptionFrom(helper->DCILastError());
				
				//get time and output variable values
				timeOrig = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputTime());
				y1Orig   = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputValues("TopContainer/y1"));
				y2Orig   = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputValues("TopContainer/y2"));
				y3Orig   = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputValues("TopContainer/y3"));

				String ^ savedFile = SpecsHelper::TestFileFrom("saved_ExampleInput06.xml");
				helper->SaveSimulationToXML(NETToCPPConversions::MarshalString(savedFile));

				SimModelCompWrapper^ reloadedSim=gcnew SimModelCompWrapper(SpecsHelper::SimModelCompConfigFilePath());
				
				reloadedSim->ConfigureFrom(savedFile);

				helper = reloadedSim->SMCSpecsHelper;
				dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
				if(!dciRetVal)
					ExceptionHelper::ThrowExceptionFrom(helper->DCILastError());

				timeReloaded = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputTime());
				y1Reloaded   = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputValues("TopContainer/y1"));
				y2Reloaded   = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputValues("TopContainer/y2"));
				y3Reloaded   = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputValues("TopContainer/y3"));
			}
			catch(std::string & str)
			{
				ExceptionHelper::ThrowExceptionFrom(str);
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
        void should_produce_same_results_when_reloaded_from_saved_file()
		{
			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			SpecsHelper::ArraysShouldBeEqual(timeOrig, timeReloaded, relTol);
			SpecsHelper::ArraysShouldBeEqual(y1Orig, y1Reloaded, relTol);
			SpecsHelper::ArraysShouldBeEqual(y2Orig, y2Reloaded, relTol);
			SpecsHelper::ArraysShouldBeEqual(y3Orig, y3Reloaded, relTol);
		}

	};

	public ref class when_performing_process_simulation_with_adultpopulation_sim : public concern_for_simmodelcomp
    {
	protected:   
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("AdultPopulation.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
				dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
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
        void should_perform_simulation_run()
        {
			try
			{
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };

	public ref class when_saving_simulation_Popsim_to_file : public concern_for_simmodelcomp
	{
	protected:
		bool dciRetVal;
		array<double> ^ timeOrig, ^ timeReloaded;
		array<double> ^ y1Orig, ^ y1Reloaded, ^y2Orig, ^y2Reloaded, ^y3Orig, ^y3Reloaded;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("POPsim.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
				
				//mark all constant parameters as variable
				helper->SetAllConstantParametersAsVariable();
				dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
				if(!dciRetVal)
					ExceptionHelper::ThrowExceptionFrom(helper->DCILastError());
				
				//get time and output variable values
				timeOrig = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputTime());

				String ^ savedFile = SpecsHelper::TestFileFrom("saved_POPsim.xml");
				helper->SaveSimulationToXML(NETToCPPConversions::MarshalString(savedFile));

				SimModelCompWrapper^ reloadedSim=gcnew SimModelCompWrapper(SpecsHelper::SimModelCompConfigFilePath());
				
				reloadedSim->ConfigureFrom(savedFile);

				helper = reloadedSim->SMCSpecsHelper;
				dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
				if(!dciRetVal)
					ExceptionHelper::ThrowExceptionFrom(helper->DCILastError());

				timeReloaded = CPPToNETConversions::DoubleVectorToArray(helper->GetOutputTime());
			}
			catch(std::string & str)
			{
				ExceptionHelper::ThrowExceptionFrom(str);
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
        void should_produce_same_results_when_reloaded_from_saved_file()
		{
			const double relTol = 1e-5; //max. allowed relative deviation 0.001%

			SpecsHelper::ArraysShouldBeEqual(timeOrig, timeReloaded, relTol);
		}

	};

	public ref class when_simulation_contains_table_parameters : public concern_for_simmodelcomp
    {
	protected:   
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("TableParametersViaDCI_Test01.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
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
        void should_perform_simulation_run()
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;

				std::vector<int> allTableParameterIds = helper->GetIds("AllTableParameters");

				BDDExtensions::ShouldBeTrue(allTableParameterIds.size()>0);

				dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };

	public ref class when_simulation_contains_persistable_parameters : public concern_for_simmodelcomp
    {
	protected:   
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("PersistableParams.xml");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
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
        void should_add_persistable_parameters_as_observers_into_observers_table()
        {
			try
			{
				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;
 
				dciRetVal = helper->ProcessMetaData();
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());

				BDDExtensions::ShouldBeTrue(
					helper->ContainsStringValue("AllObservers", "Path", "da|Organism|InverseTime"));

			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };

	public ref class when_asking_for_version : public concern_for_simmodelcomp
	{
	protected:
		bool dciRetVal;

		virtual void Context() override
		{
			Environment::CurrentDirectory = SpecsHelper::BaseDirectory();
			sut = gcnew SimModelCompWrapper(SpecsHelper::SimModelCompConfigFilePath());
		}

	public:
		[TestAttribute]
		void should_retrieve_version_of_simmodel_and_simmodelcomp()
		{
			try
			{
				SimModelComp* comp = sut->SMCSpecsHelper->GetInstance();
				std::string simModelVersion = comp->Invoke("GetSimModelVersion", "");
				std::string simModelCompVersion = comp->Invoke("GetSimModelCompVersion", "");

				BDDExtensions::ShouldBeTrue(simModelVersion.find_first_of(".") >=0);
				BDDExtensions::ShouldBeTrue(simModelCompVersion.find_first_of(".") >= 0);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
		}
	};
}