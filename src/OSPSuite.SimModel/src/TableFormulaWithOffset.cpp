#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/TableFormulaWithOffset.h"
#include "SimModel/Simulation.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/ConstantFormula.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative 
{
using namespace std;

TableFormulaWithOffset::TableFormulaWithOffset(void)
{
	_tableObjectId = INVALID_QUANTITY_ID;
	_offsetObjectId = INVALID_QUANTITY_ID;
	_tableObject = NULL;
	_offsetObject = NULL;
	_tableFormula = NULL;
}

TableFormulaWithOffset::~TableFormulaWithOffset(void)
{
}

void TableFormulaWithOffset::LoadFromXMLNode (const XMLNode & pNode)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(XMLConstants::TableWithOffsetFormula));

	//common object base part
	ObjectBase::LoadFromXMLNode(pNode);

	XMLNode pTableNode = pNode.GetChildNode(XMLConstants::TableObject);
	_tableObjectId = (long)pTableNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);

	XMLNode pOffsetNode = pNode.GetChildNode(XMLConstants::OffsetObject);
	_offsetObjectId = (long)pOffsetNode.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID);
}

void TableFormulaWithOffset::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	const char * ERROR_SOURCE = "TableFormulaWithOffset::XMLFinalizeInstance";

	ObjectBase::XMLFinalizeInstance(pNode, sim);

	_tableObject = sim->AllQuantities().GetObjectById(_tableObjectId);
	if (_tableObject == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "TableWithOffset-Formula with id="+_idAsString+" references invalid table object with id "+XMLHelper::ToString(_tableObjectId));

	long tableFormulaId = _tableObject->GetFormulaId();

	if (tableFormulaId != INVALID_QUANTITY_ID)
	{
		Formula * formula = sim->Formulas().GetObjectById(tableFormulaId);

		if (formula == NULL)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Object with id "+XMLHelper::ToString(_tableObjectId)+" references invalid formula");

		_tableFormula = dynamic_cast<TableFormula *>(formula);
		if (_tableFormula == NULL)
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "TableWithOffset-Formula with id="+_idAsString+" references nontable object with id "+XMLHelper::ToString(_tableObjectId));
	}
	
	_offsetObject = sim->AllQuantities().GetObjectById(_offsetObjectId);
	if (_offsetObject == NULL)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "TableWithOffset-Formula with id="+_idAsString+" references invalid offset object with id "+XMLHelper::ToString(_offsetObjectId));
}

bool TableFormulaWithOffset::Simplify(bool forCurrentRunOnly)
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));

	return (_tableObject->Simplify(forCurrentRunOnly) && _offsetObject->Simplify(forCurrentRunOnly));
}

string TableFormulaWithOffset::Equation()
{
	return "Table(X-Offset)";
}

double TableFormulaWithOffset::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));

	double offset = _offsetObject->GetValue(y, time, scaleFactorMode);

	return _tableObject->GetValue(y, time - offset, scaleFactorMode);
}

void TableFormulaWithOffset::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));

	if (preFactor == 0.0)
		return;

	double offset = _offsetObject->GetValue(y, time, USE_SCALEFACTOR);

	_tableObject->DE_Jacobian(jacobian, y, time - offset, iEquation, preFactor);
}

Formula* TableFormulaWithOffset::DE_Jacobian(const int iEquation)
{
	return _tableObject->DE_Jacobian(iEquation); // TODO: fix
}

Formula * TableFormulaWithOffset::clone()
{
	return new ConstantFormula(0.0);
	TableFormulaWithOffset * f = new TableFormulaWithOffset();

	// TODO: fix

	return f;
}

Formula * TableFormulaWithOffset::RecursiveSimplify()
{
	return this;
}

void TableFormulaWithOffset::SetQuantityReference (const QuantityReference & quantityReference)
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));
}

bool TableFormulaWithOffset::IsZero(void)
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));

	const bool forCurrentRunOnly = false;
	return (_tableObject->IsConstant(forCurrentRunOnly) && 
		   (_tableObject->GetValue(NULL, 0.0, USE_SCALEFACTOR) == 0));
}

bool TableFormulaWithOffset::IsRefIndependent(double & value)
{
	return false;
}

vector < HierarchicalFormulaObject * > TableFormulaWithOffset::GetUsedHierarchicalFormulaObjects()
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));

	vector<HierarchicalFormulaObject * > hfos;

	hfos.push_back((HierarchicalFormulaObject *)_tableObject);
	hfos.push_back((HierarchicalFormulaObject *)_offsetObject);

	return hfos;
}

void TableFormulaWithOffset::Finalize()
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));
}

vector <double> TableFormulaWithOffset::RestartTimePoints()
{
	assert ((_tableObject != NULL) && (_offsetObject != NULL));

	double offset;

	const bool forCurrentRunOnly = false;
	if(_offsetObject->IsConstant(forCurrentRunOnly))
	{
		offset = _offsetObject->GetValue(NULL, 0.0, USE_SCALEFACTOR);
	}
	else
	{
		offset = 0.0;
	}
	
	vector <double> tableObjectRestartTimes = _tableFormula->RestartTimePoints();

	vector <double> restartTimes;

	for(size_t i=0; i<tableObjectRestartTimes.size(); i++)
	{
		restartTimes.push_back(tableObjectRestartTimes[i] + offset);
	}

	return restartTimes;
}

void TableFormulaWithOffset::WriteFormulaMatlabCode (ostream & mrOut)
{
	const char * ERROR_SOURCE = "TableFormulaWithOffset::WriteFormulaMatlabCode";

	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Table formulas with offset not supported by matlab export" + FormulaInfoForErrorMessage()); 
}

void TableFormulaWithOffset::WriteFormulaCppCode(ostream & mrOut)
{
	const char * ERROR_SOURCE = "TableFormulaWithOffset::WriteFormulaCppCode";

	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Table formulas with offset not supported by C++ export" + FormulaInfoForErrorMessage());
}

void TableFormulaWithOffset::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	_tableObject->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	_offsetObject->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void TableFormulaWithOffset::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	_tableObject->AppendUsedParameters(usedParameterIDs);
	_offsetObject->AppendUsedParameters(usedParameterIDs);
}

void TableFormulaWithOffset::UpdateIndicesOfReferencedVariables()
{
	_tableObject->UpdateIndicesOfReferencedVariables();
	_offsetObject->UpdateIndicesOfReferencedVariables();
}

void TableFormulaWithOffset::UpdateScaleFactorOfReferencedVariable(const int quantity_id, const double ODEScaleFactor)
{
	_tableObject->UpdateScaleFactorOfReferencedVariable(quantity_id, ODEScaleFactor);
	_offsetObject->UpdateScaleFactorOfReferencedVariable(quantity_id, ODEScaleFactor);
}

}//.. end "namespace SimModelNative"