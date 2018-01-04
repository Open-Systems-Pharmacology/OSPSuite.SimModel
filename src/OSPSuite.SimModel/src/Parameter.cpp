#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/Parameter.h"
#include "SimModel/HierarchicalFormulaObject.h"
#include "SimModel/Simulation.h"
#include "SimModel/MathHelper.h"
#include "SimModel/Observer.h"
#include <SimModel/TableFormula.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

Parameter::Parameter(void)
{
	_canBeVaried = true;
	_isPersistable = false;
	_calculateSensitivity = false;
}

Parameter::~Parameter(void)
{
}

bool Parameter::CanBeVaried () const
{
	return _canBeVaried;
}

void Parameter::SetCanBeVaried(bool canBeVaried)
{
	_canBeVaried=canBeVaried;
}

bool Parameter::CalculateSensitivity() const
{
	return _calculateSensitivity;
}

void Parameter::SetCalculateSensitivity(bool calculateSensitivity)
{
	_calculateSensitivity = calculateSensitivity;
}

void Parameter::LoadFromXMLNode (const XMLNode & pNode)
{
	// ---- XML sample
	//<Parameter Id="P1" Name="Volume" Path="Liver/Cells" FormulaId="F1" Unit="µmol" canBeVaried="1" calculateSensitivity="0">

	//common quantity part
	Quantity::LoadFromXMLNode(pNode);

	//parameter specific part
	_canBeVaried = (pNode.GetAttribute(XMLConstants::CanBeVaried,_canBeVaried ? 1:0)==1);
	_calculateSensitivity = (pNode.GetAttribute(XMLConstants::CalculateSensitivity, _calculateSensitivity ? 1 : 0) == 1);
}
string Parameter::getFormulaXMLAttributeName()
{
	return XMLConstants::FormulaId;
}

void Parameter::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	//---- common quantity part
	Quantity::XMLFinalizeInstance(pNode, sim);

	//---- parameter specific part
	//     (nothing to do so far)
}

vector < HierarchicalFormulaObject * > Parameter::GetUsedHierarchicalFormulaObjects ()
{
	if (_valueFormula == NULL)
		return std::vector < HierarchicalFormulaObject * > ();

	return _valueFormula->GetUsedHierarchicalFormulaObjects();
}


double Parameter::GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode)
{
	if (_valueFormula)
	{
		return _valueFormula->DE_Compute(y, time, scaleFactorMode);
	}
	else
		return _value;
}

void Parameter::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	if (!_valueFormula)
		return; //nothing to do (constant parameter)

	_valueFormula->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

std::string Parameter::GetShortUniqueName ()
{
    return _shortUniqueName;
}

void Parameter::SetShortUniqueNameForDESystem (const std::string & uniqueName)
{
	_shortUniqueName = uniqueName;
}

void Parameter::WriteMatlabCode (std::ostream & mrOut, bool forInitialValue)
{
	if (forInitialValue && IsTable())
		return;

	if (_isChangedBySwitch || IsTable())
	{
		if (forInitialValue)
		{
			mrOut<<"if isempty("<<_shortUniqueName<<")"<<endl;
			mrOut<<"    "<<_shortUniqueName+" = ";
			if (!_valueFormula)
			{
				mrOut<<MathHelper::ToString(_value);
			}
			else
			{
				_valueFormula->WriteMatlabCode(mrOut);
			}
			mrOut<<";  % "<<GetFullName()<<std::endl;
			mrOut<<"end"<<endl;
		}
		else
		{
			mrOut<<"EvalParameter("<<_shortUniqueName<<", Time, y)";
		}
	}
	else
	{
		mrOut<<_shortUniqueName+" = ";
		if (!_valueFormula)
		{
			mrOut<<MathHelper::ToString(_value);
		}
		else
		{
			_valueFormula->WriteMatlabCode(mrOut);
		}
		mrOut<<";  % "<<GetFullName()<<std::endl;
	}
}

//will be called between Load and Finalize of the parent simulation
//must set all properties of the quantity info (name, unit, id, ...)
//The quantity formula may not be evaluated at this time point
//The only exception are constant formulas, which were already replaced
//by values in Simulation::LoadFromXXX
void Parameter::InitialFillInfo(ParameterInfo & info)
{
	//---- fill common quantity properties first
	Quantity::InitialFillInfo(info);

	//---- set parameter specific properties
	FillInfoWithParameterSpecificProperties(info);
}

//will be called after the finalize of the parent simulation
//only formula/value info must be updated
//Species initial values and sim. start time are required for the formula evaluation
void Parameter::FillInfo(ParameterInfo & info,const double * speciesInitialValues, double simulationStartTime)
{
	//---- fill common quantity properties first
	Quantity::FillInfo(info, speciesInitialValues, simulationStartTime);

	//---- set parameter specific properties
	FillInfoWithParameterSpecificProperties(info);
}

bool Parameter::IsTable(void)
{
	return _valueFormula && _valueFormula->IsTable();
}

void Parameter::FillInfoWithParameterSpecificProperties(ParameterInfo & info)
{
	info.SetCanBeVaried(_canBeVaried);
	info.SetCalculateSensitivity(_calculateSensitivity);

	if (IsTable())
		info.SetTablePoints(_valueFormula->GetTablePoints());
}

void Parameter::SetTablePoints(const std::vector <ValuePoint> & valuePoints)
{
	if(!IsTable())
	{
		//previous formula was not a table and must be created first
		DeleteFormula();

		_value = 0.0;
		_originalValue = 0.0;

		TableFormula * tableFormula = new TableFormula();
		tableFormula->SetUseDerivedValues(false); //TODO There is no information how to set UseDerivedValues-property
		                                           //Setting it to false solves the current problem of aging populations, but ...
		_valueFormula = tableFormula;

		_originalValueFormula = _valueFormula;
	}

	_valueFormula->SetTablePoints(valuePoints);
}

Observer * Parameter::CreateObserverWithId(long objectId, Formula * observerFormula)
{
	return new Observer(objectId, 
		                _name, _fullName, _description, _containerPath, _unit, 
						observerFormula, _entityId, _pathWithoutRoot);
}

bool Parameter::IsConstant(bool forCurrentRunOnly)
{
	return (Quantity::IsConstant(forCurrentRunOnly) && !_calculateSensitivity);
}

bool Parameter::ExportAsGlobalForMatlab()
{
	return (IsChangedBySwitch() || IsUsedBySwitch() || IsTable());
}

//the name of the table function which will be assigned to the parameter in case it's table-based
string Parameter::TableFunctionNameForMatlab()
{
	return GetShortUniqueName()+"_Table";
}

//writes the table function in matlab code
void Parameter::WriteTableFunctionForMatlab(ostream & mrOut)
{
	const char * ERROR_SOURCE = "Parameter::WriteTableFunctionForMatlab";

	TableFormula *tableFormula = dynamic_cast<TableFormula *>(_valueFormula);

	if(tableFormula == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, 
		                "Trying to export non-table parameter ("+_fullName+") as a table");

	mrOut<<"function yout = "+TableFunctionNameForMatlab()+"(Time, y)"<<endl<<endl;
	tableFormula->WriteMatlabCode(mrOut);
}

}//.. end "namespace SimModelNative"
