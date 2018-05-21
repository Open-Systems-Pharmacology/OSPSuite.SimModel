#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/TableFormulaWithXArgument.h"
#include "SimModel/Simulation.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/ConstantFormula.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative 
{
using namespace std;

TableFormulaWithXArgument::TableFormulaWithXArgument(void)
{
	_tableObjectId = INVALID_QUANTITY_ID;
	_XArgumentObjectId = INVALID_QUANTITY_ID;
	_tableObject = NULL;
	_XArgumentObject = NULL;
	_tableFormula = NULL;
}

TableFormulaWithXArgument::~TableFormulaWithXArgument(void)
{
}

void TableFormulaWithXArgument::LoadFromXMLNode (const XMLNode & pNode)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(XMLConstants::TableWithXArgumentFormula));

	//common object base part
	ObjectBase::LoadFromXMLNode(pNode);

	XMLNode pTableNode = pNode.GetChildNode(XMLConstants::TableObject);
	_tableObjectId = (long)pTableNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);

	XMLNode pXArgumentNode = pNode.GetChildNode(XMLConstants::XArgumentObject);
	_XArgumentObjectId = (long)pXArgumentNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);
}

void TableFormulaWithXArgument::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	const char * ERROR_SOURCE = "TableFormulaWithXArgument::XMLFinalizeInstance";

	ObjectBase::XMLFinalizeInstance(pNode, sim);

	_tableObject = sim->AllQuantities().GetObjectById(_tableObjectId);
	if (_tableObject == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "TableWithXArgument-Formula with id="+_idAsString+" references invalid table object with id "+XMLHelper::ToString(_tableObjectId));

	long tableFormulaId = _tableObject->GetFormulaId();

	if (tableFormulaId != INVALID_QUANTITY_ID)
	{
		Formula * formula = sim->Formulas().GetObjectById(tableFormulaId);

		if (formula == NULL)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Object with id "+XMLHelper::ToString(_tableObjectId)+" references invalid formula");

		_tableFormula = dynamic_cast<TableFormula *>(formula);
		if (_tableFormula == NULL)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "TableWithXArgument-Formula with id="+_idAsString+" references nontable object with id "+XMLHelper::ToString(_tableObjectId));
	}
	
	_XArgumentObject = sim->AllQuantities().GetObjectById(_XArgumentObjectId);
	if (_XArgumentObject == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "TableWithXArgument-Formula with id="+_idAsString+" references invalid X argument object with id "+XMLHelper::ToString(_XArgumentObjectId));
}

bool TableFormulaWithXArgument::Simplify(bool forCurrentRunOnly)
{
	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));

	_tableObject->Simplify(forCurrentRunOnly);

	//currently table object is defined by (x,y) data points and is NOT dependent from time or
	//any parameters/variables
	//Thus if the X argument object can be simplified (will be constant): 
	//table formula with X argument can be simplified too
	if (_XArgumentObject->IsConstant(forCurrentRunOnly))
		return true;

	return  _XArgumentObject->Simplify(forCurrentRunOnly);
}

string TableFormulaWithXArgument::Equation()
{
	return "Table(X-Argument)"; //no explicit equation available
}

double TableFormulaWithXArgument::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));

	double argument = _XArgumentObject->GetValue(y, time, scaleFactorMode);

	//table formula always interprets the time passed to TableFormula::DE_Compute() as X argument
	//thus here, just pass <argument> as a time parameter to _tableObject->GetValue
	return _tableObject->GetValue(y, argument, scaleFactorMode);
}

void TableFormulaWithXArgument::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	const char * ERROR_SOURCE = "TableFormulaWithXArgument::DE_Jacobian";

	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));

	if (preFactor == 0.0)
		return;

	bool forCurrentRunOnly = true;
	if (_XArgumentObject->IsConstant(forCurrentRunOnly))
		return;

	//TODO if X-argument is time dependent but not ODE-variables-dependent, 
	//     everything is OK
	// If it's ODE-variables-dependent: either an exception must be thrown or
	// the Jacobian-Calculation must take it into account

	//throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Table formula with X argument does not support time dependent argument objects");
}

Formula* TableFormulaWithXArgument::DE_Jacobian(const int iEquation)
{
	return _tableObject->DE_Jacobian(iEquation); // TODO: fix
}

Formula * TableFormulaWithXArgument::clone()
{
	return new ConstantFormula(0.0);
	TableFormulaWithXArgument * f = new TableFormulaWithXArgument();

	// TODO: fix

	return f;
}

Formula * TableFormulaWithXArgument::RecursiveSimplify()
{
	return this;
}

void TableFormulaWithXArgument::SetQuantityReference (const QuantityReference & quantityReference)
{
	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));
}

bool TableFormulaWithXArgument::IsZero(void)
{
	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));

	const bool forCurrentRunOnly = false;

	//case 1: table formula is constant (all y values are the same)
	//        If so: calculate table formula value for ANY argument (e.g. zero) and check if it returns 0
	if (_tableObject->IsConstant(forCurrentRunOnly))
		return (_tableObject->GetValue(NULL, 0.0, USE_SCALEFACTOR) == 0);

	//case 2: X argument is constant. Check if table formula for this const argument value is zero
	if (_XArgumentObject->IsConstant(forCurrentRunOnly))
	{
		double argument = _XArgumentObject->GetValue(NULL, 0.0, USE_SCALEFACTOR);
		return (_tableObject->GetValue(NULL, argument, USE_SCALEFACTOR) == 0);
	}

	//neither x argument nor table values are constant => formula is not const zero
	return false;
}

bool TableFormulaWithXArgument::IsRefIndependent(double & value)
{
	return false;
}

vector < HierarchicalFormulaObject * > TableFormulaWithXArgument::GetUsedHierarchicalFormulaObjects()
{
	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));

	vector<HierarchicalFormulaObject * > hfos;

	hfos.push_back((HierarchicalFormulaObject *)_tableObject);
	hfos.push_back((HierarchicalFormulaObject *)_XArgumentObject);

	return hfos;
}

void TableFormulaWithXArgument::Finalize()
{
	assert ((_tableObject != NULL) && (_XArgumentObject != NULL));
}

vector <double> TableFormulaWithXArgument::RestartTimePoints()
{
	assert((_tableObject != NULL) && (_XArgumentObject != NULL));

	vector <double> restartTimes;
	
	const bool forCurrentRunOnly = false;
	if (_XArgumentObject->IsConstant(forCurrentRunOnly))
		return restartTimes; //argument const=>formula will always be const=>no need to restart

	//TODO fix non-constant argument
	return restartTimes;
}

void TableFormulaWithXArgument::WriteFormulaMatlabCode (ostream & mrOut)
{
	const char * ERROR_SOURCE = "TableFormulaWithXArgument::WriteFormulaMatlabCode";

	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Table formulas with offset not supported by matlab export" + FormulaInfoForErrorMessage()); 
}

void TableFormulaWithXArgument::WriteFormulaCppCode(ostream & mrOut)
{
	const char * ERROR_SOURCE = "TableFormulaWithXArgument::WriteFormulaCppCode";

	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Table formulas with offset not supported by C++ export" + FormulaInfoForErrorMessage());
}

void TableFormulaWithXArgument::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	_tableObject->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	_XArgumentObject->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void TableFormulaWithXArgument::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	_tableObject->AppendUsedParameters(usedParameterIDs);
	_XArgumentObject->AppendUsedParameters(usedParameterIDs);
}

void TableFormulaWithXArgument::UpdateIndicesOfReferencedVariables()
{
	_tableObject->UpdateIndicesOfReferencedVariables();
	_XArgumentObject->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"