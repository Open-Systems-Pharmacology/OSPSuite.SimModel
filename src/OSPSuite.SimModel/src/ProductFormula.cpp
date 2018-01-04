#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/ProductFormula.h"
#include "SimModel/FormulaFactory.h"
#include "XMLWrapper/XMLNode.h"
#include <assert.h>

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

ProductFormula::ProductFormula ()
{
	_noOfMultipliers=0;
	_multiplierFormulas=NULL;
}

ProductFormula::~ProductFormula ()
{
	if(_multiplierFormulas)
	{
		for (int i=0; i<_noOfMultipliers; i++)
			delete _multiplierFormulas[i];
		delete[] _multiplierFormulas;
	}
	_multiplierFormulas = NULL;
}

void ProductFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// Partial XML
	//
	//  <Product>
	//		<abstractformula.../>
	//		...
	//		<abstractformula.../>
	//  <Product>

	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::Product));
	
	// ---- Read multiplier rates
	// (put into vector first, then into array to speed up calculations)

	vector<Formula *> multipliersVec;

	for (XMLNode pChildNode = pNode.GetFirstChild(); 
		 !pChildNode.IsNull();
	 	 pChildNode = pChildNode.GetNextSibling()) 
	{
		Formula * formula =  FormulaFactory::CreateFormula(pChildNode.GetNodeName());
		assert(formula!=NULL); 

		formula->LoadFromXMLNode(pChildNode);
		multipliersVec.push_back(formula);
	}

	//now cache into array
	_noOfMultipliers = (unsigned int)multipliersVec.size();
	_multiplierFormulas = new Formula * [_noOfMultipliers];

	for(int i=0; i<_noOfMultipliers; i++)
		_multiplierFormulas[i] = multipliersVec[i];

}

void ProductFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	XMLNode pChildNode;
	int iFormula;

	for (iFormula=0, pChildNode = pNode.GetFirstChild(); !pChildNode.IsNull();pChildNode = pChildNode.GetNextSibling(),iFormula++) 
		_multiplierFormulas[iFormula]->XMLFinalizeInstance(pChildNode,sim);
}

void ProductFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	for (int iFormula=0; iFormula<_noOfMultipliers; iFormula++) 
		_multiplierFormulas[iFormula] -> SetQuantityReference(quantityReference);
}

double ProductFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	double dValue = 1.;
	
	// Compute product
	for (int iFormula = 0;iFormula<_noOfMultipliers;iFormula++) 
	{
		// Multiply formulas
		dValue *= _multiplierFormulas[iFormula] -> DE_Compute(y, time, scaleFactorMode);

		// Early end if products is zero
		if (dValue == 0.) break;
	}

	// Return product value
	return dValue;
}

void ProductFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;

	// Loop formulas
	for (int iFormula = 0;iFormula<_noOfMultipliers;iFormula++)
	{
		// Product of all rates except the rate-th one
		double prod = 1.;
		for (int nr = 0; nr < _noOfMultipliers;nr++) 
		{
			if (nr != iFormula) 
			{
				prod *= _multiplierFormulas[nr] -> DE_Compute(y, time, USE_SCALEFACTOR);

				if (prod == 0.0)
					return;
			}
		}

		// Add Jacobian of current rate
		_multiplierFormulas[iFormula] -> DE_Jacobian(jacobian, y, time, iEquation, preFactor * prod);
	}
}

void ProductFormula::Finalize()
{
	for (int iFormula = 0;iFormula != _noOfMultipliers;iFormula++) 
		_multiplierFormulas[iFormula] -> Finalize();
}

void ProductFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	for (int iFormula = 0;iFormula != _noOfMultipliers;iFormula++) 
	{
		if(iFormula!=0) 
			mrOut<<"*";
		_multiplierFormulas[iFormula]->WriteMatlabCode(mrOut);
	}
}

bool ProductFormula::IsZero(void)
{
	for (int iFormula = 0;iFormula != _noOfMultipliers;iFormula++)
	{
		if(_multiplierFormulas[iFormula]->IsZero())
			return true;
	}

	return false;
}

void ProductFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	for (int iFormula = 0;iFormula != _noOfMultipliers;iFormula++)
	{
		_multiplierFormulas[iFormula]->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	}
}

void ProductFormula::UpdateIndicesOfReferencedVariables()
{
	for (int iFormula = 0;iFormula != _noOfMultipliers;iFormula++)
	{
		_multiplierFormulas[iFormula]->UpdateIndicesOfReferencedVariables();
	}
}

}//.. end "namespace SimModelNative"
