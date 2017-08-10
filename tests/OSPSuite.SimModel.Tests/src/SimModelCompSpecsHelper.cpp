#define __STRUCT__ struct
#define interface __STRUCT__

#include "SimModelSpecs/SimModelCompSpecsHelper.h"
#include "SimModelComp/SimModelComp.h"
#include "DCI/Manager.h"


namespace UnitTests
{
	using namespace DCI;
	using namespace std;

	void AddColumn(ITableHandle hTab, std::string Name, DataType datatype)
	{
		IVariableHandle hVar;
		IFieldDefHandle hFieldDef;
		IVariableHandle dummyVar;

		hVar = hTab->GetColumns()->AddNew(Name.c_str(), dummyVar);
		hFieldDef = hVar->GetFieldDef();
		hFieldDef->SetDataType(datatype);
		hFieldDef->SetName(Name.c_str());
	}

	void FillDoubleVector(IVariableHandle hVar, vector<double> & vec)
	{
		vec.clear();

		DCI::DoubleVector dcivector = hVar->GetValues();

		for(int i=0; i<dcivector.Len(); i++)
			vec.push_back(dcivector[i]);
	}

	void FillIntVector(IVariableHandle hVar, vector<int> & vec)
	{
		vec.clear();

		DCI::IntVector dcivector = hVar->GetValues();

		for(int i=0; i<dcivector.Len(); i++)
			vec.push_back(dcivector[i]);
	}

	SimModelComp * SimModelCompSpecsHelper::GetInstance()
	{
		return _simModelComp;
	}

	SimModelCompSpecsHelper::SimModelCompSpecsHelper(const char * simModelCompConfigFilePath)
	{
		//---- Create via DCI Manager (Load from config)
		DCI::IComponentHandle hComp = DCI::Manager::LoadComponentFromXMLFile(simModelCompConfigFilePath);
		hComp.GetPtr()->AddRef();

		_simModelComp=dynamic_cast<SimModelComp *> (hComp.GetPtr());		
	}

	SimModelCompSpecsHelper::~SimModelCompSpecsHelper()
	{
		delete _simModelComp;
		_simModelComp = NULL;
	}

	void SimModelCompSpecsHelper::ConfigureFrom(const char *schemaFilePath, const char *simulationFilePath)
	{
		ITableHandle configTab=_simModelComp->GetParameterPorts()->Item(1)->GetTable();
		
		configTab->SetValue(1,1,schemaFilePath);
		configTab->SetValue(1,2,simulationFilePath);

		if (!_simModelComp->Configure())
			throw std::string((const char *)DCI::Error::GetDescription());
	}

	bool SimModelCompSpecsHelper::ProcessMetaData()
	{
		return _simModelComp->ProcessMetaData();
	}

	bool SimModelCompSpecsHelper::ProcessData()
	{
		return _simModelComp->ProcessData();
	}

	std::string SimModelCompSpecsHelper::DCILastError()
	{
		string source = (const char *)DCI::Error::GetSource();
		string description = (const char *)DCI::Error::GetDescription();

		string error = "DCI Error: "+description+"\n";

		if (source != "")
			error += "in "+source+"\n";

		return error;
	}

	void SimModelCompSpecsHelper::SetAllParametersAsVariable()
	{
		ITableHandle hTab = _simModelComp->GetInputPorts()->Item(2)->GetTable();

		for(int i = 1; i<= hTab->GetRecords()->GetCount(); i++)
		{
			hTab->SetValue(i, "IsVariable", (DCI::Byte)1);
		}
	}

	void SimModelCompSpecsHelper::SetAllConstantParametersAsVariable()
	{
		ITableHandle hTab = _simModelComp->GetInputPorts()->Item(2)->GetTable();

		for(int i = 1; i<= hTab->GetRecords()->GetCount(); i++)
		{
			if ((DCI::String)hTab->GetValue(i, "ParameterType") != "Value")
				continue;

			hTab->SetValue(i, "IsVariable", (DCI::Byte)1);
		}
	}

	void SimModelCompSpecsHelper::SetParametersForCalculateSensitivity(vector<string> parameterPaths)
	{
		ITableHandle hTab = _simModelComp->GetInputPorts()->Item(2)->GetTable();

		for (int i = 1; i <= hTab->GetRecords()->GetCount(); i++)
		{
			string fullName = hTab->GetValue(i, "Path");
			if (find(parameterPaths.begin(), parameterPaths.end(), fullName) == parameterPaths.end())
				continue; //parameter path not in the list

			hTab->SetValue(i, "CalculateSensitivity", (DCI::Byte)1);
		}
	}

	vector<double> SimModelCompSpecsHelper::GetOutputTime()
	{
		vector<double> times;

		FillDoubleVector(_simModelComp->GetOutputPorts()->Item(1)->GetTable()->GetColumn("Time"), times);
		return times;
	}

	vector<double> SimModelCompSpecsHelper::GetOutputValues(const string & variableName)
	{
		vector<double> values;

		IVariableHandle hVar = _simModelComp->GetOutputPorts()->Item(2)->GetTable()->GetColumn(variableName.c_str());
		if (!hVar)
			throw string("Output variable "+ variableName+" not found");

		FillDoubleVector(hVar, values);
		return values;
	}

	vector<double> SimModelCompSpecsHelper::GetOutputSensitivities(const string & variablePath, 
		                                                           const string & parameterPath,
		                                                           const string & objectPathDelimiter)
	{
		vector<double> values;

		string Key = variablePath + objectPathDelimiter + parameterPath;

		IVariableHandle hVar = _simModelComp->GetOutputPorts()->Item(3)->GetTable()->GetColumn(Key.c_str());
		if (!hVar)
			throw string("Output variable " + Key + " not found");

		FillDoubleVector(hVar, values);

		return values;
	}

	vector<int> SimModelCompSpecsHelper::GetIds(const string & tableName)
	{
		vector<int> ids;

		IVariableHandle hVar = _simModelComp->GetInputPorts()->Item(tableName.c_str())->GetTable()->GetColumn("ID");
		if (!hVar)
			throw string("Variable 'ID' not found");

		FillIntVector(hVar, ids);

		return ids;
	}

//some code in the function below does not work as managed
//See http://social.msdn.microsoft.com/Forums/en-US/clr/thread/cd3c34a6-84f9-4e2b-a483-12e179eb84b7
#pragma managed(push, off)
	bool SimModelCompSpecsHelper::ContainsStringValue(const string & tableName,
 													  const string & columnName,
													  const string & value)
	{
		IVariableHandle hVar = _simModelComp->GetInputPorts()->Item(tableName.c_str())->GetTable()->GetColumn(columnName.c_str());
		if (!hVar)
			throw string("Variable " + columnName+ " not found");

		DCI::StringVector svec = hVar->GetValues();

		for(size_t i=0; i<svec.Len(); i++)
		{
			if (svec[i] == value.c_str()) //this line does not work as managed code
				return true;
		}

		return false;
	}
#pragma managed(pop)

	void SimModelCompSpecsHelper::SaveSimulationToXML(const string & file)
	{
		DCI::String ret = _simModelComp->Invoke("SaveSimulationToXml", file.c_str());
		if (ret != "")
			throw string((const char *)ret);
	}
}