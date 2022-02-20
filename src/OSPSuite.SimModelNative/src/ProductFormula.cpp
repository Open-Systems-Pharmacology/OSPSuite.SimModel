#include "SimModel/ProductFormula.h"
#include "SimModel/FormulaFactory.h"
#include "XMLWrapper/XMLNode.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/SumFormula.h"
#include "SimModel/ConstantFormula.h"
#include <assert.h>

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
               break;
			}
		}

      if (prod == 0.0)
         continue; //no contribution to jacobian from the current multiplier

		// Add Jacobian of current rate
		_multiplierFormulas[iFormula] -> DE_Jacobian(jacobian, y, time, iEquation, preFactor * prod);
	}
}

Formula* ProductFormula::DE_Jacobian(const int iEquation)
{
	SumFormula * s = new SumFormula();

	Formula * * mult = new Formula*[_noOfMultipliers];
	Formula * * sum  = new Formula*[_noOfMultipliers];

	for (int iFormula = 0; iFormula < _noOfMultipliers; iFormula++)
	{
		ProductFormula *p = new ProductFormula();

		for (int nr = 0; nr < _noOfMultipliers; nr++)
		{
			if (nr != iFormula)
				mult[nr] = _multiplierFormulas[nr]->clone();
			else
				mult[nr] = _multiplierFormulas[nr]->DE_Jacobian(iEquation);
		}
		p->setFormula(_noOfMultipliers, mult);

		sum[iFormula] = p;
	}

	s->setFormula(_noOfMultipliers, sum);

   delete[] mult;
   delete[] sum;

	return s;
}

Formula * ProductFormula::clone()
{
	ProductFormula * f = new ProductFormula();
	f->_noOfMultipliers = _noOfMultipliers;
	f->_multiplierFormulas = new Formula*[_noOfMultipliers];
	for (int iFormula = 0; iFormula < _noOfMultipliers; iFormula++)
		f->_multiplierFormulas[iFormula] = _multiplierFormulas[iFormula]->clone();
	return f;
}

Formula * ProductFormula::RecursiveSimplify()
{
	bool isConstant = true;
	int ones = 0;
	for (int iFormula = 0; iFormula < _noOfMultipliers; iFormula++) {
		_multiplierFormulas[iFormula] = _multiplierFormulas[iFormula]->RecursiveSimplify();
		if (_multiplierFormulas[iFormula]->IsConstant(CONSTANT_CURRENT_RUN))
		{
			if (_multiplierFormulas[iFormula]->IsZero())
			{
				ConstantFormula * f = new ConstantFormula(0.0);
				delete this;
				return f;
			}
			else if (_multiplierFormulas[iFormula]->DE_Compute(NULL, 0.0, USE_SCALEFACTOR)==1.0)
			{
				++ones;
			}
		}
		else
			isConstant = false;
	}

	if (isConstant)
	{
		ConstantFormula * f = new ConstantFormula(DE_Compute(NULL, 0.0, USE_SCALEFACTOR));
		delete this;
		return f;
	}
	else if (ones) {
		int newIndex = 0;

		if (_noOfMultipliers - ones == 1) {
			for (int iFormula = 0; iFormula < _noOfMultipliers; iFormula++)
				if ( !_multiplierFormulas[iFormula]->IsConstant(CONSTANT_CURRENT_RUN) ||
					  _multiplierFormulas[iFormula]->DE_Compute(NULL, 0.0, USE_SCALEFACTOR) != 1.0)
					newIndex = iFormula;

			Formula * f = _multiplierFormulas[newIndex];
			_multiplierFormulas[newIndex] = NULL;
			delete this;
			return f;
		}

		Formula * * newMultiplierFormulas = new Formula *[_noOfMultipliers - ones];
		for (int iFormula = 0; iFormula < _noOfMultipliers; iFormula++) {
			if (_multiplierFormulas[iFormula]->IsConstant(CONSTANT_CURRENT_RUN) &&
				_multiplierFormulas[iFormula]->DE_Compute(NULL, 0.0, USE_SCALEFACTOR) == 1.0)
			{
				delete _multiplierFormulas[iFormula];
				_multiplierFormulas[iFormula] = NULL;
			}
			else
			{
				newMultiplierFormulas[newIndex] = _multiplierFormulas[iFormula];
				++newIndex;
			}
		}
		_noOfMultipliers = _noOfMultipliers - ones;
		delete[] _multiplierFormulas;
		_multiplierFormulas = newMultiplierFormulas;
	}

	return this;
}

void ProductFormula::setFormula(int noOfMultipliers, Formula * * multiplierFormulas)
{
	// free old memory if necessary
	if (_multiplierFormulas)
	{
		for (int i = 0; i<_noOfMultipliers; i++)
			delete _multiplierFormulas[i];
		delete[] _multiplierFormulas;
	}

	// set new pointers
	_noOfMultipliers = noOfMultipliers;
	_multiplierFormulas = new Formula *[noOfMultipliers];
	for (int i = 0; i < noOfMultipliers; i++)
		_multiplierFormulas[i] = multiplierFormulas[i];
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

void ProductFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	for (int iFormula = 0; iFormula != _noOfMultipliers; iFormula++)
	{
		if (iFormula != 0)
			mrOut << "*";
		_multiplierFormulas[iFormula]->WriteCppCode(mrOut);
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

void ProductFormula::AppendUsedVariables(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
{
	for (int iFormula = 0;iFormula != _noOfMultipliers;iFormula++)
	{
		_multiplierFormulas[iFormula]->AppendUsedVariables(usedVariablesIndices,variablesIndicesUsedInSwitchAssignments);
	}
}

void ProductFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	for (int iFormula = 0; iFormula != _noOfMultipliers; iFormula++)
	{
		_multiplierFormulas[iFormula]->AppendUsedParameters(usedParameterIDs);
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
