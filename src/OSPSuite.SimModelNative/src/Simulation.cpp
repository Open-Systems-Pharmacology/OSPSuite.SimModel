#include "SimModel/Simulation.h"
#include "SimModel/MathHelper.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/SimModelXMLHelper.h"
#include "XMLWrapper/XMLCache.h"
#include "XMLWrapper/XMLNode.h"
#include "XMLWrapper/XMLDocument.h"
#include "XMLWrapper/XMLHelper.h"
#include <time.h>
#include "SimModel/ParameterFormula.h"
#include "SimModel/BandwidthReduction.h"
#include "../../OSPSuite.SimModelNative/version.h"
#include "SimModel/SimulationTask.h"

#ifdef _WINDOWS
#include <atlbase.h>
#endif

#ifdef _WINDOWS
#pragma warning(disable:4996)
#ifdef _DEBUG
//#include <vld.h> //uncomment to compile with Visual Leak Detector support
                   //s. https://github.com/Open-Systems-Pharmacology/OSPSuite.SimModel/wiki/Find-memory-leaks-with-Visual-Leak-Detector
#endif
#endif

namespace SimModelNative
{

using namespace std;

Simulation::Simulation(void)
{
	m_TimeValues = NULL;

	ResetScalarProperties();
}

Simulation::~Simulation(void)
{
	ReleaseMemory();
}

void Simulation::ReleaseMemory()
{
	ResetSimulation();

	if (!m_XMLDoc.IsNull())
		m_XMLDoc.Release();
}

SimulationOptions & Simulation::Options()
{
	return _options;
}

bool Simulation::UseBandLinearSolver()
{
	return m_Solver.UseBandLinearSolver();
}

void Simulation::SetUseBandLinearSolver(bool useBandLinearSolver)
{
	m_Solver.SetUseBandLinearSolver(useBandLinearSolver);
}

void Simulation::SetupBandLinearSolver()
{
	if(!UseBandLinearSolver())
		return; //nothing to do

	BandwidthReductionTask bandwidthReductionTask(this);

	bandwidthReductionTask.ReorderDEVariables();

	int lowerHalfBandWidth = bandwidthReductionTask.GetLowerHalfBandWidth();
	int upperHalfBandWidth = bandwidthReductionTask.GetUpperHalfBandWidth();

	m_Solver.SetLowerHalfBandWidth(lowerHalfBandWidth);
	m_Solver.SetUpperHalfBandWidth(upperHalfBandWidth);
}

void Simulation::Finalize ()
{
	const char * ERROR_SOURCE = "Simulation::Finalize";

	if (!_isLoaded)
		throw  ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, "Simulation was not loaded yet - cannot finalize!!");
	if (_isFinalized) 
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, "Simulation was already finalized!!");

	//cache sensitivity parameters
	for (int i = 0; i < _parameters.size(); i++)
	{
		if (_parameters[i]->CalculateSensitivity())
			_sensitivityParameters.Add(_parameters[i]);
	}

	//set hierarchy levels of dependent formula objects
	SetupHierarchicalFormulaObjects(DontCheckForCyclingDependencies);

	// - simplify formulas for: parameters, species initial values, RHS equations;
	// - identify species constant during calculations (i.e. RHS Formula is constant zero)
	//   (such species are treated as parameters with constant value, not as DEQ-variables)
	SimplifyObjects(false);

	//set hierarchy levels of dependent formula objects
	//(after simplifying, dependencies have changed because some formula objects were simplified)
	SetupHierarchicalFormulaObjects(DontCheckForCyclingDependencies);

	// Second pass: Determine equation numbers
	DE_SetSpeciesIndex();	

	//Finalize formulas
	FinalizeFormulas();

	//finalize switches
	FinalizeSwitches();

	CreateObserversForPersistableParameters();

	SimulationTask::CacheRHSUsedVariables(this);

	//Setup band linear solver. Band solver will only be used if
	// m_Solver.UseBandLinearSolver() = true
	//
	//(Default is false!)
	SetupBandLinearSolver();
	
	//Everything ok, we can allow the run 
	_isFinalized = true;
}

void Simulation::FinalizeFormulas()
{
	for(int i=0;i<_formulas.size();i++)
		_formulas[i]->Finalize();
}

void Simulation::DE_SetSpeciesIndex()
{
	// Initialize number of unknowns
	m_ODE_NumUnknowns = 0;
	_DE_Variables.clear();

	for(int i=0; i<_species.size(); i++)
	{
		Species * species = _species[i];

		if (species->IsConstantDuringCalculation())
			continue; //will not be part of DE system, just use it as parameter

		species->DE_SetSpeciesIndex(m_ODE_NumUnknowns);
		_DE_Variables.push_back(species);
	}
	assert(_DE_Variables.size()==m_ODE_NumUnknowns);
}

TObjectList<Quantity> & Simulation::AllQuantities(void)
{
	return _allQuantities;
}

TObjectList<Parameter>& Simulation::SensitivityParameters(void)
{
	return _sensitivityParameters;
}

TObjectList<Parameter> & Simulation::Parameters(void)
{
	return _parameters;
}

TObjectList<Species> & Simulation::SpeciesList(void)
{
	return _species;
}

TObjectList<Observer> & Simulation::Observers(void)
{
	return _observers;
}

TObjectList<Switch> & Simulation::Switches(void)
{
	return _switches;
}

TObjectList<Formula> & Simulation::Formulas(void)
{
	return _formulas;
}

void Simulation::LoadFromXMLNode (const XMLNode & pNode)
{
	//delete previous stuff if available
	ResetSimulation();

	//simulation attributes
	_objectPathDelimiter=pNode.GetAttribute(XMLConstants::ObjectPathDelimiter);
	_XML_Version=(int)pNode.GetAttribute(XMLConstants::SimModelXMLVersion, OLD_SIMMODEL_XML_VERSION);

	//Load parameter list
	SimModelXMLHelper<Parameter>::ObjectListLoadFromXMLNode(_parameters, pNode.GetChildNode(XMLConstants::ParameterList));

	//Load species list
	SimModelXMLHelper<Species>::ObjectListLoadFromXMLNode(_species, pNode.GetChildNode(XMLConstants::VariableList));

	//Load observer list
	SimModelXMLHelper<Observer>::ObjectListLoadFromXMLNode(_observers, pNode.GetChildNode(XMLConstants::ObserverList));

	//Load switches list
	SimModelXMLHelper<Switch>::ObjectListLoadFromXMLNode(_switches, pNode.GetChildNode(XMLConstants::SwitchList));

	//---- Load formula list
	XMLNode formulaList = pNode.GetChildNode(XMLConstants::FormulaList);

	for (XMLNode pFormulaNode = formulaList.GetFirstChild(); !pFormulaNode.IsNull();pFormulaNode = pFormulaNode.GetNextSibling())
	{
		string formulaName = pFormulaNode.GetNodeName();

		Formula * formula = FormulaFactory::CreateFormula(formulaName);
		formula->LoadFromXMLNode(pFormulaNode);

		_formulas.Add(formula);
	}

	//---- load solver
	m_Solver.LoadFromXMLNode(pNode.GetChildNode(XMLConstants::Solver));

	//---- load output schema
	_outputSchema.LoadFromXMLNode(pNode.GetChildNode(XMLConstants::OutputSchema));

	//save XML string if required
	if (_options.KeepXMLNodeAsString())
		m_XMLString = pNode.GetXML();

	_isLoaded = true;
}

void Simulation::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	//Parameter list
	SimModelXMLHelper<Parameter>::ObjectListXMLFinalizeInstance(_parameters, pNode.GetChildNode(XMLConstants::ParameterList),this);

	//Species list
	SimModelXMLHelper<Species>::ObjectListXMLFinalizeInstance(_species, pNode.GetChildNode(XMLConstants::VariableList),this);

	//Observer list
	SimModelXMLHelper<Observer>::ObjectListXMLFinalizeInstance(_observers, pNode.GetChildNode(XMLConstants::ObserverList),this);

	//Switches list
	SimModelXMLHelper<Switch>::ObjectListXMLFinalizeInstance(_switches, pNode.GetChildNode(XMLConstants::SwitchList),this);

	//Formula list
	SimModelXMLHelper<Formula>::ObjectListXMLFinalizeInstance(_formulas, pNode.GetChildNode(XMLConstants::FormulaList),this);

	//Solver
	m_Solver.XMLFinalizeInstance(pNode.GetChildNode(XMLConstants::Solver), this);

	//Output schema
	_outputSchema.XMLFinalizeInstance(pNode.GetChildNode(XMLConstants::OutputSchema), this);
}

void Simulation::ResetScalarProperties(void)
{
	_isLoaded = false;
	_isFinalized = false;
	m_ODE_NumUnknowns = 0;
	_numberOfTimePoints = 0;
	_cancelFlag = false;
	m_TimeLatestIndex = DE_INVALID_INDEX;
	m_XMLString = "";
	_XML_Version = OLD_SIMMODEL_XML_VERSION;
}

void Simulation::ResetSimulation(void)
{
	ResetScalarProperties();

	_allQuantities.FreeVector(); //just refs to parameters/species/...
	_sensitivityParameters.FreeVector(); //just refs

	_parameters.clear();
	_species.clear();
	_observers.clear();
	_switches.clear();
	_formulas.clear();

	_solverWarnings.clear();

	_leveledHierarchicalFormulaObjects.clear();

	if (m_TimeValues)
		delete[] m_TimeValues;
	m_TimeValues = NULL;

	_DE_Variables.clear();
}

#ifdef _WINDOWS
std::string DescriptionFromComError(const _com_error & e)
{
	try
	{
		LPCTSTR errMsg = e.ErrorMessage();
		return string(ATL::CT2A(errMsg));
	}
	catch (...)
	{
		return "";
	}
}
#endif

void Simulation::LoadFromXMLFile   (const string & sFileName)
{
	const char * ERROR_SOURCE = "Simulation::LoadFromXMLFile";

	try
	{
		if (!m_XMLDoc.IsNull())
			m_XMLDoc.Release();

		// Create XML DOM
		m_XMLDoc = XMLDocument::FromFile(sFileName);

		//load simulation from m_XMLDoc
		LoadFromXMLDocument();
	}
	catch(ErrorData &)
	{
		throw;
	}
	catch (std::bad_alloc& )
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
			            "Out of memory during loading from the XML file'" + sFileName + "'");
	}
#ifdef _WINDOWS
	catch (_com_error & e)
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
			            "Out of memory during loading from the XML file'" + sFileName + "' ("+
						DescriptionFromComError(e)+")");
	}
#endif
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
		                "Unknown Error occured during loading from the XML file'"+sFileName+"'");
	}
}

void Simulation::LoadFromXMLString (const string & sSimulationXML)
{
	const char * ERROR_SOURCE = "Simulation::LoadFromXMLString";

	try
	{
		if (!m_XMLDoc.IsNull())
			m_XMLDoc.Release();

		// Create XML DOM
		m_XMLDoc = XMLDocument::FromString(sSimulationXML);
		
		//load simulation from m_XMLDoc
		LoadFromXMLDocument();
	}
	catch(ErrorData &)
	{
		throw;
	}
	catch (std::bad_alloc& )
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
			"Out of memory during loading from the XML string");
	}
#ifdef _WINDOWS
	catch (_com_error & e)
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
			"Out of memory during loading from the XML string (" +
			DescriptionFromComError(e) + ")");
	}
#endif
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
		                "Unknown Error occured during loading from the XML string");
	}
}

void Simulation::LoadFromXMLDocument(void)
{
	const char * ERROR_SOURCE = "Simulation::LoadFromXMLDocument";

	assert(!m_XMLDoc.IsNull());

	if (_options.ValidateWithXMLSchema())
	{
		XMLCache* pXMLCache = XMLCache::GetInstance();

		if (!pXMLCache->SchemaInitialized())
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,"Simulation Schema File is not specified");

		XMLHelper::ValidateXMLDomWithSchema(m_XMLDoc,pXMLCache);
	}

	// Get "<Simulation>" tag
	m_SimNode = m_XMLDoc.GetRootElement();
	while (!m_SimNode.IsNull() && !m_SimNode.HasName(XMLConstants::Simulation))
		m_SimNode = m_SimNode.GetNextSibling();

	// If this didn't work for some reason...
	if (m_SimNode.IsNull())
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Unable to find node <Simulation> in the XML File");

	//---- Load simulation from current node
	LoadFromXMLNode(m_SimNode); //1st pass

	//save references to all quantities in common vector
	int i;

	for (i = 0; i < _parameters.size(); i++)
	{
		_allQuantities.Add(_parameters[i]);
	}

	for(i=0;i<_species.size();i++)
		_allQuantities.Add(_species[i]);

	for(i=0;i<_observers.size();i++)
		_allQuantities.Add(_observers[i]);

	XMLFinalizeInstance(m_SimNode, this); //2nd pass (resolve references etc.)

	SimulationTask::MarkUsedParameters(this);
}

//estimate and save hierarchy level of each HFObject and 
//arrange them according to hierarchy level in _leveledHierarchicalFormulaObjects
void Simulation::SetupHierarchicalFormulaObjects (enum CheckForCyclingDependenciesMode checkMode)
{
	TObjectVector<HierarchicalFormulaObject> SimHObjects;

	try
	{
		int i, j, simHOBjectsSize;
		HierarchicalFormulaObject * HObject;

		//---- get all HF objects
		
		for (j=0; j<_allQuantities.size(); j++)
		{
			HObject = _allQuantities[j]->GetHierarchicalFormulaObject();

			if (HObject)
				SimHObjects.push_back(HObject);
		}
		
		//cache objects size for speedup
		simHOBjectsSize = SimHObjects.size();

		//---- for each HF object:
		//       - save list of all HF objects (needed for cycling dep. check)
		//       - set  HF object index (1..#HFObjects) (dito)
		for (i=0; i<simHOBjectsSize; i++)
		{
			HObject = SimHObjects[i];
			
			//set the list of all hierarchical formula objects
			HObject->SetHierarchicalObjects(SimHObjects);
			
			//set object index in the list of all objects
			HObject->SetObjectIndex(i);
		}

		if (checkMode == CheckForCyclingDependencies)
		{
			//---- check for cycling dependencies
			for (i=0; i<simHOBjectsSize; i++)
			{
				HObject = SimHObjects[i];
				
				//check cyclic dependencies
				HObject->CheckCyclicDependencies();			
			}
		}
		
		//---- set hierarchy depth level
		bool HLevelAdjusted = true;
		while(HLevelAdjusted)
		{
			HLevelAdjusted = false;
			
			for (i=0; i<simHOBjectsSize; i++)
			{
				HObject = SimHObjects[i];
				HLevelAdjusted |= HObject->AdjustHierarchyLevel();
			}
		}

		//---- arrange and save HF objects according to their hierarchy level
		int HLevelIdx, MaxHLevel = HFOBJECT_TOP_LEVEL - 1;

		//get max. hierarchy level
		for (i = 0; i < simHOBjectsSize; i++) 
		{
			HObject = SimHObjects[i];
			
			if (HObject->GetHierarchyLevel() > MaxHLevel)
				MaxHLevel = HObject->GetHierarchyLevel();
		}

		//save HF objects in local list for later use (bottom up)
		_leveledHierarchicalFormulaObjects.clear();

		for (HLevelIdx = HFOBJECT_TOP_LEVEL; HLevelIdx <= MaxHLevel; HLevelIdx++)
		{
			vector <HierarchicalFormulaObject *> HFObjectsForLevel;

			for (i = 0; i < simHOBjectsSize; i++)
			{
				HObject = SimHObjects[i];
				
				if (HObject->GetHierarchyLevel() == HLevelIdx)
					HFObjectsForLevel.push_back(HObject);
			}

			_leveledHierarchicalFormulaObjects.push_back(HFObjectsForLevel);
		}

		SimHObjects.FreeVector();
	}
	catch(...)
	{
		SimHObjects.FreeVector();
		throw;
	}
}

void Simulation::SimplifyObjects(bool forCurrentRunOnly)
{
	unsigned int HLevelIdx, HFObjectIdx;
	vector <HierarchicalFormulaObject *> HFObjectsForLevel;
	HierarchicalFormulaObject * HObject;

	bool simplified = true;

	while(simplified)
	{
		simplified = false;

		//---- Step #1, simplify formula objects (bottom up, independent objects first)
		for (HLevelIdx=0; HLevelIdx<_leveledHierarchicalFormulaObjects.size(); HLevelIdx++)
		{
			HFObjectsForLevel = _leveledHierarchicalFormulaObjects[HLevelIdx];

			for (HFObjectIdx = 0; HFObjectIdx<HFObjectsForLevel.size(); HFObjectIdx++)
			{
				HObject = HFObjectsForLevel[HFObjectIdx];
				if (HObject->IsConstant(forCurrentRunOnly))
					continue; //is already a value

				simplified |= HObject->Simplify(forCurrentRunOnly);
			}
		}

		//---- Step #2, simplify right hand side of ODE equations
		if (forCurrentRunOnly)
			break; //species are not simplified anymore for current run 
		           //we can leave the "while simplified"-loop then

		for (int speciesIdx=0; speciesIdx<_species.size(); speciesIdx++)
		{
			simplified |= _species[speciesIdx]->SimplifyRHSList();
		}
	}

	//simplify formulas for: switch conditions; new formulas set by switches
	for(int i=0; i<_switches.size(); i++)
		_switches[i]->SimplifyFormulas(forCurrentRunOnly);

	//simplify observers
	if(!forCurrentRunOnly)
	{
		for(int i=0; i<_observers.size(); i++)
		{
			_observers[i]->Simplify(forCurrentRunOnly);
		}
	}
}

string Simulation::GetObjectPathDelimiter(void) const
{
	return _objectPathDelimiter;
}

int Simulation::GetODENumUnknowns ()
{
	// Number of Unknowns in the models 
    return m_ODE_NumUnknowns;
}

double Simulation::GetStartTime ()
{
	//by now start time is always 0
	return 0.0;
}

OutputSchema & Simulation::GetOutputSchema()
{
	return _outputSchema;
}

void Simulation::RedimAndInitValues (int numberOfTimePoints, 
									 double * speciesInitialValuesScaled, double * speciesInitialValuesUnscaled)
{
	const char * ERROR_SOURCE = "Simulation::RedimValues";

	int i, numberOfSensitivityTimePoints;

	if (numberOfTimePoints<2) //should never happen
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "At least one time step for performing simulation required");

	//set number of output time points
	_numberOfTimePoints = numberOfTimePoints;

	//---- redim time values vector
	if (m_TimeValues != NULL)
	{
		delete[] m_TimeValues;
		m_TimeValues = NULL;
	}
	
	m_TimeValues = new double[numberOfTimePoints];
	if (!m_TimeValues)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Cannot allocate memory for time values vector");

	//set initial time
	m_TimeValues[0] = GetStartTime();

	//---- redim species values vector and set their initial value
	for(i=0; i<_species.size(); i++)
	{
		Species * species = _species[i];
		numberOfSensitivityTimePoints = numberOfTimePoints;

		if (species->IsConstantDuringCalculation())
		{
			/*
			The method must receive the unscaled values - otherwise the sub-sequential "GetInitialValue" might return the scaled value of a formula.
			*/
			species->FillWithInitialValue(speciesInitialValuesUnscaled);

			//for constant species, fill with initial sensitivity value for all parameters
			numberOfSensitivityTimePoints = 1;
		}
		else
		{
			//values of non-persistable species can be ignored once the simulation is finished
			//thus redim those variables to 1 value, which will be overwritten with the latest value
			//during every ODE iteration
			species->RedimValues(species->IsPersistable() ?  numberOfTimePoints : 1);
			species->SetValue(0, speciesInitialValuesScaled[species->GetODEIndex()]);
		}

		//init and redim parameter sensitivity values
		species->InitParameterSensitivities(_sensitivityParameters, numberOfSensitivityTimePoints, species->IsPersistable());
	}

	//---- redim observer values vector and set their initial value
	for(i=0; i<_observers.size(); i++)
	{
		Observer * observer = _observers[i];
		if (!observer->IsUsedInSimulation())
			continue;

		numberOfSensitivityTimePoints = numberOfTimePoints;
		
		double initialValue = observer->CalculateValue(speciesInitialValuesScaled, GetStartTime(), USE_SCALEFACTOR);

		if (observer->IsConstantDuringCalculation())
		{
			observer->SetTheOnlyValue(initialValue);

			//for constant observer, fill with initial sensitivity value for all parameters
			numberOfSensitivityTimePoints = 1;
		}
		else
		{
			//same as for species: values of non-persistable observers are not of interest and
			//will be overwritten with the latest value during every ODE iteration
			observer->RedimValues(observer->IsPersistable() ?  numberOfTimePoints : 1);
			observer->SetValue(0, initialValue);
		}

		//init and redim parameter sensitivity values
		observer->InitParameterSensitivities(_sensitivityParameters, numberOfSensitivityTimePoints, observer->IsPersistable());
	}
}

double * Simulation::GetDEInitialValues ()
{
	const char * ERROR_SOURCE = "Simulation::GetODEInitialValues";

	double * initialvalues = NULL;
	
	try
	{
		unsigned int i,j;
		int k;

		//allocate memory (must be released by caller!)
		initialvalues = new double[m_ODE_NumUnknowns];
		if (initialvalues == NULL)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Could not reserve memory for ODE initial values");
		
		//reset initial values to NAN initially
		for(k=0; k<m_ODE_NumUnknowns; k++)
			initialvalues[k] = MathHelper::GetNaN();

		//loop through all hierarchical formula objects according to their level
		//(starting with independent objects)
		//For every HFObject, which is species and not constant during simulation:
		//  get species initial value and put it into the array
		for(i=0; i<_leveledHierarchicalFormulaObjects.size(); i++)
		{
			for(j=0; j<_leveledHierarchicalFormulaObjects[i].size(); j++)
			{
				HierarchicalFormulaObject * HFObject = _leveledHierarchicalFormulaObjects[i][j];

				Species * species = dynamic_cast<Species *>(HFObject);
				if (!species)
					continue; //not a species (parameter, etc.)

				if (species->IsConstantDuringCalculation())
					continue; //is not part of ode system

				int speciesDEIndex = species->GetODEIndex();
				assert(speciesDEIndex != DE_INVALID_INDEX);
				assert((speciesDEIndex>=0) && (speciesDEIndex<m_ODE_NumUnknowns));

				initialvalues[speciesDEIndex] = species->GetInitialValue(initialvalues, GetStartTime());
			}
		}
		
		//perform some checks of initial values
		for (k=0; k<m_ODE_NumUnknowns; k++)
		{
			//just in case - check if all initial values were set properly
			if (MathHelper::IsNaN(initialvalues[k]))
			{
				Species * species = GetDEVariableFromIndex(k);
				throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Initial value of " + species->GetFullName() + " was not set");
			}

			//check if initial value of every positive variable is >=0
			if (initialvalues[k] < 0.0)
			{
				Species * species = GetDEVariableFromIndex(k);
				if (species->NegativeValuesAllowed())
					continue;

				throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Initial value of " + species->GetFullName() + " is negative, but it was marked as positive.\nPlease check your model");
			}
		}
	}
	catch(...)
	{
		if (initialvalues)
			delete[] initialvalues;
		throw;
	}
	
	return initialvalues;
}

double * Simulation::GetDEInitialValuesScaled ()
{
	//get not scaled initial values
	double * initialValues = GetDEInitialValues(); //dynamic memory for initial values allocated inside!!
	
	for (int i=0; i<m_ODE_NumUnknowns; i++)
	{
		Species * species = GetDEVariableFromIndex(i);
		initialValues[i] /= species->GetODEScaleFactor(); 
	}
	
	return initialValues;
}

Species * Simulation::GetDEVariableFromIndex (int DESpeciesIndex)
{
	assert(DESpeciesIndex != DE_INVALID_INDEX);
	assert((DESpeciesIndex>=0) && (DESpeciesIndex<(int)_DE_Variables.size()));

	Species * species =  _DE_Variables[DESpeciesIndex];
	assert(species->GetODEIndex()==DESpeciesIndex);

	return species;
}

bool Simulation::PerformSwitchUpdate (double * y, double time)
{
	bool switchUpdate = false;

	for(int i=0; i<_switches.size(); i++)
		switchUpdate |= _switches[i]->PerformSwitchUpdate(y, time);
	
	return switchUpdate;
}

void Simulation::Cancel()
{
	_cancelFlag = true;
}

bool Simulation::GetCancelFlag ()
{
	return _cancelFlag;
}

long Simulation::GetProgress ()
{
	return _progress;
}

void Simulation::SetProgress (long progress)
{
	_progress = progress;
}

void Simulation::AddToLog (const string & msg, bool PrintTime /*= false*/, bool FirstLogEntry /*= false*/)
{
	ofstream outfile;
	
	try
	{
		string logFile = _options.LogFile();

		if (!_options.WriteLogFile() || (logFile == ""))
			return;
		
		//for the first log entry, delete evtl. available old log file
		if (FirstLogEntry)
			outfile.open(logFile.c_str(), ios::out | ios::trunc);
		else
			outfile.open(logFile.c_str(), ios::out | ios::app);
		
		if (PrintTime)
		{
			time_t ltime;
			time( &ltime );
			outfile<<ctime( &ltime );
		}
		
		outfile<<msg<<endl;
		outfile.close();
	}
	catch(...)
	{
		try
		{
			if (outfile.is_open())
				outfile.close();
		}
		catch(...){}
	}
}

void Simulation::AddWarning(const std::string & msg, double solverTime)
{
	_solverWarnings.push_back(new SolverWarning(solverTime,msg));
}

//<sensitivityValues> has dimensions [NoOf_ODE_Variables] x [NoOf_Sensitivity_Parameters]
//sensitivityValues[i] contains sensitivity values for the i-th ODE Variable
//The order of sensitivity values in sensitivityValues[i] is the same as the order of 
// sensitivity parameters stored in each ODE variable (per construction)
void Simulation::SetObserverSensitivityValues(int index, const double time, double** sensitivityValues)
{
	int observersSize = _observers.size();
	int sensitivityParametersSize = _sensitivityParameters.size();
	double * variablesSensitivityValues = NULL;  //used to store sensitivity values of all ODE variables
	                                             //  with respect to one sensitivity parameter
	double * observerSensitivityValues = NULL;   //used to store sensitivity values of observer 
	                                             //  with respect to all parameters

	if (!m_ODE_NumUnknowns || !sensitivityParametersSize)
		return; //nothing to do

	try
	{
		variablesSensitivityValues = new double[m_ODE_NumUnknowns];
		observerSensitivityValues = new double[sensitivityParametersSize];

		int observerIdx, variableIdx, parameterIdx;
		for (observerIdx = 0; observerIdx<observersSize; observerIdx++)
		{
			auto observer = _observers[observerIdx];
			if (observer->IsConstantDuringCalculation() || !observer->IsUsedInSimulation())
				continue;

			//---- set sensitivity values for all parameters
			for (parameterIdx = 0; parameterIdx < sensitivityParametersSize; parameterIdx++)
			{
				//---- fill sensitivity values for given parameter: {dy1/dp_j, ... dy_m/dp_j}
				for (variableIdx = 0; variableIdx < m_ODE_NumUnknowns; variableIdx++)
					variablesSensitivityValues[variableIdx] = sensitivityValues[variableIdx][parameterIdx];

				//---- calculate observer sensitivity assuming it's linear regarding the ODE variables
				//     TODO: for nonlinear case, observer derivative with respect to sensitivity param must be calced!!
				observerSensitivityValues[parameterIdx] = observer->CalculateValue(variablesSensitivityValues, time, USE_SCALEFACTOR);
			}
			
			observer->SetSensitivityValues(index, observerSensitivityValues);
		}

		delete[] variablesSensitivityValues;
		variablesSensitivityValues = NULL;

		delete[] observerSensitivityValues;
		observerSensitivityValues = NULL;
	}
	catch (...)
	{
		if (variablesSensitivityValues)
			delete[] variablesSensitivityValues;
		if (observerSensitivityValues)
			delete[] observerSensitivityValues;
		throw;
	}
}

void Simulation::SetObserverValues(int index, const double * y, const double time, double ** sensitivityValues)
{
	int observersSize = _observers.size(); 
	if (observersSize == 0)
		return; //nothing to do

	double * newObserverValues = NULL;

	try
	{
		//first step:  calculate values of all observers
		//second step: set values of observers
		//
		//done so, because if observer is referenced by any formula, 
		// its LATEST value will be used
		newObserverValues = new double [observersSize];

		int i;
		Observer * observer;

		for (i=0; i<observersSize; i++)
		{
			observer = _observers[i];
			if (observer->IsConstantDuringCalculation() || !observer->IsUsedInSimulation())
				continue;

			newObserverValues[i] = observer->CalculateValue(y, time, USE_SCALEFACTOR);
		}
		
		for (i=0; i<observersSize; i++)
		{
			observer = _observers[i];
			if (observer->IsConstantDuringCalculation() || !observer->IsUsedInSimulation())
				continue;

			//for non-persistable observers: overwrite the (only) value with the new one
			observer->SetValue(observer->IsPersistable() ?  index : 0, newObserverValues[i]);
		}

		SetObserverSensitivityValues(index, time, sensitivityValues);

		delete[] newObserverValues;
		newObserverValues = NULL;
	}
	catch(...)
	{
		if (newObserverValues)
			delete[] newObserverValues;
		throw;
	}
}

void Simulation::SetTimeValue (int index, double value)
{
	assert((index>=0) && (index<_numberOfTimePoints));
    m_TimeValues[index] = value;
    
    m_TimeLatestIndex = index;
}

void Simulation::RunSimulation (bool & toleranceWasReduced, double & newAbsTol, double & newRelTol)
{
	const char * ERROR_SOURCE = "Simulation::RunSimulation";

	try
	{	
		toleranceWasReduced=false;
		
		_solverWarnings.clear();

		if (!_isFinalized)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Simulation is not finalized");
		
		AddToLog("Starting simulation run...", true);
		
		_cancelFlag = false;
		_progress = 0;
		
		//simplify parameters that could not be simplified earlier (in Finalize)
		//(e.g. parameters that depend on not fixed constant parameters)
		SimplifyObjects(true);
		
		AddToLog("Params simplified, starting solving ODE...", true);
		
		//---- try to solve ODE system. If fails with convergence failure,
		//     try to reduce tolerances and resolve it. 
		// Repeat until ODE system is solved or lower bound of both tolerances reached
		while(1)
		{
			try
			{
				m_Solver.Solve_ODE();
				
				//system successfully solved
				break;
			}
			catch(SimModelSolverErrorData & SED)
			{
				//check if error cause is convergence failure or error test failure.
				//ONLY in this case: try to reduce tolerances and resolve the system
				if (_options.AutoReduceTolerances() &&
					((SED.GetNumber() == SimModelSolverErrorData::err_CONV_FAILURE) || 
					 (SED.GetNumber() == SimModelSolverErrorData::err_TEST_FAILURE)))
				{
					//try to reduce solver tolerances
					if(!m_Solver.ReduceTolerances())
						throw;

					toleranceWasReduced = true;

					//reset simulation state (parameter values changed by switches etc.)
					ResetState();

               //reset solver warnings
               _solverWarnings.clear();
				}
				else
					throw;
			}
		}

		AddToLog("ODE solved", true);
		
		//reset simulation state (parameter values changed by switches etc.)
		ResetState();
		
		_progress = 100;
		
		AddToLog("Simulation finished!", true);

		newAbsTol = m_Solver.GetSolverProperties().GetAbsTol();
		newRelTol = m_Solver.GetSolverProperties().GetRelTol();
	}
	catch(ErrorData &)
	{
		//reset simulation state (parameter values changed by switches etc.)
		ResetState();
		
		_progress = 100;
		throw;
	}
	catch(SimModelSolverErrorData & SED)
	{
		//reset simulation state (parameter values changed by switches etc.)
		ResetState();
		
	    _progress = 100;
		throw ErrorData(ErrorData::ED_ERROR, SED.GetSource(), SED.GetDescription());
	}
	catch(...)
	{
		//reset simulation state (parameter values changed by switches etc.)
		ResetState();
		
		_progress = 100;
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Unknown Error occured during solving of the ODE system");
	}
}

void Simulation::ResetState()
{
	int i;

	for(i=0; i<_allQuantities.size(); i++)
		_allQuantities[i]->ResetState();

	for(i=0; i<_switches.size(); i++)
		_switches[i]->ResetState();
}

double * Simulation::GetTimeValues ()
{
	return m_TimeValues;
}

int Simulation::GetNumberOfTimePoints ()
{
	return _numberOfTimePoints;
}

void Simulation::FinalizeSwitches()
{
	for(int i=0; i<_switches.size(); i++)
		_switches[i]->Finalize();
}

const TObjectVector<SolverWarning> & Simulation::SolverWarnings() const
{
	return _solverWarnings;
}

bool Simulation::IsFinalized ()
{
    return _isFinalized;
}

const DESolver & Simulation::GetSolver () const
{
	return m_Solver;
}

vector <Simulation::HierarchicalFormulaObjectVector> Simulation::GetLeveledHierarchicalFormulaObjects()
{
	return _leveledHierarchicalFormulaObjects;
}

void Simulation::FillObserverProperties(std::vector<QuantityInfo> & observerProperties)
{
	observerProperties.clear();

	for(int idx=0; idx<_observers.size(); idx++)
	{
		if (!_observers[idx]->IsUsedInSimulation())
			continue;

		QuantityInfo quantityInfo;
		_observers[idx]->InitialFillInfo(quantityInfo);
		observerProperties.push_back(quantityInfo);
	}
}

void Simulation::FillParameterProperties(vector<ParameterInfo> & paramProperties)
{
	const char * ERROR_SOURCE = "Simulation::FillParameterProperties";
	double * speciesInitialValues = NULL;

	try
	{
		if (!_isFinalized)
		{
			paramProperties.clear();
			for(int idx=0; idx<_parameters.size(); idx++)
			{
				ParameterInfo parameterInfo;
				_parameters[idx]->InitialFillInfo(parameterInfo);
				paramProperties.push_back(parameterInfo);
			}
		}
		else
		{
			speciesInitialValues = GetDEInitialValues();
			double simStartTime = GetStartTime();

			for(unsigned int idx=0; idx<paramProperties.size(); idx++)
			{
				ParameterInfo & parameterInfo = paramProperties[idx];
				
				Parameter * parameter = _parameters.GetObjectById(parameterInfo.GetId());
				if (!parameter)
					throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,"Cannot get parameter from id " +XMLHelper::ToString(parameterInfo.GetId()));

				parameter->FillInfo(parameterInfo, speciesInitialValues,simStartTime);
			}

			delete[] speciesInitialValues;
			speciesInitialValues = NULL;
		}
	}
	catch(ErrorData &)
	{
		if (speciesInitialValues)
			delete[] speciesInitialValues;

		throw;
	}
	catch(...)
	{
		if (speciesInitialValues)
			delete[] speciesInitialValues;

		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                "Unknown Error occured during filling parameter properties");
	}

}

void Simulation::SetVariableParameters (std::vector<ParameterInfo> & paramProperties)
{
	const char * ERROR_SOURCE = "Simulation::SetVariableParameters";

	if (_isFinalized)
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                "Cannot set parameter properties: simulation already finalized");

	for(size_t i=0; i<paramProperties.size(); i++)
	{
		ParameterInfo & paramInfo = paramProperties[i];
		Parameter * param = _parameters.GetObjectById(paramInfo.GetId());

		if (param == NULL)
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                    "Parameter " + paramInfo.GetFullName() + " has invalid id");

		param->SetIsFixed(false);
		param->SetCalculateSensitivity(paramInfo.CalculateSensitivity());
	}
}

void Simulation::FillDEVariableProperties(std::vector<SpeciesInfo> & variableProperties)
{
	const char * ERROR_SOURCE = "Simulation::FillDEVariableProperties";
	double * speciesInitialValues = NULL;

	try
	{
		if (!_isFinalized)
		{
			variableProperties.clear();
			for(int idx=0; idx<_species.size(); idx++)
			{
				SpeciesInfo speciesInfo;
				_species[idx]->InitialFillInfo(speciesInfo);
				variableProperties.push_back(speciesInfo);
			}
		}
		else
		{
			speciesInitialValues = GetDEInitialValues();
			double simStartTime = GetStartTime();

			for(unsigned int idx=0; idx<variableProperties.size(); idx++)
			{
				SpeciesInfo & speciesInfo = variableProperties[idx];
				
				Quantity * quantity = _species.GetObjectById(speciesInfo.GetId());
				if (!quantity)
					throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,"Cannot get quantity from id " +XMLHelper::ToString(speciesInfo.GetId()));

				quantity->FillInfo(speciesInfo, speciesInitialValues,simStartTime);
			}

			delete[] speciesInitialValues;
			speciesInitialValues = NULL;
		}
	}
	catch(ErrorData &)
	{
		if (speciesInitialValues)
			delete[] speciesInitialValues;

		throw;
	}
	catch(...)
	{
		if (speciesInitialValues)
			delete[] speciesInitialValues;

		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                "Unknown Error occured during filling DE variable properties");
	}
}

void Simulation::SetVariableDEVariables (std::vector<SpeciesInfo> & variableProperties)
{
	const char * ERROR_SOURCE = "Simulation::SetVariableDEVariables";

	if (_isFinalized)
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                "Cannot set DE variable properties: simulation already finalized");

	for(size_t i=0; i<variableProperties.size(); i++)
	{
		SpeciesInfo & speciesInfo = variableProperties[i];
		Species * species = _species.GetObjectById(speciesInfo.GetId());

		if (species == NULL)
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                    "DE Variable " + speciesInfo.GetFullName() + " has invalid id");

		species->SetIsFixed(false);
	}
}

void Simulation::SetParametersValues(vector<ParameterInfo> & paramProperties)
{
	const char * ERROR_SOURCE = "Simulation::SetParametersValues";

	if (!_isFinalized)
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                "Cannot set parameter values: simulation is not finalized");

	for(size_t i=0; i<paramProperties.size(); i++)
	{
		ParameterInfo & paramInfo = paramProperties[i];
		Parameter * param = _parameters.GetObjectById(paramInfo.GetId());

		if (param == NULL)
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                    "Parameter " + paramInfo.GetFullName() + " has invalid id");

		if (param->IsFixed())
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
			                "Parameter " + param->GetFullName() + " is fixed and cannot be varied");

		if (paramInfo.IsTable())
			param->SetTablePoints(paramInfo.GetTablePoints());
		else
			param->SetInitialValue(paramInfo.GetValue());
	}
}

void Simulation::SetDEVariablesProperties (vector<SpeciesInfo> & variableProperties)
{
	const char * ERROR_SOURCE = "Simulation::SetDEVariablesProperties";

	if (!_isFinalized)
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                "Cannot set DE variable properties: simulation is not finalized");

	for(size_t i=0; i<variableProperties.size(); i++)
	{
		SpeciesInfo & speciesInfo = variableProperties[i];
		Species * species = _species.GetObjectById(speciesInfo.GetId());

		if (species == NULL)
			throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE,
		                    "Species " + speciesInfo.GetFullName() + " has invalid id");

		if (species->IsFixed())
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
			                "Variable " + species->GetFullName() + " is fixed and cannot be varied");

		species->SetInitialValue(speciesInfo.GetValue());
		species->SetODEScaleFactor(speciesInfo.GetScaleFactor());
	}
}

std::string Simulation::GetSimulationXMLString ()
{
	const char * ERROR_SOURCE = "Simulation::GetSimulationXMLString";

	try
	{
		if (m_XMLString=="")
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Simulation XML String is empty");

		// Create XML DOM
		XMLDocument pXMLDoc = XMLDocument::FromString(m_XMLString);
		assert(!pXMLDoc.IsNull());

		// Get "<Simulation>" tag
		XMLNode oSimNode = pXMLDoc.GetRootElement();
		while (!oSimNode.IsNull() && !oSimNode.HasName(XMLConstants::Simulation))
			oSimNode = oSimNode.GetNextSibling();

		// If this didn't work for some reason...
		if (oSimNode.IsNull())
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Unable to find node <Simulation> in the XML string");

		XMLNode formulaListNode = oSimNode.GetChildNode(XMLConstants::FormulaList);
		XMLNode speciesListNode = oSimNode.GetChildNode(XMLConstants::VariableList);
		XMLNode parametersListNode = oSimNode.GetChildNode(XMLConstants::ParameterList);

		//Update parameter values in XML
		int idx;

		for(idx=0; idx<_parameters.size(); idx++)
			_parameters[idx]->UpdateFormulaInXMLNode(formulaListNode, parametersListNode);

      for (idx = 0; idx < _species.size(); idx++)
      {
         _species[idx]->UpdateFormulaInXMLNode(formulaListNode, speciesListNode);
         //also update the scale factor in the XML node.
         _species[idx]->UpdateScaleFactorInXMLNode(speciesListNode);
      }

		XMLNode outputSchemaNode = oSimNode.GetChildNode(XMLConstants::OutputSchema);
		
		//---- remove <OutputIntervalList> subnode from xml (if exists)
		XMLNode intervalsNode = outputSchemaNode.GetChildNode(XMLConstants::OutputIntervalList);
		if (!intervalsNode.IsNull())
			outputSchemaNode.RemoveChildNode(intervalsNode);

		//---- remove <OutputTimeList> subnode from xml (if exists)
		XMLNode pointsNode = outputSchemaNode.GetChildNode(XMLConstants::OutputTimeList);
		if (!pointsNode.IsNull())
			outputSchemaNode.RemoveChildNode(pointsNode);

		//---- add new intervals to xml
		intervalsNode = outputSchemaNode.CreateChildNode(XMLConstants::OutputIntervalList);

		for (int intervalIdx = 0; intervalIdx < _outputSchema.OutputIntervals().size(); intervalIdx++)
		{
			OutputInterval * interval = _outputSchema.OutputIntervals()[intervalIdx];

			XMLNode intervalNode = intervalsNode.CreateChildNode(XMLConstants::OutputInterval);

			intervalNode.SetAttribute(XMLConstants::Distribution,
				interval->IntervalDistribution() == OutputIntervalDistribution::Equidistant ?
				XMLConstants::DistributionEquidistant : XMLConstants::DistributionLogarithmic);

			intervalNode.CreateChildNode(XMLConstants::StartTime).SetValue(interval->StartTime());
			intervalNode.CreateChildNode(XMLConstants::EndTime).SetValue(interval->EndTime());
			intervalNode.CreateChildNode(XMLConstants::NumberOfTimePoints).SetValue(interval->NumberOfTimePoints());
		}

		m_XMLString = pXMLDoc.ToString();

		//Release Doc
		pXMLDoc.Release();

		return m_XMLString;
	}
	catch(ErrorData &)
	{
		throw;
	}
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
  	                    "Unknown Error occured during updating from the XML string");
	}
}

std::string Simulation::GetVersion ()
{
	return VER_FILE_VERSION_STR;
}

bool Simulation::ContainsPersistableParameters()
{
	for(int idx=0; idx<_parameters.size(); idx++)
	{
		Parameter * param = _parameters[idx];

		if (param->IsPersistable())
			return true;
	}

	return false;
}

void Simulation::CreateObserversForPersistableParameters()
{
	int idx;
	long objectId = _allQuantities.GetMaxObjectId();
	long formulaId = _formulas.GetMaxObjectId();

	for(idx=0; idx<_parameters.size(); idx++)
	{
		Parameter * param = _parameters[idx];

		if (!param->IsPersistable())
			continue;

		//increment formula id in order to get new unique id
		formulaId++;
		
		string formulaName = "Observer formula for persistable parameter "+param->GetName();
		ParameterFormula * formula = new ParameterFormula(formulaId,formulaName, param, "P");

		_formulas.Add(formula);

		//increment object id in order to get new unique id
		objectId++;

		Observer * obs = param->CreateObserverWithId(objectId, formula);

		_observers.Add(obs);
		_allQuantities.Add(obs);
	}
}

int Simulation::GetXMLVersion()
{
	return _XML_Version;
}

vector<Species *> & Simulation::DE_Variables()
{
	return _DE_Variables;
}

std::set<double> Simulation::GetSwitchTimes()
{
	std::set<double> switchTimes;

	switchTimes.emplace(GetStartTime());

	vector <OutputTimePoint> outputTimePoints = SimulationTask::OutputTimePoints(this);

	for (unsigned int timeStepIdx = 0; timeStepIdx<outputTimePoints.size(); timeStepIdx++)
	{
		OutputTimePoint outTimePoint = outputTimePoints[timeStepIdx];

		if (outTimePoint.IsSwitchTimePoint())
			switchTimes.emplace_hint(switchTimes.end(), outTimePoint.Time());
	}

	return switchTimes;
}

void Simulation::MarkQuantitiesUsedBySwitches()
{
	for (int i = 0; i<_switches.size(); i++)
		_switches[i]->MarkQuantitiesDirectlyUsedBy();
}

}//.. end "namespace SimModelNative"
