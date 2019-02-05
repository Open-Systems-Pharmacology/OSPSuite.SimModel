#ifdef _WINDOWS
#pragma warning(disable:4018) //signed/unsigned mismatch
#pragma warning(disable:4244) //conversion from 'long' to 'char', possible loss of data
#pragma warning(disable:4786)
#pragma warning(disable:4005) //macro redefinition
#endif
 
#ifndef __TEST_COMP_H_
#define __TEST_COMP_H_

#include "DCI/Component.h"
#include "ErrorData.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/Simulation.h"
#include "SimModel/SpeciesInfo.h"
#include "SimModel/ParameterInfo.h"

//forward declaration
class XMLCache;

class SimModelComp : public DCI::Component
{

public:
	SimModelComp();
	~SimModelComp();
	DCI::Bool   Initialize(const DCI::String &ComponentTypeName, const DCI::String &ComponentTypeDescription);

	DCI::Bool   Configure();
	DCI::Bool   ProcessMetaData();
	DCI::Bool   ProcessData();
	DCI::Bool   ProcessDataRecord(DCI::IPortHandle &port);
	DCI::Bool   CheckRunnable();
	DCI::String Invoke(const DCI::String &fncName, const DCI::String &args);

	void SetName(const DCI::String &newName);

private:
	enum TableFillMode
	{
		INITIAL_FILL,
		FURTHER_FILL
	};
	enum QuantitiesSelectionMode
	{
		ALL_QUANTITIES,
		VARIABLE_QUANTITIES
	};

	SimModelNative::Simulation * m_Sim;
	XMLCache * m_XMLSchemaCache;

	std::vector<SimModelNative::ParameterInfo> m_AllParameters;
	std::vector<SimModelNative::ParameterInfo> m_VariableParameters;
	std::vector<SimModelNative::ParameterInfo> m_SensitivityParameters;
	std::vector<SimModelNative::SpeciesInfo> m_AllSpecies;
	std::vector<SimModelNative::SpeciesInfo> m_VariableSpecies;

	bool m_IsConfigured;
	bool m_ProcessMetaDataPerformed;
	bool m_ProcessDataFirstRun;

	void LoadConfigurationFromParameterTable();
	bool GetBooleanValueFrom(DCI::ITableHandle & hTab, const std::string & columnName, bool defaultValue);
	double GetDoubleValueFrom(DCI::ITableHandle & hTab, const std::string & columnName, double defaultValue);
	std::string GetStringValueFrom(DCI::ITableHandle & hTab, const std::string & columnName, const std::string & defaultValue="");

	void FillParameterInputTable(DCI::IPortHandle hPort, 
                                 std::vector<SimModelNative::ParameterInfo> & simParams,
								 TableFillMode fillMode, QuantitiesSelectionMode selectionMode);

	void FillTableParameterPointsInputTable(DCI::IPortHandle hPort, 
                                 std::vector<SimModelNative::ParameterInfo> & simParams,
								 TableFillMode fillMode, QuantitiesSelectionMode selectionMode);

	void FillSpeciesInputTable(DCI::IPortHandle hPort, 
                               std::vector<SimModelNative::SpeciesInfo> & simSpecies,
						       TableFillMode fillMode, QuantitiesSelectionMode selectionMode);

	void FillOutputSchemaInputTable();
	void FillCurvesOutputSchemaInputTable();
	void FillObserversInputTable();

	void AddParameterTableColumns(const DCI::ITableHandle hTab);
	void AddTableParameterTableColumns(const DCI::ITableHandle hTab);
	void AddSpeciesTableColumns(const DCI::ITableHandle hTab);
	void AddOutputSchemaTableColumns(const DCI::ITableHandle hTab);
	void AddObserversTableColumns(const DCI::ITableHandle hTab);
	
	void AddByteColumn(DCI::ITableHandle hTab, const std::string & columnName);
	
	std::vector<SimModelNative::ParameterInfo> GetVariableParameters(void);
	void UpdateVariableParameters(void);

	std::vector<SimModelNative::SpeciesInfo> GetVariableSpecies(void);
	void UpdateVariableSpeciesProperties(void);
	void UpdateOutputTimeSchema(void);

	void CheckInputTableColumns(const DCI::ITableHandle hTab, unsigned int TableID,
		                        bool IsVariableColumnPresent);

	std::string IntervalDistributionAsString(SimModelNative::OutputIntervalDistribution distribution);
	SimModelNative::OutputIntervalDistribution IntervalDistributionFromString(DCI::String distribution);
	SimModelNative::OutputIntervalDistribution IntervalDistributionFromString(const std::string & distribution);

	void FillOutputTables();
	void BindNewTableTo(DCI::IPortHandle & hPort, DCI::ITableHandle & hTab, bool recordBased);

	DCI::IVariableHandle AddOutputVariable(DCI::ITableHandle & hTab,
										   const std::string & Key,
										   const long id,
										   const std::string & path,
										   const std::string & type,
										   const int timeColIdx,
										   const std::string Name);

	void AddValuesToOutputTables(DCI::ITableHandle & hTabOutputValues,
		                         DCI::ITableHandle & hTabOutputSensitivities,
		                         SimModelNative::VariableWithParameterSensitivity * pVariable,
								 const std::string TypeName,
							     bool ProcessDataFirstRun,
								 const std::string & FullPath,
								 const std::string & Name,
							     const long id);

	void AddSensitivitiesToOutputTable(DCI::ITableHandle & hTab,
									   SimModelNative::VariableWithParameterSensitivity * pVariable,
									   const std::string TypeName,
									   bool ProcessDataFirstRun,
									   const std::string & FullPath,
									   const std::string & Name,
									   const long id);

	//output schema interval distributions
	const static char * conEquidistantDistribution;
	const static char * conLogarithmicDistribution;

	//table column names
	const static char * conID;
	const static char * conPath;
	const static char * conDescription;
	const static char * conValue;
	const static char * conInitialValue;
	const static char * conUnit;
	const static char * conFormula;
	const static char * conIsFormula;
	const static char * conScaleFactor;
	const static char * conIsVariable;
	const static char * conStartTime;
	const static char * conEndTime;
	const static char * conNoOfTimePoints;
	const static char * conDistribution;
	const static char * conParameterType;
	const static char * conParameterTypeFormula;
	const static char * conParameterTypeValue;
	const static char * conParameterTypeTable;
	const static char * conTime;
	const static char * conRestartSolver;
	const static char * conCalculateSensitivity;

	//table IDs
	const static unsigned int conTabAllParameter;
	const static unsigned int conTabVariableParameter;
	const static unsigned int conTabAllSpecies;
	const static unsigned int conTabVariableSpecies;
	const static unsigned int conTabAllObserver;
	const static unsigned int conTabOutputSchema;
	const static unsigned int conTabCurveOutputSchema;
	const static unsigned int conTabAllTableParameter;
	const static unsigned int conTabVariableTableParameter;

	const static unsigned int conTabOutputTimes;
	const static unsigned int conTabOutputValues;
	const static unsigned int conTabOutputSensitivities;

	//Attributes
	const static char * conAttrID;
	const static char * conAttrPath;
	const static char * conAttrType;
	const static char * conAttrTimeColumnIndex;
	const static char * conAttrSensitivityParameterID;


	void CheckDCIError(DCI::String msg="",bool ret=true);
	void ErrorHandler(const char *msg);
	void ErrorHandler(const DCI::String &msg);
	void ErrorHandler(const ErrorData & ED);

	std::string m_SimFileName;

	std::string getSolverWarnings(void);

	void UpdateSimulationSettings();

	std::string getSimulationString();

	void SetAllOutputsPersistable();
};


#endif
