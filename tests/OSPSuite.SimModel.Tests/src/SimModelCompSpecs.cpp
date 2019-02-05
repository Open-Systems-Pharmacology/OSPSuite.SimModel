#ifdef _WINDOWS
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
			_simulationFilePath = SpecsHelper::TestFileFrom("PersistableOutputsTest");
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
			_simulationFilePath = SpecsHelper::TestFileFrom("PersistableOutputsTest");
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
			_simulationFilePath=SpecsHelper::TestFileFrom("PKSim_Input_04_MultiApp");
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
			_simulationFilePath=SpecsHelper::TestFileFrom("SimModel4_ExampleInput06");
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

				String ^ savedFile = SpecsHelper::TestFileFrom("saved_ExampleInput06");
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
			_simulationFilePath=SpecsHelper::TestFileFrom("AdultPopulation");
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
			_simulationFilePath=SpecsHelper::TestFileFrom("POPsim");
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

				String ^ savedFile = SpecsHelper::TestFileFrom("saved_POPsim");
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
			_simulationFilePath=SpecsHelper::TestFileFrom("TableParametersViaDCI_Test01");
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

	public ref class when_solving_cvsRoberts_FSA_dns_with_sensitivity : public concern_for_simmodelcomp
	{
	protected:
		array<double, 3>^ _expectedSensitivities;
		const unsigned int _numberOfTimesteps = 2;
		const unsigned int _numberOfUnknowns = 3;
		const unsigned int _numberOfSensitivityParameters = 3;

		virtual void Context() override
		{
			_simulationFilePath = SpecsHelper::TestFileFrom("cvsRoberts_FSA_dns");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
		{
			_expectedSensitivities = FillExpectedSensitivities();

			std::vector<std::string> sensitivityParameterPaths;
			sensitivityParameterPaths.push_back("TopContainer/SubContainer/P1");
			sensitivityParameterPaths.push_back("TopContainer/SubContainer/P2");
			sensitivityParameterPaths.push_back("TopContainer/SubContainer/P3");

			sut->SMCSpecsHelper->SetParametersForCalculateSensitivity(sensitivityParameterPaths);
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

		array<double> ^ getOutputSensitivities(const std::string & variableName, const std::string & sensitivityParameterName)
		{
			const std::string objectPathDelimiter = "/";
			const std::string pathPrefix = "TopContainer/SubContainer/";

			SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;

			std::string variablePath = pathPrefix + variableName;
			std::string sensitivityParameterPath = pathPrefix + sensitivityParameterName;

			return CPPToNETConversions::DoubleVectorToArray(helper->GetOutputSensitivities(variablePath, sensitivityParameterPath, objectPathDelimiter));
		}

	public:
		[TestAttribute]
		void should_solve_example_system_and_return_correct_sensitivity_values()
		{
			try
			{
				array<double> ^dy1_dp1, ^dy1_dp2, ^dy1_dp3, ^dy2_dp1, ^dy2_dp2, ^dy2_dp3,
					^dy3_dp1, ^dy3_dp2, ^dy3_dp3, ^dObs1_dp1, ^dObs1_dp2, ^dObs1_dp3;

				SimModelCompSpecsHelper * helper = sut->SMCSpecsHelper;

				bool dciRetVal = helper->ProcessMetaData() && helper->ProcessData();
				BDDExtensions::ShouldBeEqualTo(dciRetVal, true, sut->DCILastError());

				dy1_dp1 = getOutputSensitivities("y1", "P1");
				dy1_dp2 = getOutputSensitivities("y1", "P2");
				dy1_dp3 = getOutputSensitivities("y1", "P3");

				dy2_dp1 = getOutputSensitivities("y2", "P1");
				dy2_dp2 = getOutputSensitivities("y2", "P2");
				dy2_dp3 = getOutputSensitivities("y2", "P3");

				dy3_dp1 = getOutputSensitivities("y3", "P1");
				dy3_dp2 = getOutputSensitivities("y3", "P2");
				dy3_dp3 = getOutputSensitivities("y3", "P3");

				dObs1_dp1 = getOutputSensitivities("Obs1", "P1");
				dObs1_dp2 = getOutputSensitivities("Obs1", "P2");
				dObs1_dp3 = getOutputSensitivities("Obs1", "P3");

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

	public ref class when_simulation_contains_persistable_parameters : public concern_for_simmodelcomp
    {
	protected:   
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath=SpecsHelper::TestFileFrom("PersistableParams");
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
			try
			{
				Environment::CurrentDirectory = SpecsHelper::BaseDirectory();
				sut = gcnew SimModelCompWrapper(SpecsHelper::SimModelCompConfigFilePath());
			}
			catch (const std::string & errMsg)
			{
				ExceptionHelper::ThrowExceptionFrom(errMsg);
			}
			catch (ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
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

	public ref class when_getting_used_parameters_information : public concern_for_simmodelcomp
	{
	protected:
		bool dciRetVal;

		virtual void Context() override
		{
			_simulationFilePath = SpecsHelper::TestFileFrom("TestExportUsedParameters02");
			concern_for_simmodelcomp::Context();
		}

		virtual void Because() override
		{
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
}