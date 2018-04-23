#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/ExplicitFormula.h"
#include "SimModel/GlobalConstants.h"
#include "FuncParser/FuncParserErrorData.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/MathHelper.h"
#include "SimModel/FormulaFactory.h"
#include "SimModel/ConstantFormula.h"
#include "SimModel/Species.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

using namespace FuncParserNative;

namespace SimModelNative
{

using namespace std;

ExplicitFormula::ExplicitFormula(void)
{
	_formula = NULL;
	_isGloballySimplified = false;
}

ExplicitFormula::~ExplicitFormula(void)
{
	_quantityRefs.clear();
	
	if (_formula != NULL)
		delete _formula;
	_formula = NULL;
}

void ExplicitFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// ---- XML sample

	//<Formula Id="F1">
	//	<ExplicitFormula>
	//		<ParameterList>
	//			<P Id="P1" Alias="P1"/>
	//		</ParameterList>
	//		<VariableList>
	//			<V Id="S1" Alias="S1"/>
	//		</VariableList>
	//		<ObserverList>
	//			<Observer Id="Obs1" Alias="Obs1"/>
	//		</ObserverList>
	//		<Equation>P1*S1</Equation>
	//	</ExplicitFormula>
	//</Formula>

	//common object base part
	ObjectBase::LoadFromXMLNode(pNode);

	//read equation string
	_equation = pNode.GetChildNodeValue(XMLConstants::Equation);
}

void ExplicitFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	ObjectBase::XMLFinalizeInstance(pNode, sim);

	//add parameter references
	AddQuantityRefsFromXMLNode(pNode.GetChildNode(XMLConstants::ParameterList), sim);

	//add species references
	AddQuantityRefsFromXMLNode(pNode.GetChildNode(XMLConstants::VariableList), sim);

	//add observer references
	AddQuantityRefsFromXMLNode(pNode.GetChildNode(XMLConstants::ObserverList), sim);

	//add other reference from <ReferenceList>
	AddQuantityRefsFromXMLNode(pNode.GetChildNode(XMLConstants::ReferenceList), sim);

	//initial formula creation
	SetupFormula();
}

void ExplicitFormula::SetupFormula()
{
	vector<string> variableNames;
	vector<string> parameterNames;
	vector<double> parameterValues;
	vector<string> parameterNotToSimplifyNames;

	for(int i =0; i<_quantityRefs.size(); i++)
	{
		parameterNames.push_back(_quantityRefs[i]->GetAlias());
		parameterValues.push_back(0.0); //just for parsing, not relevant here!
	}

	//HERE, create formula using ALL references as parameters 
	//(which will NOT be simplified)
	CreateFormulaFromEquation(variableNames, parameterNames, parameterValues, 
		                      parameterNotToSimplifyNames, false);
}

void ExplicitFormula::CreateFormulaFromEquation(const vector<string> & variableNames, 
	 										    const vector<string> & parameterNames,
											    const vector<double> & parameterValues,
	 										    const vector<string> & parameterNotToSimplifyNames,
											    bool simplifyParameter)
{
	ParsedFunction parsedFunc;
	FuncParserErrorData fpED;

	parsedFunc.SetCaseSensitive(true, fpED); 
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetLogicOperatorsAllowed(true, fpED);
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetLogicalNumericMixAllowed(true, fpED);
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetSimplifyParametersAllowed(simplifyParameter, fpED); 
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetParameterNames(parameterNames, fpED); 
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());
	
	parsedFunc.SetParameterValues(parameterValues, fpED); 
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetVariableNames(variableNames, fpED); 
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetParametersNotToSimplify(parameterNotToSimplifyNames, fpED); 
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	parsedFunc.SetStringToParse(_equation,fpED);
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	XMLNode pRateNode = GetRateNode(parsedFunc);
	
	if(_formula)
	{
		delete _formula;
		_formula = NULL;
	}

	_formula = FormulaFactory::CreateFormula(pRateNode.GetNodeName());

	_formula->LoadFromXMLNode(pRateNode);

	//---- set quantity references into parameter rates
	for(int i = 0;i<_quantityRefs.size();i++)
		_formula->SetQuantityReference(*_quantityRefs[i]);

	//XML Finalize embedded formula
	// (pass NULL as parent simulation, because embedded formula don't need 
	//  any further info from it. If this changes, either cache simulation in
	//  explicit formula or pass it as argument to Formula->Finalize()
	_formula->XMLFinalizeInstance(pRateNode, NULL);

	//release memory
	pRateNode.FreeNode();

}


XMLNode ExplicitFormula::GetRateNode(ParsedFunction & parsedFunction)
{
	const char * ERROR_SOURCE = "ExplicitFormula::GetRateNode";

	FuncParserErrorData fpED;

	//Get Parsed XML Node
	std::string sXMLString = parsedFunction.GetXMLString(fpED,true,"ROOT");
	if (fpED.GetNumber() != FuncParserErrorData::err_OK) throw ErrorData(ErrorData::ED_ERROR, fpED.GetSource(), fpED.GetDescription() + FormulaInfoForErrorMessage());

	//Create DOM document
	try
	{
		XMLDocument pXMLDoc = XMLDocument::FromString(sXMLString);

		// Get "<Rate>" tag
		XMLNode pRootNode = pXMLDoc.GetRootElement();
		while (!pRootNode.IsNull() && !pRootNode.HasName("ROOT"))
			pRootNode = pRootNode.GetNextSibling();

		// If this didn't work for some reason...
		if (pRootNode.IsNull())
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
 						    "Failed to find node <ROOT> in XML String of expression " + parsedFunction.GetStringToParse(fpED));

		//create and return deep copy of the rate node
		XMLNode rateNode = pRootNode.GetFirstChild().Clone(true);

		pXMLDoc.Release();

		//Return first child of the node
		return rateNode;
	}
	catch(ErrorData &)
	{
		throw;
	}
	catch(...)
	{
		throw ErrorData(ErrorData::ED_ERROR,ERROR_SOURCE, "Unknown Error occured during loading from the XML string");
	}
}

void ExplicitFormula::AddQuantityRefsFromXMLNode(XMLNode refListNode, Simulation * sim)
{
	if (refListNode.IsNull())
		return; //nothing to do
	
	const string formulaInfo = "Formula id="+_idAsString;

	for (XMLNode pChild = refListNode.GetFirstChild(); !pChild.IsNull();pChild = pChild.GetNextSibling()) 
	{
		QuantityReference * quantityRef = new QuantityReference();
		quantityRef->SetParentFormulaInfo(formulaInfo);

		quantityRef->LoadFromXMLNode(pChild);
		quantityRef->XMLFinalizeInstance(pChild, sim);
	
		_quantityRefs.push_back(quantityRef);
	}
}

bool ExplicitFormula::Simplify(bool forCurrentRunOnly)
{
	if (dynamic_cast<ConstantFormula *>(_formula) != NULL)
		return true; //constant formula, ergo simplified

	bool Simplified = true;

	//try to simplify all used objects
	for (int i=0; i<_quantityRefs.size(); i++)
	{
		QuantityReference * quantityRef = _quantityRefs[i];

		if(quantityRef->IsConstant(forCurrentRunOnly))
			continue; //already simplified

		Simplified &= quantityRef->SimplifyQuantity(forCurrentRunOnly);
	}

	if(!forCurrentRunOnly)
	{
		//---- check if formula is constant or is zero. 
		// If so, replace with constant formula

		double value=0.0;

		if(Simplified)
		{
			value = DE_Compute(NULL, 0.0, USE_SCALEFACTOR);
		}
		else if (IsZero())
		{
			value = 0.0;
			Simplified = true;
		}

		if(Simplified)
		{
			//replace with constant formula
			delete(_formula);
			_formula=new ConstantFormula(value);

			//mark as simplified to avoid creating during finalize
			_isGloballySimplified = true;
		}
	}

	return Simplified;
}

bool ExplicitFormula::IsZero(void)
{
	return _formula->IsZero();
}

vector < HierarchicalFormulaObject * > ExplicitFormula::GetUsedHierarchicalFormulaObjects()
{
	vector<HierarchicalFormulaObject *> HFObjects;

	//return all quantities used in the formula, which are hierarchical formula objects
	for(int i=0; i<_quantityRefs.size(); i++)
	{
		HierarchicalFormulaObject * HFObject = _quantityRefs[i]->GetHierarchicalFormulaObject();

		if (HFObject != NULL)
			HFObjects.push_back(HFObject);
	}

	return HFObjects;
}

double ExplicitFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	return _formula->DE_Compute(y, time, scaleFactorMode);
}

void ExplicitFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	_formula->DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

Formula* ExplicitFormula::DE_Jacobian(const int iEquation)
{
	return _formula->DE_Jacobian(iEquation);
}

Formula* ExplicitFormula::clone()
{
	ExplicitFormula* f = new ExplicitFormula();
	f->_formula = _formula->clone();
	return f;
}

Formula * ExplicitFormula::RecursiveSimplify()
{
	return _formula->RecursiveSimplify();
	//throw ErrorData(ErrorData::ED_ERROR, "ExplicitFormula::RecusiveSimplify", "This method should not be called.");
}

void ExplicitFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	_formula->SetQuantityReference(quantityReference);
}

bool ExplicitFormula::IsRefIndependent(double & value)
{
	//---- check if the formula is a simple scalar (e.g. "2.3")
	if (MathHelper::IsNumeric(_equation))
	{
		value = XMLHelper::ToDouble(_equation);
		return true;
	}

	return false;
}

void ExplicitFormula::Finalize()
{
	if (_isGloballySimplified)
		return; //nothing to do

	vector<string> variableNames;
	vector<string> parameterNames;
	vector<string> parameterNotToSimplifyNames;
	vector<double> parameterValues;
	
	for(int i =0; i<_quantityRefs.size(); i++)
	{
		QuantityReference * quantityRef = _quantityRefs[i];

		bool isDEVariable = (quantityRef->IsSpecies() && 
			                !quantityRef->GetSpecies()->IsConstantDuringCalculation());

		if (isDEVariable)
		{
			//DE Variable
			variableNames.push_back(quantityRef->GetAlias());
		}
		else
		{
			//parameter, observer, 
			//species which is CONSTANT DURING SIMULATION (treated like parameter!)
			parameterNames.push_back(quantityRef->GetAlias());

			if (quantityRef->IsConstant(false))
			{
				//constant - pass value to the parser
				parameterValues.push_back(quantityRef->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR));
			}
			else
			{
				//not constant 
				//Mark as not to be simplified and pass any value to the parser,
				//(because it's not relevant - <Parameter>-Rate will be created)
				parameterNotToSimplifyNames.push_back(quantityRef->GetAlias());
				parameterValues.push_back(0.0);
			}
		}
	}

	//Now, create the final formula  
	//Parameters will be simplified now (of course except those in parameterNotToSimplifyNames)
	CreateFormulaFromEquation(variableNames, parameterNames, parameterValues, 
		                      parameterNotToSimplifyNames, true);

}

void ExplicitFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	_formula->WriteMatlabCode(mrOut);
}

void ExplicitFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	_formula->WriteCppCode(mrOut);
}

//don't use brackets (embedded formula will be bracketed)
bool ExplicitFormula::UseBracketsForODESystemGeneration ()
{
	return false;
}

vector <double> ExplicitFormula::SwitchTimePoints()
{
	return _formula->SwitchTimePoints();
}

bool ExplicitFormula::IsConstant(bool forCurrentRunOnly)
{
//	return (dynamic_cast<ConstantFormula *>(_formula) != NULL);
	return _formula->IsConstant(forCurrentRunOnly);
}

std::string ExplicitFormula::Equation()
{
	return _equation;
}

void ExplicitFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	_formula->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
}

void ExplicitFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	_formula->AppendUsedParameters(usedParameterIDs);
}

void ExplicitFormula::UpdateIndicesOfReferencedVariables()
{
	if (_formula != NULL)
		_formula->UpdateIndicesOfReferencedVariables();
}

}//.. end "namespace SimModelNative"
