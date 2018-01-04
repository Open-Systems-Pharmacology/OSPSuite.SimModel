#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/GlobalConstants.h"
#include <string>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

const bool IsNan (const double d) 
{
#ifdef _WINDOWS
	return _isnan(d)?true:false;
#endif

#ifdef linux
	return isnan(d);
#endif
}

const long INVALID_QUANTITY_ID = -1;
const long TIME_QUANTITY_ID = 0;

const char * csTime = "Time";
const int DE_INVALID_INDEX = - 1;

//This is the current version of SimModel. 
const double csSimModelVersion = 4.0;

//Minimal Version of XML file that can be loaded
const double csSimModelMinVersion = 4.0;

//Invalid index for HierarchicalFormulaObjects
const int HFOBECT_INVALID_INDEX = - 1;

const int HFOBJECT_TOP_LEVEL = 1;

//Return value for success from DE-Solver
const int DE_NOERROR = 0;

//old SimModel XML version (before the version attribute was introduced)
const int OLD_SIMMODEL_XML_VERSION = 2;

//---- XML constants
const string XMLConstants::SimModelSchema = "http://www.systems-biology.com";
const std::string XMLConstants::GetSchemaNamespace()
{
	return SimModelSchema;
}

const string XMLConstants::Simulation = "Simulation";
const string XMLConstants::ObjectPathDelimiter ="objectPathDelimiter";
const string XMLConstants::SimModelXMLVersion = "version";

const string XMLConstants::Id = "id";
const string XMLConstants::EntityId = "entityId";
const string XMLConstants::Name = "name";
const string XMLConstants::Description = "description";
const string XMLConstants::Path = "path";
const string XMLConstants::Persistable = "persistable";

const string XMLConstants::ParameterList = "ParameterList";
const string XMLConstants::Parameter = "P";
const string XMLConstants::FormulaId = "formulaId";
const string XMLConstants::Unit = "unit";
const string XMLConstants::CanBeVaried = "canBeVaried";
const string XMLConstants::Value = "value";
const string XMLConstants::CalculateSensitivity = "calculateSensitivity";

const string XMLConstants::VariableList = "VariableList";
const string XMLConstants::Variable = "V";
const string XMLConstants::InitialValueFormulaId = "initialValueFormulaId";
const string XMLConstants::ScaleFactor = "ScaleFactor";
const string XMLConstants::RHSFormulaList = "RHSFormulaList";
const string XMLConstants::RHSFormula = "RHSFormula";
const string XMLConstants::NegativeValuesAllowed = "negativeValuesAllowed";

const string XMLConstants::ReferenceList="ReferenceList";
const string XMLConstants::Reference="R";


const string XMLConstants::ObserverList = "ObserverList";
const string XMLConstants::Observer = "Observer";

const string XMLConstants::FormulaList = "FormulaList";
const string XMLConstants::Formula = "Formula";

const string XMLConstants::ExplicitFormula = "ExplicitFormula";
const string XMLConstants::Alias = "alias";
const string XMLConstants::Equation = "Equation";

const string XMLConstants::SwitchList = "EventList";
const string XMLConstants::Switch = "Event";
const string XMLConstants::ConditionFormulaId = "conditionFormulaId";
const string XMLConstants::ChangeList = "AssignmentList";
const string XMLConstants::Change = "Assignment";
const string XMLConstants::ObjectId = "objectId";
const string XMLConstants::NewFormulaId = "newFormulaId";
const string XMLConstants::UseAsValue = "useAsValue";
const string XMLConstants::OneTime = "oneTime";

const string XMLConstants::OutputSchema = "OutputSchema";
const string XMLConstants::OutputInterval = "OutputInterval";
const string XMLConstants::OutputIntervalList = "OutputIntervalList";

const string XMLConstants::Distribution = "distribution";
const string XMLConstants::DistributionEquidistant = "Uniform";
const string XMLConstants::DistributionLogarithmic ="Logarithmic";
const string XMLConstants::StartTime = "StartTime";
const string XMLConstants::EndTime = "EndTime";
const string XMLConstants::NumberOfTimePoints = "NumberOfTimePoints";

const string XMLConstants::OutputTime = "OutputTime";
const string XMLConstants::OutputTimeList = "OutputTimeList";

const string XMLConstants::Solver = "Solver";
const string XMLConstants::AbsTol = "AbsTol";
const string XMLConstants::RelTol = "RelTol";
const string XMLConstants::H0 = "H0";
const string XMLConstants::HMin = "HMin";
const string XMLConstants::HMax = "HMax";
const string XMLConstants::MxStep = "MxStep";
const string XMLConstants::UseJacobian = "UseJacobian";
const string XMLConstants::SolverOptionsList = "SolverOptionsList";
const string XMLConstants::SolverOption = "SolverOption";

const string XMLConstants::TableFormula="TableFormula";
const string XMLConstants::PointList="PointList";
const string XMLConstants::ValuePointX="x";
const string XMLConstants::ValuePointY="y";
const string XMLConstants::RestartSolver="restartSolver";
const string XMLConstants::UseDerivedValues="useDerivedValues";

const string XMLConstants::TableWithOffsetFormula="TableFormulaWithOffset";
const string XMLConstants::TableObject="Table";
const string XMLConstants::OffsetObject="Offset";


//---- Formula names
const string FormulaName::Constant="Constant";
const string FormulaName::Power="Power"; 
const string FormulaName::Exp="Exp"; 
const string FormulaName::Product="Product"; 
const string FormulaName::Sum="Sum"; 
const string FormulaName::Diff="Diff";
const string FormulaName::Div="Div";
const string FormulaName::Parameter="Parameter";
const string FormulaName::Variable="Variable";
const string FormulaName::SimpleProduct="SimpleProduct";
const string FormulaName::ExplicitFormula="ExplicitFormula"; 
const string FormulaName::TableFormula="TableFormula"; 
const string FormulaName::TableFormulaWithOffset="TableFormulaWithOffset"; 
const string FormulaName::Min="Min"; 
const string FormulaName::Max="Max"; 
const string FormulaName::IF="IF"; 
const string FormulaName::Equal="Equal"; 
const string FormulaName::Unequal="Unequal"; 
const string FormulaName::Less="Less"; 
const string FormulaName::LessEqual="LessEqual"; 
const string FormulaName::Greater="Greater"; 
const string FormulaName::GreaterEqual="GreaterEqual"; 
const string FormulaName::And="And"; 
const string FormulaName::Or="Or"; 
const string FormulaName::Not="Not"; 
const string FormulaName::VariableWithArgument="VariableWithArgument";
const string FormulaName::Log10="Log10";
const string FormulaName::Acos="Acos";
const string FormulaName::Asin="Asin";
const string FormulaName::Atan="Atan";
const string FormulaName::Cosh="Cosh";
const string FormulaName::Cos="Cos";
const string FormulaName::Log="Log";
const string FormulaName::Ln="Ln";
const string FormulaName::Sinh="Sinh";
const string FormulaName::Sin="Sin";
const string FormulaName::Sqrt="Sqrt";
const string FormulaName::Tanh="Tanh";
const string FormulaName::Tan="Tan";

const string FormulaConstants::FirstOperand = "FirstOperand";
const string FormulaConstants::SecondOperand = "SecondOperand"; 
const string FormulaConstants::FirstArgument = "FirstArgument";
const string FormulaConstants::SecondArgument = "SecondArgument"; 

const string FormulaConstants::IfStatement = "IfStatement"; 
const string FormulaConstants::ThenStatement = "ThenStatement"; 
const string FormulaConstants::ElseStatement = "ElseStatement"; 

const string FormulaConstants::Base = "Base"; 
const string FormulaConstants::Exponent = "Exponent"; 
const string FormulaConstants::Numerator = "Numerator"; 
const string FormulaConstants::Denominator = "Denominator"; 
const string FormulaConstants::Minuend = "Minuend"; 
const string FormulaConstants::Subtrahend = "Subtrahend"; 

}//.. end "namespace SimModelNative"
