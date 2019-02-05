#ifndef _Simulation_H_
#define _Simulation_H_

#ifdef _WINDOWS
#pragma warning(disable:4786)
#pragma warning( disable : 4279)
#endif

#include "SimModel/Species.h"
#include "SimModel/Parameter.h"
#include "SimModel/Observer.h"
#include "SimModel/Switch.h"
#include "SimModel/Formula.h"
#include "SimModel/TObjectList.h"
#include "SimModel/TObjectVector.h"
#include "XMLWrapper/XMLDocument.h"
#include "XMLWrapper/XMLNode.h"
#include "SimModel/DESolver.h"
#include "SimModel/OutputSchema.h"
#include "SimModel/SolverWarning.h"
#include "SimModel/QuantityInfo.h"
#include "SimModel/SimulationOptions.h"

#include <string>

namespace SimModelNative
{

class Simulation : 
	public ObjectBase
{
public:
	typedef std::vector <HierarchicalFormulaObject *> HierarchicalFormulaObjectVector;

private:

	enum CheckForCyclingDependenciesMode
	{
		CheckForCyclingDependencies = 1,
		DontCheckForCyclingDependencies = 2
	};

	DESolver m_Solver;
	OutputSchema _outputSchema;
	SimulationOptions _options;

	int m_ODE_NumUnknowns;
	std::vector<Species *> _DE_Variables;
	int _numberOfTimePoints;
	double * m_TimeValues;
	int m_TimeLatestIndex;

	long   _progress;
	bool _cancelFlag;

	bool _isLoaded;
	bool _isFinalized;
	std::string _objectPathDelimiter;
	XMLDocument m_XMLDoc;
	XMLNode m_SimNode;
	void ResetScalarProperties(void);
	void ResetSimulation(void);
	void LoadFromXMLDocument(void);

	//version of the SimModel-XML
	int _XML_Version;

	//cached simulation string (for saving simulation in XML)
	std::string m_XMLString;

	//all hierarchical formula objects of the simulation, 
	//saved level by level, starting with HFOBJECT_TOP_LEVEL
	std::vector <HierarchicalFormulaObjectVector> _leveledHierarchicalFormulaObjects;

	//estimate and save hierarchy level of each HFObject and 
	//arrange them according to hierarchy level in _leveledHierarchicalFormulaObjects
	void SetupHierarchicalFormulaObjects (enum CheckForCyclingDependenciesMode checkMode);

	//set species index in the diff equations system
	void DE_SetSpeciesIndex();

	//
	void FinalizeFormulas();

	// - reset formula/value state of all quantities after simulation run is finished
	//   (e.g. if changed by switches during simulation)
	// - reset state of the switches
	void ResetState();

	//
	void FinalizeSwitches();

	//for every persistable parameter an observer will be created with the same:
	// - entityId
	// - name
	// - containerpath
	// - value
	// - valueFormula
	void CreateObserversForPersistableParameters();

	void SetObserverSensitivityValues(int index, const double time, double ** sensitivityValues);

	//setup band linear solver
	void SetupBandLinearSolver();

protected:
	TObjectList<Parameter> _parameters;
	TObjectList<Species>   _species;
	TObjectList<Observer>  _observers;
	TObjectList<Switch>    _switches;
	TObjectList<Formula>   _formulas;
	TObjectList<Quantity>  _allQuantities; //union of parameters/observers/species (refs)
	TObjectList<Parameter> _sensitivityParameters; //sensitivity parameters (refs)

	TObjectVector<SolverWarning> _solverWarnings;

public:
	SIM_EXPORT Simulation(void);
	SIM_EXPORT virtual ~Simulation(void);
	SIM_EXPORT std::string GetVersion(void);

	SIM_EXPORT std::string GetObjectPathDelimiter(void) const;

	TObjectList<Parameter> & Parameters(void);
	SIM_EXPORT TObjectList<Species>   & SpeciesList(void); //unfortunately, plural von Species is Species :-)
	SIM_EXPORT TObjectList<Observer>  & Observers(void);
	TObjectList<Switch>    & Switches(void);
	TObjectList<Formula>   & Formulas(void);

	TObjectList<Quantity>  & AllQuantities(void);
	TObjectList<Parameter> & SensitivityParameters(void);

	SIM_EXPORT const TObjectVector<SolverWarning> & SolverWarnings() const;

	std::vector <HierarchicalFormulaObjectVector> GetLeveledHierarchicalFormulaObjects();

	const DESolver & GetSolver () const;

	SIM_EXPORT void Finalize();

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	SIM_EXPORT void LoadFromXMLFile   (const std::string & sFileName);
	SIM_EXPORT void LoadFromXMLString (const std::string & sSimulationXML);

	int GetODENumUnknowns ();
	double GetStartTime ();

	SIM_EXPORT OutputSchema & GetOutputSchema();

	void RedimAndInitValues (int numberOfTimePoints, double * speciesInitialValuesScaled);

	//replace formulas with values where possible
	void SimplifyObjects(bool forCurrentRunOnly);

	//Returns initial values of all DE variables (not scaled!)
	//Array must be destroyed by caller!
	double * GetDEInitialValues ();

	//Returns initial values of all DE variables SCALED with corr. ScaleFactor
	//Array must be destroyed by caller!
	double * GetDEInitialValuesScaled ();

	//execute switches at <time>, 
	// - y is DE solution at <time>, which will be UPDATED by the function
	// y must be allocated before function call
	//Returns true if at least one quantity was effectively changed by switches
	bool PerformSwitchUpdate (double * y, double time);

	//get species used in DE system by its DE index
	Species * GetDEVariableFromIndex (int DESpeciesIndex);

	SIM_EXPORT long GetProgress ();
	void SetProgress (long progress);

	SIM_EXPORT void Cancel();
	bool GetCancelFlag ();

	void AddToLog (const std::string & msg, bool PrintTime = false, bool FirstLogEntry = false);
	void AddWarning(const std::string & msg, double solverTime);

	void SetObserverValues(int index, const double * y, const double time, double ** sensitivityValues);
	void SetTimeValue (int index, double value);

	SIM_EXPORT void RunSimulation (bool & toleranceWasReduced, double & newAbsTol, double & newRelTol);

	SIM_EXPORT int GetNumberOfTimePoints ();
	SIM_EXPORT double * GetTimeValues ();

	bool IsFinalized ();

	//fill the properties of all simulation observers
	SIM_EXPORT void FillObserverProperties(std::vector<QuantityInfo> & observerProperties);

	//fill the properties of all simulation parameters
	SIM_EXPORT void FillParameterProperties(std::vector<ParameterInfo> & paramProperties);

	//set variable parameters
	SIM_EXPORT void SetVariableParameters (std::vector<ParameterInfo> & paramProperties);

	//fill the properties of all simulation DE variables
	SIM_EXPORT void FillDEVariableProperties(std::vector<SpeciesInfo> & variableProperties);

	//set variable DE Variables
	SIM_EXPORT void SetVariableDEVariables (std::vector<SpeciesInfo> & variableProperties);

	//sets parameter values into the simulation for the next run
	SIM_EXPORT void SetParametersValues(std::vector<ParameterInfo> & paramProperties);

	//sets DE variable properties into the simulation for the next run
	SIM_EXPORT void SetDEVariablesProperties (std::vector<SpeciesInfo> & variableProperties);

	//returns XML string of the simulation 
	//(with updated formulas for not fixed parameters/species initial values)
	SIM_EXPORT std::string GetSimulationXMLString ();

	//checks if simulation has any contains parameters
	SIM_EXPORT bool ContainsPersistableParameters();

	SIM_EXPORT int GetXMLVersion();

	std::vector<Species *> & DE_Variables();

	// used by Matlab and cpp exporter
	std::set<double> GetSwitchTimes();
	void MarkQuantitiesUsedBySwitches();

	SIM_EXPORT bool UseBandLinearSolver();
	SIM_EXPORT void SetUseBandLinearSolver(bool useBandLinearSolver);

	SIM_EXPORT void ReleaseMemory();

	SIM_EXPORT SimulationOptions & Options();

	SIM_EXPORT std::vector<std::string> GetPathsOfUsedParameters(void);
};

}//.. end "namespace SimModelNative"

#endif //_Simulation_H_

