#include "SimModelManaged/ManagedOutputSchema.h"
#include "SimModelManaged/ManagedSimulation.h"
#include "SimModelManaged/Conversions.h"
#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/XMLSchemaCache.h"
#include "SimModel/MatlabODEExporter.h"
#include "SimModel/MathHelper.h"
#include "XMLWrapper/XMLDocument.h"
#include "SimModel/ParameterSensitivity.h"
#include "SimModel/SimulationOptions.h"

namespace SimModelNET
{
	using namespace OSPSuite::Utility::Xml;

	SimModelNative::Simulation * Simulation::GetNativeSimulation()
	{
		return _simulation;
	}

	//constructor
	Simulation::Simulation()
	{
		_simulation=new SimModelNative::Simulation();
		_toleranceWasReduced = false;
		_newAbsTol = SimModelNative::MathHelper::GetNaN();
		_newRelTol = SimModelNative::MathHelper::GetNaN();

		_vecAllParameters = new std::vector<SimModelNative::ParameterInfo>();
		_vecVariableParameters = new std::vector<SimModelNative::ParameterInfo>();
		_vecAllSpecies = new std::vector<SimModelNative::SpeciesInfo>();
		_vecVariableSpecies = new std::vector<SimModelNative::SpeciesInfo>();
	}

	void Simulation::CleanUp()
	{
		if(_simulation)
			delete _simulation;
		_simulation = NULL;

		if(_vecAllParameters)
			delete _vecAllParameters;
		_vecAllParameters=NULL;

		if(_vecVariableParameters)
			delete _vecVariableParameters;
		_vecVariableParameters=NULL;

		if(_vecAllSpecies)
			delete _vecAllSpecies;
		_vecAllSpecies=NULL;

		if(_vecVariableSpecies)
			delete _vecVariableSpecies;
		_vecVariableSpecies=NULL;
	}

	// destructor
	Simulation::~Simulation()
	{
		CleanUp();
	}

	// finalizer
	Simulation::!Simulation()
	{
		CleanUp();
	} 

	///(internal) version
	System::String^ Simulation::Version::get()
	{
		System::String^ version;

		try
		{
			version = gcnew System::String("4.0");
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

		return version;
	}

	void Simulation::ValidateXMLStringWithSchema(System::String^ xmlString)
	{
		_simulation->Options().ValidateWithXMLSchema(false);

		IXmlValidator^ xmlValidator = gcnew XmlValidator(
			XMLSchemaCache::SchemaFile(), 
			CPPToNETConversions::MarshalString(SimModelNative::XMLConstants::SimModelSchema));

		SchemaValidationResult^ result = xmlValidator->Validate(xmlString);
		if (!result->Success::get())
			throw gcnew OSPSuite::Utility::Exceptions::OSPSuiteException(result->FullMessageLog::get());
	}

	///load simulation from xml file
    void Simulation::LoadFromXMLFile(System::String^ file)
	{
		try
		{
			XMLDocument XMLDoc = XMLDocument::FromFile(NETToCPPConversions::MarshalString(file));
			std::string xmlString = XMLDoc.ToString();
			XMLDoc.Release();

			LoadFromXMLString(CPPToNETConversions::MarshalString(xmlString));
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

	///load simulation from xml string
    void Simulation::LoadFromXMLString(System::String^ xmlString)
	{
		try
		{
			SimModelNative::SimulationOptions options;

			//---- clear previous stuff
			if (_simulation != NULL)
			{
				//keep options of the previous simulation and copy them into the new one
				options.CopyFrom(_simulation->Options());
				delete _simulation;
			}
			
			_simulation=new SimModelNative::Simulation();
			_simulation->Options().CopyFrom(options);

			_vecAllParameters->clear();
			_vecAllSpecies->clear();
			_vecVariableParameters->clear();
			_vecVariableSpecies->clear();

			//---- validate xml with schema and load simulation from xml string
			ValidateXMLStringWithSchema(xmlString);
			_simulation->LoadFromXMLString(NETToCPPConversions::MarshalString(xmlString));

			//---- fill parameters and species infos
			_simulation->FillParameterProperties(*_vecAllParameters);
			_simulation->FillDEVariableProperties(*_vecAllSpecies);
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

    ///Export simulation to matlab 
    void Simulation::WriteMatlabCode(System::String^ outDir, MatlabCodeWriteMode writeMode, ParameterNamesWriteMode parameterNamesWriteMode)
	{
		try
		{
			SimModelNative::MatlabODEExporter matlabExporter;
			matlabExporter.WriteMatlabCode(_simulation, 
										   NETToCPPConversions::MarshalString(outDir),
										   writeMode==MatlabCodeWriteMode::Formula);
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

    ///list of (initial) properties of ALL parameters of the simulation
	IList<IParameterProperties^>^ Simulation::ParameterProperties::get()
	{
		IList<IParameterProperties^>^ parameterProperties;

		try
		{
			parameterProperties = CPPToNETConversions::ParameterPropertiesListFrom(*_vecAllParameters,
				                                                                   _simulation->GetObjectPathDelimiter());
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

		return parameterProperties;
	}

    ///list of (initial) properties of ALL Species of the simulation
	IList<ISpeciesProperties^>^ Simulation::SpeciesProperties::get()
	{
		IList<ISpeciesProperties^>^ speciesProperties;

		try
		{
			speciesProperties = CPPToNETConversions::SpeciesPropertiesListFrom(*_vecAllSpecies,
				                                                               _simulation->GetObjectPathDelimiter());
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

		return speciesProperties;
	}

    ///set parameters which should be varied
	void Simulation::VariableParameters::set(IList<IParameterProperties^>^ parameterProperties)
	{
		try
		{
			//---- cache parameter properties
			std::map<long, SimModelNative::ParameterInfo> mapAllParameters;

			for(size_t i=0; i<_vecAllParameters->size(); i++)
			{
				SimModelNative::ParameterInfo & paramInfo = (*_vecAllParameters)[i];
				mapAllParameters[paramInfo.GetId()] = paramInfo;
			}

			_vecVariableParameters->clear();

			for each (IParameterProperties^ parameterProp in parameterProperties)
			{
				long id = parameterProp->Id;
				std::map<long, SimModelNative::ParameterInfo>::iterator iter = mapAllParameters.find(id);

				if (iter == mapAllParameters.end())
					throw gcnew System::Exception("Invalid parameter passed");

				SimModelNative::ParameterInfo & paramInfo = iter->second;

				paramInfo.SetCalculateSensitivity(parameterProp->CalculateSensitivity);
				
				_vecVariableParameters->push_back(paramInfo);
			}

			_simulation->SetVariableParameters(*_vecVariableParameters);
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

	IList<IParameterProperties^>^ Simulation::VariableParameters::get()
	{
		IList<IParameterProperties^>^ variableParameters;

		try
		{
			variableParameters = CPPToNETConversions::ParameterPropertiesListFrom(*_vecVariableParameters,
				                                                                  _simulation->GetObjectPathDelimiter());
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

		return variableParameters;
	}

    ///set species which should be varied
	void Simulation::VariableSpecies::set(IList<ISpeciesProperties^>^ speciesProperties)
	{
		try
		{
			//cache species properties
			std::map<long, SimModelNative::SpeciesInfo> mapAllSpecies;

			for(size_t i=0; i<_vecAllSpecies->size(); i++)
			{
				SimModelNative::SpeciesInfo & speciesInfo = (*_vecAllSpecies)[i];
				mapAllSpecies[speciesInfo.GetId()] = speciesInfo;
			}

			_vecVariableSpecies->clear();

			for each (ISpeciesProperties^ speciesProp in speciesProperties)
			{
				long id = speciesProp->Id;
				std::map<long, SimModelNative::SpeciesInfo>::iterator iter = mapAllSpecies.find(id);

				if (iter == mapAllSpecies.end())
					throw gcnew System::Exception("Invalid Species passed");

				SimModelNative::SpeciesInfo & speciesInfo = iter->second;
				
				_vecVariableSpecies->push_back(speciesInfo);
			}

			_simulation->SetVariableDEVariables(*_vecVariableSpecies);
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

	IList<ISpeciesProperties^>^ Simulation::VariableSpecies::get()
	{
		IList<ISpeciesProperties^>^ variableSpecies;

		try
		{
			variableSpecies = CPPToNETConversions::SpeciesPropertiesListFrom(*_vecVariableSpecies,
				                                                             _simulation->GetObjectPathDelimiter());
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

		return variableSpecies;
	}


	///set parameter initial values for the next simulation run
    void Simulation::SetParameterValues(IList<IParameterProperties^>^ parameterProperties)
	{	
		try
		{
			if (parameterProperties->Count != _vecVariableParameters->size())
				throw gcnew System::Exception("Invalid number of parameters passed");

			for (int i=0; i<parameterProperties->Count; i++)
			{
				IParameterProperties^ srcParameterProperty = parameterProperties[i];
				SimModelNative::ParameterInfo & targetParameterInfo = (*_vecVariableParameters)[i];

				if(srcParameterProperty->Id != targetParameterInfo.GetId())
					throw gcnew System::Exception("Invalid parameter id passed");

				if (srcParameterProperty->IsTable)
				{
					std::vector<SimModelNative::ValuePoint> targetPoints;
					IList <IValuePoint^ >^ srcPoints = srcParameterProperty->TablePoints;

					for(int pointIdx=0; pointIdx<srcPoints->Count; pointIdx++)
					{
						IValuePoint^ srcPoint = srcPoints[pointIdx];
						targetPoints.push_back(SimModelNative::ValuePoint(srcPoint->X, srcPoint->Y, srcPoint->RestartSolver));
					}
					targetParameterInfo.SetTablePoints(targetPoints);
				}
				else
				{
					if (SimModelNative::MathHelper::IsNaN(srcParameterProperty->Value))
						continue; //ignore NaNs

					targetParameterInfo.SetValue(srcParameterProperty->Value);
				}
			}
			
			_simulation->SetParametersValues(*_vecVariableParameters);
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

	///set species initial properties for the next simulation run
	void Simulation::SetSpeciesProperties(IList<ISpeciesProperties^>^ speciesProperties)
	{
		try
		{
			if (speciesProperties->Count != _vecVariableSpecies->size())
				throw gcnew System::Exception("Invalid number of species passed");

			for (int i=0; i<speciesProperties->Count; i++)
			{
				ISpeciesProperties^ srcSpeciesProperty = speciesProperties[i];
				SimModelNative::SpeciesInfo & targetSpeciesInfo = (*_vecVariableSpecies)[i];

				if(srcSpeciesProperty->Id != targetSpeciesInfo.GetId())
					throw gcnew System::Exception("Invalid species id passed");

				if (SimModelNative::MathHelper::IsNaN(srcSpeciesProperty->Value))
					continue; //ignore NaNs

				targetSpeciesInfo.SetValue(srcSpeciesProperty->Value);
			}
			
			_simulation->SetDEVariablesProperties(*_vecVariableSpecies);

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

    ///finalize simulation (perform internal optimizations etc.)
    void Simulation::FinalizeSimulation()
	{
		try
		{
			_simulation->Finalize();

			//---- update parameters and species infos (all values can be evaluated now)
			_simulation->FillParameterProperties(*_vecAllParameters);
			_simulation->FillParameterProperties(*_vecVariableParameters);
			_simulation->FillDEVariableProperties(*_vecAllSpecies);
			_simulation->FillDEVariableProperties(*_vecVariableSpecies);
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

    ///sets if simulation progress should be calculated
	bool Simulation::ShowProgress::get()
	{
		bool showProgress=false;

		try
		{
			showProgress = _simulation->Options().ShowProgress();
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

		return showProgress;
	}
	void Simulation::ShowProgress::set(bool showProgress)
	{
		try
		{
			_simulation->Options().SetShowProgress(showProgress);
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

    ///Max. simulation runtime (will be cancelled with Exception if exceeded)
    ///0 = unlimited (default)
	double Simulation::ExecutionTimeLimit::get()
	{
		double executionTimeLimit = 0.0;

		try
		{
			executionTimeLimit = _simulation->Options().ExecutionTimeLimit();
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

		return executionTimeLimit;
	}
	void Simulation::ExecutionTimeLimit::set(double executionTimeLimit)
	{
		try
		{
			_simulation->Options().SetExecutionTimeLimit(executionTimeLimit);
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

    ///sets if warnings should be treated as errors
    ///(for example numerical solver-warnings etc.)
    ///Default is FALSE
	bool Simulation::StopOnWarnings::get()
	{
		bool stopOnWarnings = false;

		try
		{
			stopOnWarnings = _simulation->Options().StopOnWarnings();
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

		return stopOnWarnings;
	}
	void Simulation::StopOnWarnings::set(bool stopOnWarnings)
	{
		try
		{
			_simulation->Options().SetStopOnWarnings(stopOnWarnings);
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

	bool Simulation::AutoReduceTolerances::get()
	{
		bool autoReduceTolerances = false;

		try
		{
			autoReduceTolerances = _simulation->Options().AutoReduceTolerances();
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

		return autoReduceTolerances;
	}
	void Simulation::AutoReduceTolerances::set(bool autoReduceTolerances)
	{
		try
		{
			_simulation->Options().SetAutoReduceTolerances(autoReduceTolerances);
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

	void Simulation::ReleaseMemory()
	{
		try
		{
			_simulation->ReleaseMemory();
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

	void Simulation::RunSimulation()
	{
		try
		{
			bool toleranceWasReduced;
			double newAbsTol, newRelTol;

			_simulation->RunSimulation(toleranceWasReduced, newAbsTol, newRelTol);

			_toleranceWasReduced = toleranceWasReduced;
			_newAbsTol = newAbsTol;
			_newRelTol = newRelTol;
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

    ///simulation progress in % during calculation
	int Simulation::Progress::get()
	{
		int progress = 0;

		try
		{
			progress = _simulation->GetProgress();
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

		return progress;
	}

    ///cancels current simulation run
    void Simulation::Cancel()
	{
		try
		{
			_simulation->Cancel();
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

    ///(numerical) warnings of ODE/DDE solver
	IList<ISolverWarning^>^ Simulation::SolverWarnings::get()
	{		
		IList<ISolverWarning^>^ solverWarnings;

		try
		{
			solverWarnings = gcnew List<ISolverWarning^>();

			SimModelNative::TObjectVector<SimModelNative::SolverWarning> warnings = 
				_simulation->SolverWarnings();

			for(int i=0; i<warnings.size(); i++)
			{
				solverWarnings->Add(gcnew SolverWarning(warnings[i]->Time(),
					CPPToNETConversions::MarshalString(warnings[i]->Message())));
			}
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

		return solverWarnings;
	}

	array<double>^ Simulation::SimulationTimes::get()
	{
		array<double>^ simTimes;

		try
		{
			simTimes =  CPPToNETConversions::DoubleArrayToArray(
				_simulation->GetTimeValues(), _simulation->GetNumberOfTimePoints());
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

		return simTimes;
	}

	IList<IValues^>^ Simulation::AllValues::get()
	{
		IList<IValues^>^ allValues;

		try
		{
			int i;

			allValues = gcnew List<IValues^>();

			//add species
			for(i=0; i<_simulation->SpeciesList().size(); i++)
			{
				if(!_simulation->SpeciesList()[i]->IsPersistable())
					continue;

				allValues->Add(ValuesFor(gcnew System::String(_simulation->SpeciesList()[i]->GetEntityId().c_str())));
			}
			
			//add observers
			for(i=0; i<_simulation->Observers().size(); i++)
			{
				if(!_simulation->Observers()[i]->IsPersistable())
					continue;

				allValues->Add(ValuesFor(gcnew System::String(_simulation->Observers()[i]->GetEntityId().c_str())));
			}
			
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

		return allValues;
	}

    IValues^ Simulation::ValuesFor(System::String^ entityId)
	{
		IValues^ variableValues;

		try
		{
			std::string entityIdCPP = NETToCPPConversions::MarshalString(entityId);
			SimModelNative::Quantity * quantity = NULL;

			//first, try to get observer with given entity id
			quantity = _simulation->Observers().GetObjectByEntityId(entityIdCPP);

			//if does not exists - try to get species with given entity id
			if (quantity == NULL)
				quantity = _simulation->SpeciesList().GetObjectByEntityId(entityIdCPP);

			if (quantity == NULL)
				throw gcnew System::ArgumentException(gcnew System::String(entityId + " is not a valid entity of system variable"));

			//make sure quantity is a "valued" variable
			SimModelNative::Variable * var = dynamic_cast <SimModelNative::Variable *> (quantity);
			if (var == NULL)
				throw gcnew System::ArgumentException(gcnew System::String(entityId + " is available in model but is not an entity with values(species, observer, ...)"));

			variableValues = gcnew VariableValues(quantity);
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

		return variableValues;
	}

	bool Simulation::ToleranceWasReduced::get()
	{
		return _toleranceWasReduced;
	}

	double Simulation::UsedAbsoluteTolerance::get()
	{
		return _newAbsTol;
	}

	double Simulation::UsedRelativeTolerance::get()
	{
		return _newRelTol;
	}

	bool Simulation::CheckForNegativeValues::get()
	{
		bool checkForNegativeValues = false;

		try
		{
			checkForNegativeValues = _simulation->Options().CheckForNegativeValues();
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

		return checkForNegativeValues;
	}

	void Simulation::CheckForNegativeValues::set(bool checkForNegativeValues)
	{
		try
		{
			_simulation->Options().SetCheckForNegativeValues(checkForNegativeValues);
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

	array<double>^ Simulation::SensitivityValuesFor(System::String^ entityId, System::String^ parameterEntityId)
	{
		array<double>^ values;

		try
		{
			std::string entityIdCPP = NETToCPPConversions::MarshalString(entityId);
			SimModelNative::Quantity * entity = _simulation->AllQuantities().GetObjectByEntityId(entityIdCPP);
			if (entity == NULL)
				throw gcnew System::ArgumentException(gcnew System::String(entityId + " is not a valid entity id of system variable or observer"));

			std::string parameterEntityIdCPP = NETToCPPConversions::MarshalString(parameterEntityId);
			SimModelNative::Quantity * parameter = _simulation->AllQuantities().GetObjectByEntityId(parameterEntityIdCPP);

			if (parameter == NULL)
				throw gcnew System::ArgumentException(gcnew System::String(parameterEntityId + " is not a valid entity id of a sensitivity parameter"));

			return SensitivityValuesByPathFor(entity->GetPathWithoutRoot(), parameter->GetPathWithoutRoot());
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

		return values;

	}

	array<double>^ Simulation::SensitivityValuesByPathFor(std::string entityPath, std::string parameterPath)
	{
		SimModelNative::TObjectList <SimModelNative::ParameterSensitivity> parameterSensitivities;
		int sizeToFill, idx;

		SimModelNative::Species * species = NULL;
		for (idx = 0; idx < _simulation->SpeciesList().size(); idx++)
		{
			if (_simulation->SpeciesList()[idx]->GetPathWithoutRoot() == entityPath)
			{
				species = _simulation->SpeciesList()[idx];

				if (!species->IsPersistable())
					throw gcnew System::ArgumentException(gcnew System::String("Cannot retrieve sensitivity information for "+CPPToNETConversions::MarshalString(entityPath) + ": variable is declared as nonpersistable"));

				sizeToFill = species->IsConstant(false) ? 1 : species->GetValuesSize();
				parameterSensitivities = species->ParameterSensitivities();
				break;
			}
		}

		if (species == NULL)
		{
			SimModelNative::Observer * observer = NULL;
			for (idx = 0; idx < _simulation->Observers().size(); idx++)
			{
				if (_simulation->Observers()[idx]->GetPathWithoutRoot() == entityPath)
				{
					observer = _simulation->Observers()[idx];

					if (!observer->IsPersistable())
						throw gcnew System::ArgumentException(gcnew System::String("Cannot retrieve sensitivity information for " + CPPToNETConversions::MarshalString(entityPath) + ": observer is declared as nonpersistable"));

					sizeToFill = observer->IsConstant(false) ? 1 : observer->GetValuesSize();
					parameterSensitivities = observer->ParameterSensitivities();
					break;
				}
			}

			if (observer == NULL)
				throw gcnew System::ArgumentException(gcnew System::String(CPPToNETConversions::MarshalString(entityPath) + " is not a valid path of system variable or observer"));
		}

		SimModelNative::ParameterSensitivity * paramSensitivity = NULL;
		for (idx = 0; idx < parameterSensitivities.size(); idx++)
		{
			if (parameterSensitivities[idx]->GetParameter()->GetPathWithoutRoot() == parameterPath)
			{
				paramSensitivity = parameterSensitivities[idx];
				break;
			}
		}

		if (paramSensitivity == NULL)
			throw gcnew System::ArgumentException(gcnew System::String(CPPToNETConversions::MarshalString(parameterPath) + " is not a valid path of a sensitivity parameter"));

		//number of sensitivities is identical with the number of output values of the corr. species
		return CPPToNETConversions::DoubleArrayToArray(paramSensitivity->GetValues(), sizeToFill);
	}

	array<double>^ Simulation::SensitivityValuesByPathFor(System::String^ entityPath, System::String^ parameterPath)
	{
		array<double>^ values;

		try
		{
			std::string entityPathCPP = NETToCPPConversions::MarshalString(entityPath);
			std::string parameterPathCPP = NETToCPPConversions::MarshalString(parameterPath);

			values = SensitivityValuesByPathFor(entityPathCPP, parameterPathCPP);
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

		return values;

	}

	IOutputSchema^ Simulation::OutputSchema::get()
	{
		SimModelNET::OutputSchema^ outputSchema;

		try
		{
			outputSchema = gcnew SimModelNET::OutputSchema();

			for each (IOutputInterval^ interval in outputSchema->OutputIntervalsFor(_simulation->GetOutputSchema()))
			{
				outputSchema->AddInterval(interval);
			}
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

		return outputSchema;
	}

	void Simulation::OutputSchema::set(IOutputSchema^ outputSchema)
	{
		try
		{
			_simulation->GetOutputSchema().Clear();

			for each (IOutputInterval^ interval in outputSchema->OutputIntervals)
			{
				_simulation->GetOutputSchema().OutputIntervals().push_back(
					new SimModelNative::OutputInterval(interval->StartTime, interval->EndTime, interval->NumberOfTimePoints, SimModelNative::OutputIntervalDistribution::Equidistant));
			}
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
}
