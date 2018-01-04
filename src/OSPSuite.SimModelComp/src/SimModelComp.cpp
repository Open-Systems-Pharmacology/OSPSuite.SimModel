#include "SimModelComp/SimModelComp.h" 
#include "SimModel/Simulation.h"
#include "XMLWrapper/XMLCache.h"
#include "DCI/DCI.h"
#include "DCI/Manager.h"
#include "SimModelComp/SimModelComp_XMLHelper.h"
#include "SysToolConfig.h"
#include "SimModel/OutputSchema.h"
#include "SimModelComp/SimModelComp_HelpFunctions.h"
#include "SimModel/MatlabODEExporter.h"
#include "../../OSPSuite.SimModelComp/version.h"

using namespace SimModelNative;
using namespace std;

const char * SimModelComp::conID = "ID";
const char * SimModelComp::conPath = "Path";
const char * SimModelComp::conDescription = "Description";
const char * SimModelComp::conValue = "Value";
const char * SimModelComp::conInitialValue = "InitialValue";
const char * SimModelComp::conUnit = "Unit";
const char * SimModelComp::conFormula = "Formula";
const char * SimModelComp::conIsFormula = "IsFormula";
const char * SimModelComp::conScaleFactor = "ScaleFactor";
const char * SimModelComp::conIsVariable = "IsVariable";
const char * SimModelComp::conStartTime = "StartTime";
const char * SimModelComp::conEndTime = "EndTime";
const char * SimModelComp::conNoOfTimePoints = "NoOfTimePoints";
const char * SimModelComp::conDistribution = "Distribution";
const char * SimModelComp::conParameterType = "ParameterType";
const char * SimModelComp::conTime="Time";
const char * SimModelComp::conRestartSolver="RestartSolver";
const char * SimModelComp::conCalculateSensitivity = "CalculateSensitivity";

const char * SimModelComp::conParameterTypeFormula="Formula";
const char * SimModelComp::conParameterTypeValue="Value";
const char * SimModelComp::conParameterTypeTable="Table";


const unsigned int SimModelComp::conTabAllParameter = 1;
const unsigned int SimModelComp::conTabVariableParameter = 2;
const unsigned int SimModelComp::conTabAllSpecies = 3;
const unsigned int SimModelComp::conTabVariableSpecies = 4;
const unsigned int SimModelComp::conTabAllObserver = 5;
const unsigned int SimModelComp::conTabOutputSchema = 6;
const unsigned int SimModelComp::conTabCurveOutputSchema = 7;
const unsigned int SimModelComp::conTabAllTableParameter=8;
const unsigned int SimModelComp::conTabVariableTableParameter=9;

const unsigned int SimModelComp::conTabOutputTimes = 1;
const unsigned int SimModelComp::conTabOutputValues = 2;
const unsigned int SimModelComp::conTabOutputSensitivities = 3;

const char * SimModelComp::conAttrID = "ID";
const char * SimModelComp::conAttrPath = "Path";
const char * SimModelComp::conAttrType = "Type";
const char * SimModelComp::conAttrTimeColumnIndex = "TimeColumnIndex";
const char * SimModelComp::conAttrSensitivityParameterID = "SensitivityParameterID";

const char * SimModelComp::conEquidistantDistribution = "Equidistant";
const char * SimModelComp::conLogarithmicDistribution = "Logarithmic";

SimModelComp::SimModelComp()
{
	m_Sim = new Simulation();
	m_XMLSchemaCache = XMLCache::GetInstance();
	m_IsConfigured = false;
	m_ProcessMetaDataPerformed = false;
	m_ProcessDataFirstRun = true;
}

SimModelComp::~SimModelComp()
{
	try
	{
		if (m_Sim)
		{
			delete m_Sim;
			m_Sim = NULL;
		}
	}
	catch(...){}
}


//---- loads simulation from XML and 
//initially fills all input tables of the DCI component
DCI::Bool SimModelComp::Configure()
{
	bool RetVal=false;
	m_IsConfigured = false;
	m_ProcessMetaDataPerformed = false;

	try
	{
		string ErrorMsg;

		assert(m_Sim != NULL);

		//delete old Simulation
		delete m_Sim;
		m_Sim = new Simulation();
		m_Sim->Options().SetKeepXMLNodeAsString(true);
		m_Sim->Options().SetUseFloatComparisonInUserOutputTimePoints(false);

		//---- check tables
		if (GetParameterPorts()->GetCount()==0)
			throw "No parameter ports found";

		if (GetInputPorts()->GetCount() != 9)
			throw "Component must have exactly nine input ports";

		if (GetOutputPorts()->GetCount() != 3)
			throw "Component must contain exactly three ouput ports";

		LoadConfigurationFromParameterTable();

		SimModelComp_XMLHelper HelpMe;

		//get simulation XML string
		string SimulationXML;

		if (!HelpMe.XMLStringFromFile(m_SimFileName,SimulationXML,ErrorMsg))
			throw ErrorMsg;

		//load simulation from XML Strings
		m_Sim->LoadFromXMLString(SimulationXML);

		//get all simulation Parameters
		m_Sim->FillParameterProperties(m_AllParameters);

		//---- fill AllParameters-Table
		FillParameterInputTable(GetInputPorts()->Item(conTabAllParameter), m_AllParameters, 
			                    INITIAL_FILL, ALL_QUANTITIES);
		
		//---- fill VariableParameters-Table
		FillParameterInputTable(GetInputPorts()->Item(conTabVariableParameter), m_AllParameters, 
			                    INITIAL_FILL, VARIABLE_QUANTITIES);
		DCI::ITableHandle hTab = GetInputPorts()->Item(conTabVariableParameter)->GetTable();
		AddByteColumn(hTab, conIsVariable);
		AddByteColumn(hTab, conCalculateSensitivity);

		//---- fill AllTableParameters-Table
		FillTableParameterPointsInputTable(GetInputPorts()->Item(conTabAllTableParameter), m_AllParameters, 
 			                               INITIAL_FILL, ALL_QUANTITIES);

        //---- fill VariableTableParameters-Table
		FillTableParameterPointsInputTable(GetInputPorts()->Item(conTabVariableTableParameter), m_AllParameters, 
			                               INITIAL_FILL, VARIABLE_QUANTITIES);

		//get all simulation Species
		m_Sim->FillDEVariableProperties(m_AllSpecies);

		//---- fill AllSpecies-Table
		FillSpeciesInputTable(GetInputPorts()->Item(conTabAllSpecies), m_AllSpecies, 
			                  INITIAL_FILL, ALL_QUANTITIES);

		//---- fill variable species table
		FillSpeciesInputTable(GetInputPorts()->Item(conTabVariableSpecies), m_AllSpecies, 
			                  INITIAL_FILL, VARIABLE_QUANTITIES);
		AddByteColumn(GetInputPorts()->Item(conTabVariableSpecies)->GetTable(), conIsVariable);

		//---- fill observers table
		FillObserversInputTable();

		//---- fill output raster
		FillOutputSchemaInputTable();

		//---- fill curve specific output raster
		FillCurvesOutputSchemaInputTable();

		m_IsConfigured = true;
		RetVal=true;
		
	}
	catch(const DCI::String & msg) { ErrorHandler(msg); }
	catch(const char *msg)	  { ErrorHandler(msg); }
	catch(const string & msg){ErrorHandler(msg.c_str());}
	catch(const ErrorData & ED) {ErrorHandler(ED);}
	catch(...)				  { ErrorHandler("Unknown Error in Configure"); }

	return RetVal;
}

//
//
void SimModelComp::LoadConfigurationFromParameterTable()
{
	DCI::ITableHandle hTab;
	DCI::IVariableHandle hVar;
	DCI::StringVector sVec;
	DCI::IntVector lVec;
	DCI::DoubleVector dVec;

	hTab = GetParameterPorts()->Item(1)->GetTable();
	if (!hTab)
		throw "Parameter Table not available";

	//init Simulation Schema cache if not done yet
	if (!m_XMLSchemaCache->SchemaInitialized())
	{
		hVar = GetVarHandle(hTab, "SimModelSchema", DCI::DT_STRING);
		sVec = hVar->GetValues();
		if (sVec.Len() != 1)
			throw "Parameter column \"SimModelSchema\" must contain exactly one value";
		m_XMLSchemaCache->SetSchemaNamespace(XMLConstants::GetSchemaNamespace());
		m_XMLSchemaCache->LoadSchemaFromFile((const char *)(sVec[0]));
	}

	//get simulation file name
	hVar = GetVarHandle(hTab, "SimulationFile", DCI::DT_STRING);
	sVec = hVar->GetValues(); 
	if (sVec.Len() != 1)
		throw "Parameter column \"SimulationFile\" must contain exactly one value";
	m_SimFileName = (const char *)(sVec[0]);

	//read StopOnWarnings-flag if available
	hVar = GetVarHandle(hTab, "StopOnWarnings", DCI::DT_INT, true);
	if(hVar)
	{
		lVec = hVar->GetValues(); 
		if (lVec.Len() > 0)
		{
			if (lVec[0] == 1)
				m_Sim->Options().SetStopOnWarnings(true);
			else
				m_Sim->Options().SetStopOnWarnings(false);
		}
	}

	//read ExecutionTimeLimit-flag if available
	hVar = GetVarHandle(hTab, "ExecutionTimeLimit", DCI::DT_DOUBLE, true);
	if(hVar)
	{
		dVec = hVar->GetValues(); 
		if (dVec.Len() > 0)
		{
			double ExecutionTimeLimit = dVec[0];
			if (ExecutionTimeLimit > 0.0)
				m_Sim->Options().SetExecutionTimeLimit(ExecutionTimeLimit * 1000); //s. -> ms.
		}
	}
}

//---- sets which parameters should be varied and finalizes the simulation
//
DCI::Bool SimModelComp::ProcessMetaData()
{
	bool RetVal=false;

	try
	{
		if(!m_IsConfigured)
			throw "Cannot proceed with 'ProcessMetaData': 'Configure' was not called or failed";

		if (m_ProcessMetaDataPerformed)
			throw "'ProcessMetaData' was already performed.\nCall 'Configure' to reinitialize the simulation";

		//get all simulation parameters that will be varied
		m_VariableParameters = GetVariableParameters();

		//set variable parameters into simulation
		m_Sim->SetVariableParameters(m_VariableParameters);

		//get all variable species
		m_VariableSpecies = GetVariableSpecies();

		//set variable species into simulation
		m_Sim->SetVariableDEVariables(m_VariableSpecies);

		//Finalize
		m_Sim->Finalize();

		//---- refresh parameter properties (formula values can be obtained now)
		//     and fill AllParameters-Table
		m_Sim->FillParameterProperties(m_AllParameters);
		FillParameterInputTable(GetInputPorts()->Item(conTabAllParameter), m_AllParameters, 
			                    FURTHER_FILL, ALL_QUANTITIES);

		//---- refresh variable parameter properties and fill VariableParameters-Table
		m_Sim->FillParameterProperties(m_VariableParameters);
		FillParameterInputTable(GetInputPorts()->Item(conTabVariableParameter), m_VariableParameters, 
			                    FURTHER_FILL, VARIABLE_QUANTITIES);

		//---- refresh parameter properties
		//     and fill AllTableParameters-Table
		FillTableParameterPointsInputTable(GetInputPorts()->Item(conTabAllTableParameter), m_AllParameters, 
 			                               FURTHER_FILL, ALL_QUANTITIES);

        //---- refresh variable parameter properties and fill  VariableTableParameters-Table
		FillTableParameterPointsInputTable(GetInputPorts()->Item(conTabVariableTableParameter), m_VariableParameters, 
			                               FURTHER_FILL, VARIABLE_QUANTITIES);


		//---- refresh DE variables properties (formula values can be obtained now)
		//     and fill AllSpecies-Table
		m_Sim->FillDEVariableProperties(m_AllSpecies);
		FillSpeciesInputTable(GetInputPorts()->Item(conTabAllSpecies), m_AllSpecies, 
			                  FURTHER_FILL, ALL_QUANTITIES);

		//---- refresh variable species properties and fill VariableParameters-Table
		m_Sim->FillDEVariableProperties(m_VariableSpecies);
		FillSpeciesInputTable(GetInputPorts()->Item(conTabVariableSpecies), m_VariableSpecies, 
			                  FURTHER_FILL, VARIABLE_QUANTITIES);
		
		//---- If simulation contains persistable parameters, new observers were added.
		//     Observers table must be adjusted in this case
		if (m_Sim->ContainsPersistableParameters())
			FillObserversInputTable();

		m_ProcessMetaDataPerformed = true;
		m_ProcessDataFirstRun = true;

		RetVal=true;
	}
	catch(const DCI::String & msg) { ErrorHandler(msg); }
	catch(const char *msg)	  { ErrorHandler(msg); }
	catch(const string & msg){ErrorHandler(msg.c_str());}
	catch(const ErrorData & ED) {ErrorHandler(ED);}
	catch(...)				  { ErrorHandler("Unknown Error in ProcessMetaData"); }

	return RetVal;
}

//---- set new parameter values, species properties, output time schema etc.
void SimModelComp::UpdateSimulationSettings()
{
	DCI::ITableHandle hTab;

	if(!m_ProcessMetaDataPerformed)
		return;

	//get and check input table
	hTab = GetInputPorts()->Item(conTabVariableParameter)->GetTable();
	CheckInputTableColumns(hTab, conTabVariableParameter, false);

	//get and check species properties table
	hTab = GetInputPorts()->Item(conTabVariableSpecies)->GetTable();
	CheckInputTableColumns(hTab, conTabVariableSpecies, false);

	//Update variable parameters from input data tables
	UpdateVariableParameters();

	//update variable parameters in the simulation
	m_Sim->SetParametersValues(m_VariableParameters);

	//update variable species properties
	UpdateVariableSpeciesProperties();

	//update DE variable properties in the simulation
	m_Sim->SetDEVariablesProperties(m_VariableSpecies);

	//update output schema
	UpdateOutputTimeSchema();
}


//---- sets new parameter values/species initial values and performs one simulation run
//
DCI::Bool SimModelComp::ProcessData()
{
	bool RetVal=false;

	try
	{
		if(!m_ProcessMetaDataPerformed)
			throw "Cannot proceed with 'ProcessData': 'ProcessMetaData' was not called or failed";

		//update parameter values, species properties, time schema etc.
		UpdateSimulationSettings();

		//run simulation
		bool toleranceWasReduced;
		double newAbsTol, newRelTol;
		m_Sim->RunSimulation(toleranceWasReduced, newAbsTol, newRelTol);

		//fill output tables with output times and values of the simulation
		FillOutputTables();

		//---- refresh parameter properties and fill AllParameters-Table
		m_Sim->FillParameterProperties(m_AllParameters);
		FillParameterInputTable(GetInputPorts()->Item(conTabAllParameter), m_AllParameters,
			FURTHER_FILL, ALL_QUANTITIES);

		//---- refresh DE variables properties and fill AllSpecies-Table
		m_Sim->FillDEVariableProperties(m_AllSpecies);
		FillSpeciesInputTable(GetInputPorts()->Item(conTabAllSpecies), m_AllSpecies,
			FURTHER_FILL, ALL_QUANTITIES);

		m_ProcessDataFirstRun = false;

		RetVal=true;
	}
	catch(const DCI::String & msg) { ErrorHandler(msg); }
	catch(const char *msg)	  { ErrorHandler(msg); }
	catch(const string & msg){ErrorHandler(msg.c_str());}
	catch(const ErrorData & ED) {ErrorHandler(ED);}
	catch(...)				  { ErrorHandler("Unknown Error in ProcessData"); }

	return RetVal;
}

//---- fills output tables with times and values of the simulation
//
void SimModelComp::FillOutputTables()
{
	DCI::IVariableHandle hVar;
	int i;

	DCI::ITableHandle hTabTimes = GetOutputPorts()->Item(conTabOutputTimes)->GetTable();
	DCI::ITableHandle hTabValues = GetOutputPorts()->Item(conTabOutputValues)->GetTable();
	DCI::ITableHandle hTabSensitivities = GetOutputPorts()->Item(conTabOutputSensitivities)->GetTable();

	if (m_ProcessDataFirstRun)
	{
		//create output tables
		BindNewTableTo(GetOutputPorts()->Item(conTabOutputTimes), hTabTimes, false);
		BindNewTableTo(GetOutputPorts()->Item(conTabOutputValues), hTabValues, false);
		BindNewTableTo(GetOutputPorts()->Item(conTabOutputSensitivities), hTabSensitivities, false);
	}

	//---- remove previous values if available
	hTabTimes->GetColumns()->Clear();
	hTabValues->GetRecords()->Clear();
	hTabSensitivities->GetRecords()->Clear();

	hVar = hTabTimes->GetColumns()->AddNew("Time");
	hVar->GetFieldDef()->SetName("Time");
	hVar->GetFieldDef()->SetDataType(DCI::DT_DOUBLE);

	int noOfTimeSteps = m_Sim->GetNumberOfTimePoints();
	double * simTimeValues = m_Sim->GetTimeValues();
	DCI::DoubleVector timesVec(simTimeValues, noOfTimeSteps);

	hVar->SetValues(timesVec);

	for(i=0; i<m_Sim->SpeciesList().size(); i++)
	{
		Species * species = m_Sim->SpeciesList()[i];
		if (!species->IsPersistable())
			continue;

		AddValuesToOutputTables(hTabValues, hTabSensitivities, species, "Species", m_ProcessDataFirstRun,
			                    species->GetFullName(), species->GetName(), species->GetId());
	}

	for(i=0; i<m_Sim->Observers().size(); i++)
	{
		Observer * obs = m_Sim->Observers()[i];
		if (!obs->IsPersistable())
			continue;

		AddValuesToOutputTables(hTabValues, hTabSensitivities, obs, "Observer", m_ProcessDataFirstRun,
			                    obs->GetFullName(), obs->GetName(), obs->GetId());
	}
}

void SimModelComp::BindNewTableTo(DCI::IPortHandle & hPort, DCI::ITableHandle & hTab, bool recordBased)
{
	hTab.BindTo(new DCI::Table);
	hTab->SetRecordBased(recordBased);
	hPort->SetTable(hTab);
}

//
//
DCI::IVariableHandle SimModelComp::AddOutputVariable(DCI::ITableHandle & hTab,
												     const std::string & Key,
													 const long id,
													 const std::string & path,
													 const std::string & type,
													 const int timeColIdx,
													 const std::string Name)
{
	DCI::IVariableHandle hVar;
	DCI::IFieldDefHandle hFieldDef;
	DCI::IAttributeHandle hAttr;

	hVar = hTab->GetColumns()->AddNew(Key.c_str());
	if(!hVar)
		throw "Cannot add variable with key \""+Key+"\"";

	hFieldDef = hVar->GetFieldDef();

	hFieldDef->SetName(Name.c_str());
	hFieldDef->SetDataType(DCI::DT_DOUBLE);

	hAttr = hFieldDef->GetAttributes()->AddNew(conAttrID);
	hAttr->SetName(conAttrID);
	hAttr->SetStringValue(XMLHelper::ToString(id).c_str());

	hAttr = hFieldDef->GetAttributes()->AddNew(conAttrPath);
	hAttr->SetName(conAttrPath);
	hAttr->SetStringValue(path.c_str());

	hAttr = hFieldDef->GetAttributes()->AddNew(conAttrType);
	hAttr->SetName(conAttrType);
	hAttr->SetStringValue(type.c_str());

	hAttr = hFieldDef->GetAttributes()->AddNew(conAttrTimeColumnIndex);
	hAttr->SetName(conAttrTimeColumnIndex);
	hAttr->SetStringValue(XMLHelper::ToString(timeColIdx).c_str());

	return hVar;
}

//
//
void SimModelComp::AddValuesToOutputTables(DCI::ITableHandle & hTabOutputValues,
	                                       DCI::ITableHandle & hTabOutputSensitivities,
	                                       VariableWithParameterSensitivity * pVariable,
										   const string TypeName,
										   bool ProcessDataFirstRun,
										   const string & FullPath,
										   const string & Name,
										   const long id)
{
	//currently all outputs refer to the same time vector
	const int timeColIdx = 1;

	DCI::IVariableHandle hVar;
	string Key = FullPath;

	if (ProcessDataFirstRun)
		hVar = AddOutputVariable(hTabOutputValues, Key, id, FullPath, TypeName, timeColIdx, Name);
	else
		hVar = hTabOutputValues->GetColumn(Key.c_str());

	DCI::DoubleVector dVec(pVariable->GetValues(), pVariable->GetValuesSize());
	hVar->SetValues(dVec);

	AddSensitivitiesToOutputTable(hTabOutputSensitivities, pVariable, TypeName, ProcessDataFirstRun, FullPath, Name, id);
}

void SimModelComp::AddSensitivitiesToOutputTable(DCI::ITableHandle & hTab,
                                                 VariableWithParameterSensitivity * pVariable,
                                                 const std::string TypeName,
                                                 bool ProcessDataFirstRun,
                                                 const std::string & FullPath,
                                                 const std::string & Name,
                                                 const long id)
{
	//currently all outputs refer to the same time vector
	const int timeColIdx = 1;

	for (size_t paramIdx = 0; paramIdx < m_SensitivityParameters.size(); paramIdx++)
	{
		ParameterInfo & sensitivityParameterInfo = m_SensitivityParameters[paramIdx];
		long sensitivityParameterId = sensitivityParameterInfo.GetId();

		DCI::IVariableHandle hVar;
		string Key = FullPath + m_Sim->GetObjectPathDelimiter() + sensitivityParameterInfo.GetFullName();

		if (ProcessDataFirstRun)
		{
			hVar = AddOutputVariable(hTab, Key, id, FullPath, TypeName, timeColIdx, Name);

			//add sensitivity parameter id as attribute
			DCI::IAttributeHandle hAttr;
			hAttr = hVar->GetFieldDef()->GetAttributes()->AddNew(conAttrSensitivityParameterID);

			hAttr->SetName(conAttrSensitivityParameterID);
			hAttr->SetStringValue(XMLHelper::ToString(sensitivityParameterId).c_str());
		}
		else
			hVar = hTab->GetColumn(Key.c_str());

		DCI::DoubleVector dVec(pVariable->ParameterSensitivities().GetObjectById(sensitivityParameterId)->GetValues(), 
			                   pVariable->GetValuesSize());
		hVar->SetValues(dVec);
	}
}


//
//
DCI::Bool SimModelComp::Initialize(const DCI::String &ComponentTypeName, const DCI::String &ComponentTypeDescription)
{
	bool RetVal=false;
	Component::Initialize(ComponentTypeName, ComponentTypeDescription);
	try
	{
		//nothing to do
		RetVal=true;
	}
	catch(const DCI::String & msg) { ErrorHandler(msg); }
	catch(const char *msg)	  { ErrorHandler(msg); }
	catch(const string & msg){ErrorHandler(msg.c_str());}
	catch(...)				  { ErrorHandler("Unknown Error in Initialize"); }

	return RetVal;
}

//
//
void SimModelComp::SetName(const DCI::String &newName)
{
	Object::SetName(newName);
}

//
//
DCI::Bool SimModelComp::CheckRunnable()
{
	bool RetVal=false;
	try
	{
		//nothing to do

		RetVal=true;
	}
	catch(const DCI::String & msg) { ErrorHandler(msg); }
	catch(const char *msg)	  { ErrorHandler(msg); }
	catch(const string & msg){ErrorHandler(msg.c_str());}
	catch(const ErrorData & ED) {ErrorHandler(ED);}
	catch(...)				  { ErrorHandler("Unknown Error in CheckRunnable"); }

	return RetVal;
}

void SimModelComp::FillCurvesOutputSchemaInputTable()
{
	//---- get curves output schema table
	DCI::IPortHandle  hPort = GetInputPorts()->Item(conTabCurveOutputSchema);
	DCI::ITableHandle hTab  = hPort->GetTable();

	//---- create new table and setup column structure
	hTab.BindTo(new DCI::Table);
	hPort->SetTable(hTab);

	//---- set up table structure
	AddColumn(hTab, conID, DCI::DT_INT);
	AddColumn(hTab, conPath, DCI::DT_STRING);

	AddOutputSchemaTableColumns(hTab);
}

void SimModelComp::FillObserversInputTable()
{
	//---- get curves output schema table
	DCI::IPortHandle  hPort = GetInputPorts()->Item(conTabAllObserver);
	DCI::ITableHandle hTab  = hPort->GetTable();

	//---- create new table and setup column structure
	hTab.BindTo(new DCI::Table);
	hPort->SetTable(hTab);

	//---- set up table structure
	AddObserversTableColumns(hTab);

	//---- Get observer infos
	vector<QuantityInfo> observerInfos;
	m_Sim->FillObserverProperties(observerInfos);

	//---- Add table records
	hTab->ReDim((DCI::UInt)observerInfos.size(), hTab->GetColumns()->GetCount());

	//---- Fill observer table from SimModel info
	for(size_t i=0; i<observerInfos.size(); i++)
	{
		QuantityInfo observerInfo = observerInfos[i];

		hTab->SetValue((DCI::UInt)i+1, conID, observerInfo.GetId());
		hTab->SetValue((DCI::UInt)i+1, conPath, observerInfo.GetFullName().c_str());
		hTab->SetValue((DCI::UInt)i+1, conUnit, observerInfo.GetUnit().c_str());
		hTab->SetValue((DCI::UInt)i+1, conFormula, observerInfo.GetFormulaEquation().c_str());
		hTab->SetValue((DCI::UInt)i+1, conDescription, observerInfo.GetDescription().c_str());
	}
}

void SimModelComp::UpdateOutputTimeSchema()
{
	//---- get output schema table
	DCI::IPortHandle  hPort = GetInputPorts()->Item(conTabOutputSchema);
	DCI::ITableHandle hTab  = hPort->GetTable();

	OutputSchema & outSchema = m_Sim->GetOutputSchema();
	outSchema.Clear();

	for(int intervalIdx=1; intervalIdx<=hTab->GetRecords()->GetCount(); intervalIdx++)
	{
		double startTime       = hTab->GetValue(intervalIdx, conStartTime);
		double endTime         = hTab->GetValue(intervalIdx, conEndTime);
		int numberOfTimePoints = hTab->GetValue(intervalIdx, conNoOfTimePoints);

		OutputIntervalDistribution pointsDistribution = IntervalDistributionFromString(
			                     hTab->GetValue(intervalIdx, conDistribution));

		OutputInterval * interval = new OutputInterval
			(startTime, endTime, numberOfTimePoints, pointsDistribution);

		outSchema.OutputIntervals().push_back(interval);
	}
}

void SimModelComp::FillOutputSchemaInputTable()
{
	//---- get output schema table
	DCI::IPortHandle  hPort = GetInputPorts()->Item(conTabOutputSchema);
	DCI::ITableHandle hTab  = hPort->GetTable();

	//---- create new table and setup column structure
	hTab.BindTo(new DCI::Table);
	hPort->SetTable(hTab);
	AddOutputSchemaTableColumns(hTab);
	
	//---- fill table from simulation output schema
	OutputSchema & outSchema = m_Sim->GetOutputSchema();

	//Add table records
	hTab->ReDim(outSchema.OutputIntervals().size(), hTab->GetColumns()->GetCount());

	for(size_t intervalIdx=1; intervalIdx<=outSchema.OutputIntervals().size(); intervalIdx++)
	{
		OutputInterval * outInterval = outSchema.OutputIntervals()[(unsigned int)intervalIdx-1];

		hTab->SetValue((DCI::UInt)intervalIdx, conStartTime, outInterval->StartTime());
		hTab->SetValue((DCI::UInt)intervalIdx, conEndTime, outInterval->EndTime());
		hTab->SetValue((DCI::UInt)intervalIdx, conUnit, "min");
		hTab->SetValue((DCI::UInt)intervalIdx, conNoOfTimePoints, (DCI::Int)outInterval->NumberOfTimePoints());
		hTab->SetValue((DCI::UInt)intervalIdx, conDistribution, IntervalDistributionAsString(outInterval->IntervalDistribution()).c_str());
	}
}

void SimModelComp::AddOutputSchemaTableColumns(const DCI::ITableHandle hTab)
{
	AddColumn(hTab, conStartTime, DCI::DT_DOUBLE);
	AddColumn(hTab, conEndTime, DCI::DT_DOUBLE);
	AddColumn(hTab, conUnit, DCI::DT_STRING);
	AddColumn(hTab, conNoOfTimePoints, DCI::DT_INT);
	AddColumn(hTab, conDistribution, DCI::DT_STRING);
}

void SimModelComp::UpdateVariableParameters(void)
{
	const char * ERROR_SOURCE = "SimModelComp::UpdateVariableParameters";

	DCI::ITableHandle hTab = GetInputPorts()->Item(conTabVariableParameter)->GetTable();
	DCI::IntVector  idVec = hTab->GetColumn(conID)->GetValues();
	DCI::DoubleVector  valueVec = hTab->GetColumn(conValue)->GetValues();

	//---- check no. of records is identical to no of variable parameters
	if (hTab->GetRecords()->GetCount() != m_VariableParameters.size())
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
		                "Number of records in the variable parameters table not equal to the number of variable parameters");

	DCI::ITableHandle hTabPoints    = GetInputPorts()->Item(conTabVariableTableParameter)->GetTable();
	DCI::IntVector idVecPoints      = hTabPoints->GetColumn(conID)->GetValues();
	DCI::DoubleVector XPoints       = hTabPoints->GetColumn(conTime)->GetValues();
	DCI::DoubleVector YPoints       = hTabPoints->GetColumn(conValue)->GetValues();
	DCI::ByteVector   RestartSolver = hTabPoints->GetColumn(conRestartSolver)->GetValues();
	size_t noOfPoints = idVecPoints.Len();

	//---- cache ids of table parameters
	set<DCI::Int> tablePointsIds;
	size_t pointIdx;
	for(pointIdx=0; pointIdx<noOfPoints; pointIdx++)
		tablePointsIds.insert(idVecPoints[pointIdx]);

	//---- set variable parameters
	for(size_t i=0; i<m_VariableParameters.size(); i++)
	{
		ParameterInfo & paramInfo = m_VariableParameters[i];

		long paramId = paramInfo.GetId();

		//make sure we have corresponding parameter in the table
		if (paramId != idVec[i])
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
			"Parameter id " + XMLHelper::ToString(idVec[i]) + " does not match");

		//---- set new value
		if(tablePointsIds.find(paramId) != tablePointsIds.end())
		{
			//---- parameter id is present in the table points ==> parameter is a table

			vector<ValuePoint> tablePoints;
			for(pointIdx=0; pointIdx<noOfPoints; pointIdx++)
			{
				if (idVecPoints[pointIdx] != paramId)
					continue;

				ValuePoint vp(XPoints[pointIdx], YPoints[pointIdx], RestartSolver[pointIdx]==1);
				tablePoints.push_back(vp);
			}

			paramInfo.SetTablePoints(tablePoints);
		}
		else
		{
			//---- parameter id is NOT present in the table points ==> parameter is single value

			paramInfo.SetValue(valueVec[i]);
		}
	}
}

void SimModelComp::UpdateVariableSpeciesProperties(void)
{
	const char * ERROR_SOURCE = "SimModelComp::UpdateVariableSpeciesProperties";

	DCI::ITableHandle hTab = GetInputPorts()->Item(conTabVariableSpecies)->GetTable();
	DCI::IntVector  idVec = hTab->GetColumn(conID)->GetValues();
	DCI::DoubleVector initValueVec   = hTab->GetColumn(conInitialValue)->GetValues();
	DCI::DoubleVector scaleFactorVec = hTab->GetColumn(conScaleFactor)->GetValues();

	//---- check no. of records is identical to no of variable species
	if (hTab->GetRecords()->GetCount() != m_VariableSpecies.size())
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
		                "Number of records in the variable species table not equal to the number of variable species");

	for(size_t i=0; i<m_VariableSpecies.size(); i++)
	{
		SpeciesInfo & speciesInfo = m_VariableSpecies[i];

		//make sure we have corresponding species in the table
		if (speciesInfo.GetId() != idVec[i])
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
			"Variable id " + XMLHelper::ToString(idVec[i]) + " does not match");

		//set new value
		speciesInfo.SetValue(initValueVec[i]);
		speciesInfo.SetScaleFactor(scaleFactorVec[i]);
	}
}

void SimModelComp::CheckInputTableColumns(const DCI::ITableHandle hTab, unsigned int TableID,
										  bool IsVariableColumnPresent)
{
	if (!hTab->GetRecordBased())
		throw "Input Table \""+hTab->GetName()+"\" is not record based";

	if ((TableID == conTabAllParameter) || (TableID == conTabVariableParameter))
	{
		CheckColumn(hTab, conID, DCI::DT_INT);
		CheckColumn(hTab, conPath, DCI::DT_STRING);
		CheckColumn(hTab, conValue, DCI::DT_DOUBLE);
		CheckColumn(hTab, conUnit, DCI::DT_STRING);
		CheckColumn(hTab, conParameterType, DCI::DT_STRING);
		CheckColumn(hTab, conFormula, DCI::DT_STRING);
		CheckColumn(hTab, conDescription, DCI::DT_STRING);
		
		if (IsVariableColumnPresent)
		{
			CheckColumn(hTab, conIsVariable, DCI::DT_BYTE);
			CheckColumn(hTab, conCalculateSensitivity, DCI::DT_BYTE);
		}
	}
	else if ((TableID == conTabAllTableParameter) || (TableID == conTabVariableTableParameter))
	{
		CheckColumn(hTab, conID, DCI::DT_INT);
		CheckColumn(hTab, conTime, DCI::DT_DOUBLE);
		CheckColumn(hTab, conValue, DCI::DT_DOUBLE);
		CheckColumn(hTab, conRestartSolver, DCI::DT_BYTE);
	}
	else if ((TableID == conTabAllSpecies) || ((TableID == conTabVariableSpecies)))
	{
		CheckColumn(hTab, conID, DCI::DT_INT);
		CheckColumn(hTab, conPath, DCI::DT_STRING);
		CheckColumn(hTab, conInitialValue, DCI::DT_DOUBLE);
		CheckColumn(hTab, conScaleFactor, DCI::DT_DOUBLE); 
		CheckColumn(hTab, conUnit, DCI::DT_STRING);
		CheckColumn(hTab, conIsFormula, DCI::DT_BYTE);
		CheckColumn(hTab, conFormula, DCI::DT_STRING);
		CheckColumn(hTab, conDescription, DCI::DT_STRING);

		if (IsVariableColumnPresent)
		{
			CheckColumn(hTab, conScaleFactor, DCI::DT_DOUBLE); 
			CheckColumn(hTab, conIsVariable, DCI::DT_BYTE);
		}
	}
	else if (TableID == conTabAllObserver)
	{
		CheckColumn(hTab, conID, DCI::DT_INT);
		CheckColumn(hTab, conPath, DCI::DT_STRING);
		CheckColumn(hTab, conFormula, DCI::DT_STRING);
		CheckColumn(hTab, conDescription, DCI::DT_STRING);
	}
	else if (TableID == conTabOutputSchema)
	{
		CheckColumn(hTab, conStartTime, DCI::DT_DOUBLE);
		CheckColumn(hTab, conEndTime, DCI::DT_DOUBLE);
		CheckColumn(hTab, conUnit, DCI::DT_STRING);
		CheckColumn(hTab, conNoOfTimePoints, DCI::DT_INT);
		CheckColumn(hTab, conDistribution, DCI::DT_STRING);
	}
	else if (TableID == conTabCurveOutputSchema)
	{
		CheckColumn(hTab, conID, DCI::DT_INT);
		CheckColumn(hTab, conPath, DCI::DT_STRING);
		CheckColumn(hTab, conStartTime, DCI::DT_DOUBLE);
		CheckColumn(hTab, conEndTime, DCI::DT_DOUBLE);
		CheckColumn(hTab, conUnit, DCI::DT_STRING);
		CheckColumn(hTab, conNoOfTimePoints, DCI::DT_INT);
		CheckColumn(hTab, conDistribution, DCI::DT_STRING);
	}
	else
		throw "Unknown TableID passed";
}

DCI::Bool SimModelComp::ProcessDataRecord(DCI::IPortHandle &port)
{
	//not supported by this component
	return false;
}

void SimModelComp::FillParameterInputTable(DCI::IPortHandle  hPort,
                                           vector<ParameterInfo> & simParams,
								           TableFillMode fillMode, 
										   QuantitiesSelectionMode selectionMode)
{
	DCI::ITableHandle hTab=hPort->GetTable();

	if(fillMode==INITIAL_FILL)
	{
		//---- 1st call - setup the table
		hTab.BindTo(new DCI::Table);
		hPort->SetTable(hTab);
		AddParameterTableColumns(hTab);
	}
	else
	{
		hTab->GetRecords()->Clear();

		if (hTab->GetColumns()->Exists(conIsVariable))
		{
			hTab->GetColumns()->Remove(conIsVariable);
			hTab->GetColumns()->Remove(conCalculateSensitivity);
		}
	}
	
	//Add table records: the table will contain <= #of params - records
	//unnecessary records will be remove at the end
	hTab->ReDim((DCI::UInt)simParams.size(), hTab->GetColumns()->GetCount());

	int LastRowIdx = 0;
	for(size_t i=0; i<simParams.size(); i++)
	{
		ParameterInfo & parameterInfo = simParams[i];

		if (!parameterInfo.CanBeVaried() && (selectionMode == VARIABLE_QUANTITIES))
			continue;

		//increment row index
		LastRowIdx++;

		hTab->SetValue(LastRowIdx, conID, parameterInfo.GetId());
		hTab->SetValue(LastRowIdx, conPath, parameterInfo.GetFullName().c_str());
		hTab->SetValue(LastRowIdx, conValue, parameterInfo.GetValue());
		hTab->SetValue(LastRowIdx, conUnit, parameterInfo.GetUnit().c_str());

		if(parameterInfo.IsTable())
			hTab->SetValue(LastRowIdx, conParameterType, conParameterTypeTable);
		else if (parameterInfo.IsFormula())
			hTab->SetValue(LastRowIdx, conParameterType, conParameterTypeFormula);
		else
			hTab->SetValue(LastRowIdx, conParameterType, conParameterTypeValue);
			
		hTab->SetValue(LastRowIdx, conFormula, parameterInfo.GetFormulaEquation().c_str());		
		hTab->SetValue(LastRowIdx, conDescription, parameterInfo.GetDescription().c_str());
	}

	//final table redim to (remove unnecessary records)
	hTab->ReDim(LastRowIdx, hTab->GetColumns()->GetCount());
}

void SimModelComp::FillTableParameterPointsInputTable(DCI::IPortHandle  hPort,
                                           vector<ParameterInfo> & simParams,
								           TableFillMode fillMode, 
										   QuantitiesSelectionMode selectionMode)
{
	DCI::ITableHandle hTab=hPort->GetTable();

	if(fillMode==INITIAL_FILL)
	{
		//---- 1st call - setup the table
		hTab.BindTo(new DCI::Table);
		hPort->SetTable(hTab);
		AddTableParameterTableColumns(hTab);
	}
	else
	{
		hTab->GetRecords()->Clear();
	}

	//---- get all table parameters and count all table points
	size_t totalNoOfPoints = 0;

	vector<ParameterInfo> tableParamInfos;

	size_t i;
	for(i=0; i<simParams.size(); i++)
	{
		ParameterInfo & parameterInfo = simParams[i];

		if (!parameterInfo.IsTable())
			continue;

		if (!parameterInfo.CanBeVaried() && (selectionMode == VARIABLE_QUANTITIES))
			continue;

		tableParamInfos.push_back(parameterInfo);
		totalNoOfPoints += parameterInfo.GetTablePoints().size();
	}

	//Add table records
	hTab->ReDim((DCI::UInt)totalNoOfPoints, hTab->GetColumns()->GetCount());

	int LastRowIdx = 0;

	for(i=0; i<tableParamInfos.size(); i++)
	{
		ParameterInfo & tableParameterInfo = tableParamInfos[i];
		const vector <ValuePoint> & valuePoints = tableParameterInfo.GetTablePoints();

		for(size_t pointIdx=0; pointIdx<valuePoints.size(); pointIdx++)
		{
			const ValuePoint & vp = valuePoints[pointIdx];
			//increment row index
			LastRowIdx++;

			hTab->SetValue(LastRowIdx, conID, tableParameterInfo.GetId());
			hTab->SetValue(LastRowIdx, conTime, vp.X);
			hTab->SetValue(LastRowIdx, conValue, vp.Y);
			hTab->SetValue(LastRowIdx, conRestartSolver, vp.RestartSolver ? (DCI::Byte)1 : (DCI::Byte)0);
		}

	}
}

void SimModelComp::FillSpeciesInputTable(DCI::IPortHandle hPort, 
                                         vector<SpeciesInfo> & simSpecies,
					                     TableFillMode fillMode, 
										 QuantitiesSelectionMode selectionMode)
{
	DCI::ITableHandle hTab=hPort->GetTable();

	if(fillMode==INITIAL_FILL)
	{
		//---- 1st call - setup the table
		hTab.BindTo(new DCI::Table);
		hPort->SetTable(hTab);
		AddSpeciesTableColumns(hTab);
	}
	else
	{
		hTab->GetRecords()->Clear();

		if (hTab->GetColumns()->Exists(conIsVariable))
			hTab->GetColumns()->Remove(conIsVariable);
	}

	//Add table records: the table will contain <= #of params - records
	//unnecessary records will be remove at the end
	hTab->ReDim((DCI::UInt)simSpecies.size(), hTab->GetColumns()->GetCount());

	int LastRowIdx = 0;
	for(size_t i=0; i<simSpecies.size(); i++)
	{
		SpeciesInfo & speciesInfo = simSpecies[i];

		//increment row index
		LastRowIdx++;

		hTab->SetValue(LastRowIdx, conID, speciesInfo.GetId());
		hTab->SetValue(LastRowIdx, conPath, speciesInfo.GetFullName().c_str());
		hTab->SetValue(LastRowIdx, conInitialValue, speciesInfo.GetValue());
		hTab->SetValue(LastRowIdx, conScaleFactor, speciesInfo.GetScaleFactor());
		hTab->SetValue(LastRowIdx, conUnit, speciesInfo.GetUnit().c_str());
		hTab->SetValue(LastRowIdx, conIsFormula, speciesInfo.IsFormula() ? (DCI::Byte)1 : (DCI::Byte)0);
		hTab->SetValue(LastRowIdx, conFormula, speciesInfo.GetFormulaEquation().c_str());		
		hTab->SetValue(LastRowIdx, conDescription, speciesInfo.GetDescription().c_str());
	}

	//final table redim to (remove unnecessary records)
	hTab->ReDim(LastRowIdx, hTab->GetColumns()->GetCount());
}

void SimModelComp::AddParameterTableColumns(const DCI::ITableHandle hTab)
{
	AddColumn(hTab, conID, DCI::DT_INT);
	AddColumn(hTab, conPath, DCI::DT_STRING);
	AddColumn(hTab, conValue, DCI::DT_DOUBLE);
	AddColumn(hTab, conUnit, DCI::DT_STRING);
	AddColumn(hTab, conParameterType, DCI::DT_STRING);
	AddColumn(hTab, conFormula, DCI::DT_STRING);
	AddColumn(hTab, conDescription, DCI::DT_STRING);
}

void SimModelComp::AddTableParameterTableColumns(const DCI::ITableHandle hTab)
{
	AddColumn(hTab, conID, DCI::DT_INT);
	AddColumn(hTab, conTime, DCI::DT_DOUBLE);
	AddColumn(hTab, conValue, DCI::DT_DOUBLE);
	AddColumn(hTab, conRestartSolver, DCI::DT_BYTE);
}

void SimModelComp::AddObserversTableColumns(const DCI::ITableHandle hTab)
{
	AddColumn(hTab, conID, DCI::DT_INT);
	AddColumn(hTab, conPath, DCI::DT_STRING);
	AddColumn(hTab, conUnit, DCI::DT_STRING);
	AddColumn(hTab, conFormula, DCI::DT_STRING);
	AddColumn(hTab, conDescription, DCI::DT_STRING);
}

void SimModelComp::AddSpeciesTableColumns(const DCI::ITableHandle hTab)
{
	AddColumn(hTab, conID, DCI::DT_INT);
	AddColumn(hTab, conPath, DCI::DT_STRING);
	AddColumn(hTab, conInitialValue, DCI::DT_DOUBLE);
	AddColumn(hTab, conScaleFactor, DCI::DT_DOUBLE);
	AddColumn(hTab, conUnit, DCI::DT_STRING);
	AddColumn(hTab, conIsFormula, DCI::DT_BYTE);
	AddColumn(hTab, conFormula, DCI::DT_STRING);
	AddColumn(hTab, conDescription, DCI::DT_STRING);
}

void SimModelComp::AddByteColumn(DCI::ITableHandle hTab, const string & columnName)
{
	AddColumn(hTab, columnName, DCI::DT_BYTE);

	for(int recIdx=1; recIdx<=hTab->GetRecords()->GetCount(); recIdx++)
		hTab->SetValue(recIdx, conIsVariable, 0);
}

OutputIntervalDistribution SimModelComp::IntervalDistributionFromString(DCI::String distribution)
{
	if (distribution == conEquidistantDistribution)
		return Equidistant;

	if (distribution == conLogarithmicDistribution)
		return Logarithmic;

	throw "Unknown interval distribution type passed: " + distribution;
}

string SimModelComp::IntervalDistributionAsString(OutputIntervalDistribution distribution)
{
	if (distribution == Equidistant)
		return conEquidistantDistribution;

	if (distribution == Logarithmic)
		return conLogarithmicDistribution;

	throw "Unknown interval distribution type passed";
}

OutputIntervalDistribution SimModelComp::IntervalDistributionFromString(const string & distribution)
{
	if (distribution == conEquidistantDistribution)
		return Equidistant;

	if (distribution == conLogarithmicDistribution)
		return Logarithmic;

	throw "Unknown interval distribution type passed:" + distribution;
}

//get table column with given name and datatype
//throw an error message if not available (and not optional)
////Fill <m_SimVariableParams>-Array (SimModel Parameters that will be varied in the current run)
vector<ParameterInfo> SimModelComp::GetVariableParameters(void)
{
	size_t i;

	//---- get and check variable parameters table
	DCI::ITableHandle hTab = GetInputPorts()->Item(conTabVariableParameter)->GetTable();
	CheckInputTableColumns(hTab, conTabVariableParameter, true);

	//get IsVariable-Values and QuiantityID-Values
	DCI::ByteVector isVariableVec = hTab->GetColumn(conIsVariable)->GetValues();
	DCI::ByteVector calcSensitivityVec = hTab->GetColumn(conCalculateSensitivity)->GetValues();
	DCI::IntVector  idVec = hTab->GetColumn(conID)->GetValues();
	DCI::StringVector  pathVec = hTab->GetColumn(conPath)->GetValues();

	//parameters that will be set variable in the simulation
	vector<ParameterInfo> SimVariableParams;

	//---- create map of all simulation parameters
	map<long, ParameterInfo> mapAllSimParams;
	for(i=0; i<m_AllParameters.size(); i++)
	{
		ParameterInfo & paramInfo = m_AllParameters[i];
		mapAllSimParams[paramInfo.GetId()] = paramInfo;
	}

	m_SensitivityParameters.clear();

	for(i=0;i<isVariableVec.Len();i++)
	{
		//---- Get variable parameters from DCI table
		//     Parameters for which sensitivity must be calculated will not be simplified in formulas,
		//     so we can set also set them to variable
		if ((isVariableVec[i] == 1) || (calcSensitivityVec[i] == 1))
		{
			map<long, ParameterInfo>::iterator iter = mapAllSimParams.find(idVec[i]);
			if (iter == mapAllSimParams.end())
				throw "Parameter " + string(pathVec[i]) + " has invalid id";

			ParameterInfo & parameterInfo = iter->second;

			if (calcSensitivityVec[i] == 1)
			{
				parameterInfo.SetCalculateSensitivity(true);
				m_SensitivityParameters.push_back(parameterInfo);
			}

			SimVariableParams.push_back(parameterInfo);
		}
	}

	return SimVariableParams;
}

vector<SpeciesInfo> SimModelComp::GetVariableSpecies(void)
{
	size_t i;

	//---- get and check variable species table
	DCI::ITableHandle hTab = GetInputPorts()->Item(conTabVariableSpecies)->GetTable();
	CheckInputTableColumns(hTab, conTabVariableSpecies, true);

	//get IsVariable-Values and QuiantityID-Values
	DCI::ByteVector isVariableVec = hTab->GetColumn(conIsVariable)->GetValues();
	DCI::IntVector  idVec = hTab->GetColumn(conID)->GetValues();
	DCI::StringVector  pathVec = hTab->GetColumn(conPath)->GetValues();

	//species that will be set variable in the simulation
	vector<SpeciesInfo> SimVariableSpecies;

	//---- create map of all simulation species
	map<long, SpeciesInfo> mapAllSimSpecies;
	for(i=0; i<m_AllSpecies.size(); i++)
	{
		SpeciesInfo & speciesInfo = m_AllSpecies[i];
		mapAllSimSpecies[speciesInfo.GetId()] = speciesInfo;
	}

	for(i=0;i<isVariableVec.Len();i++)
	{
		if (isVariableVec[i] == 1)
		{
			map<long, SpeciesInfo>::iterator iter = mapAllSimSpecies.find(idVec[i]);
			if (iter == mapAllSimSpecies.end())
				throw "Species " + string(pathVec[i]) + " has invalid id";

			SimVariableSpecies.push_back(iter->second);
		}
	}

	return SimVariableSpecies;
}


string SimModelComp::getSimulationString()
{
	string SimXMLString;

	//update parameter values, species properties, time schema etc.
	UpdateSimulationSettings();

	//get simulation string
	SimXMLString = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
		m_Sim->GetSimulationXMLString();

	return SimXMLString;
}

DCI::String SimModelComp::Invoke(const DCI::String &fncName, const DCI::String &args)
{
	DCI::String RetVal="";

	string SimXMLString;
	SimModelComp_XMLHelper HelpMe;

	try
	{
		if ((fncName == "SaveSimulationToXml"))
		{
			if(!m_ProcessMetaDataPerformed)
				throw "Cannot Invoke 'SaveSimulationToXml': 'ProcessMetaData' was not called or failed";

			//get simulation string
			SimXMLString = getSimulationString();

			string NewPath = Trim((const char*)args);

			//save simulation file
			if(NewPath=="") //Overwrite original files
			{
				HelpMe.SaveStringToFile(SimXMLString, m_SimFileName);
			}
			else
			{
				HelpMe.SaveStringToFile(SimXMLString, NewPath);
			}
		}
		else if (fncName == "GetSimulationString")
		{
			return getSimulationString().c_str();
		}
		else if (fncName == "DoEvents")
		{
#ifdef _WINDOWS
			// ================================= WINDOWS

			MSG myMSG;
			while (PeekMessage(&myMSG, 0, 0, 0, 1))
			{
				TranslateMessage(&myMSG);
				DispatchMessage(&myMSG);
			}
#endif
		}
		else if (fncName == "GetSimModelVersion")
		{
			return m_Sim->GetVersion().c_str();
		}
		else if (fncName == "GetSimModelCompVersion")
		{
			return VER_FILE_VERSION_STR;
		}
		else if (fncName == "AutoReduceTolerancesOn")
		{
			m_Sim->Options().SetAutoReduceTolerances(true);
		}
		else if (fncName == "AutoReduceTolerancesOff")
		{
			m_Sim->Options().SetAutoReduceTolerances(false);
		}
		else if (fncName == "GetSolverWarnings")
		{
			return getSolverWarnings().c_str();
		}
		else if (fncName == "GetXMLVersion")
		{
			return XMLHelper::ToString(m_Sim->GetXMLVersion()).c_str();
		}
		else if (fncName == "WriteMatlabCode")
		{
			string folder = Trim((const char*)args);
			MatlabODEExporter odeExporter;
			odeExporter.WriteMatlabCode(m_Sim, folder, false);
		}
		else if (fncName == "DisableLogFile")
		{
			m_Sim->Options().WriteLogFile(false);
		}
		else if (fncName == "EnableLogFile")
		{
			m_Sim->Options().WriteLogFile(true);
		}

		else if (fncName == "DisableBandLinearSolver")
		{
			m_Sim->SetUseBandLinearSolver(false);
		}
		else if (fncName == "EnableBandLinearSolver")
		{
			m_Sim->SetUseBandLinearSolver(true);
		}
		else if (fncName == "SetAllOutputsPersistable")
		{
			SetAllOutputsPersistable();
		}
		else
		{
			throw "Unknown function invoked: "+fncName;
		}
	}
	catch(const DCI::String & msg) {ErrorHandler(msg);RetVal=DCI::Error::GetDescription();}
	catch(const char *msg)	  {ErrorHandler(msg); }
	catch(const string & msg){ErrorHandler(msg.c_str());RetVal=DCI::Error::GetDescription();}
	catch(const ErrorData & ED) {ErrorHandler(ED);RetVal=DCI::Error::GetDescription();}
	catch(...)				  {ErrorHandler("Unknown Error in Invoke");RetVal=DCI::Error::GetDescription();}

	return RetVal;

}

void SimModelComp::SetAllOutputsPersistable()
{
	int i;

	for (i = 0; i<m_Sim->SpeciesList().size(); i++)
	{
		m_Sim->SpeciesList()[i]->SetIsPersistable(true);
	}

	for (i = 0; i<m_Sim->Observers().size(); i++)
	{
		m_Sim->Observers()[i]->SetIsPersistable(true);
	}
}

string SimModelComp::getSolverWarnings(void)
{
	string warnings = "";
	const string separator = "¦";

	for(int i=0; i<m_Sim->SolverWarnings().size(); i++)
	{
		if (i>0)
			warnings.append(separator);

		SolverWarning * warning = m_Sim->SolverWarnings()[i];
		warnings.append(warning->Message());
	}

	return warnings;
}

void SimModelComp::ErrorHandler(const char *msg)
{
	DCI::Error::SetError(NULL,DCI::EN_ERROR,msg);
}

void SimModelComp::ErrorHandler(const DCI::String &msg )
{
	DCI::Error::SetError(NULL,DCI::EN_ERROR,msg);
}

void SimModelComp::ErrorHandler(const ErrorData & ED)
{
	try
	{
		DCI::Error::SetError(NULL,DCI::EN_ERROR, ED.GetDescription().c_str());
	}
	catch(...){DCI::Error::SetError(NULL,DCI::EN_ERROR, "Couldn't set Error from SimModel ErrorData");}
}

void SimModelComp::CheckDCIError(DCI::String msg,bool ret)
{
	DCI::String ErrorMsg;
	if(!ret)
	{
		if(msg!="")
			ErrorMsg=msg+"failed ("+DCI::Error::GetDescription()+")";
		else
			ErrorMsg=DCI::Error::GetDescription();
		if(ErrorMsg=="")
			ErrorMsg="Unknown Error";
		DCI::Error::SetError(NULL,DCI::EN_ERROR,ErrorMsg);
		throw ErrorMsg;
	}
}



// -------------------- DLL INTERFACE FUNCTIONS BEGIN ------------------------------------------

// DLL Interface function for checking the DCI version

extern "C" DCI_DLLEXPORT int DCIGetVersion() {
	return DCI_VERSION;
}

// DLL Interface function for creating components

extern "C" DCI_DLLEXPORT DCI::IComponent *DCICreateComponent(const char *ComponentTypeName) {
    DCI::IComponent *pComp = new SimModelComp;
    if (pComp) pComp->AddRef();
    return pComp;
}

// -------------------- DLL INTERFACE FUNCTIONS END --------------------------------------------
