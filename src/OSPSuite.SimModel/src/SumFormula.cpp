#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/SumFormula.h"
#include "SimModel/FormulaFactory.h"
#include "XMLWrapper/XMLNode.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

SumFormula::SumFormula ()
{
	_noOfSummands=0;
	_summandFormulas=NULL;
}

SumFormula::~SumFormula ()
{
	if(_summandFormulas)
	{
		for (int i=0; i<_noOfSummands; i++)
			delete _summandFormulas[i];
		delete[] _summandFormulas;
	}
	_summandFormulas = NULL;
}

//sum formula is zero if all summands are
bool SumFormula::IsZero(void)
{
	for (int i=0; i<_noOfSummands; i++)
	{
		if(!_summandFormulas[i]->IsZero())
			return false;
	}

	return true;
}

void SumFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// Partial XML
	//
	//  <Sum>
	//		<abstractformula.../>
	//		...
	//		<abstractformula.../>
	//  <Sum>

	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Sum));
	
	// ---- Read summand rates
	// (put into vector first, then into array to speed up calculations)

	vector<Formula *> summandsVec;

	for (XMLNode pChildNode = pNode.GetFirstChild(); 
		 !pChildNode.IsNull();
	 	 pChildNode = pChildNode.GetNextSibling()) 
	{
		Formula * formula =  FormulaFactory::CreateFormula(pChildNode.GetNodeName());
		assert(formula!=NULL); 

		formula->LoadFromXMLNode(pChildNode);
		summandsVec.push_back(formula);
	}

	//now cache into array
	_noOfSummands = (unsigned int)summandsVec.size();
	_summandFormulas = new Formula * [_noOfSummands];

	for(int i=0; i<_noOfSummands; i++)
		_summandFormulas[i] = summandsVec[i];
}

void SumFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	XMLNode pChildNode;
	int iFormula;

	for (iFormula=0, pChildNode = pNode.GetFirstChild(); !pChildNode.IsNull();pChildNode = pChildNode.GetNextSibling(),iFormula++) 
		_summandFormulas[iFormula]->XMLFinalizeInstance(pChildNode,sim);
}

void SumFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	for (int iFormula=0; iFormula<_noOfSummands; iFormula++) 
		_summandFormulas[iFormula] -> SetQuantityReference(quantityReference);
}

double SumFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	double dValue = 0.;
		
	// Sum up Formulas
	for (int iFormula = 0;iFormula<_noOfSummands;iFormula++) 
		dValue += _summandFormulas[iFormula] -> DE_Compute(y, time, scaleFactorMode);
		
	// Return formula value
	return dValue;
}

void SumFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

// Loop Formulas
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++) 
		_summandFormulas[iFormula] -> DE_Jacobian(jacobian, y, time, iEquation, preFactor);
}

void SumFormula::Finalize()
{
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++) 
		_summandFormulas[iFormula] -> Finalize();
}

void SumFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++)
	{
		if(iFormula!=0) 
			mrOut<<"+";
		_summandFormulas[iFormula]->WriteMatlabCode(mrOut);
	}
}

void SumFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++)
	{
		_summandFormulas[iFormula]->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	}
}

void SumFormula::UpdateIndicesOfReferencedVariables()
{
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++)
	{
		_summandFormulas[iFormula]->UpdateIndicesOfReferencedVariables();
	}
}

}//.. end "namespace SimModelNative"