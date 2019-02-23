#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/SumFormula.h"
#include "SimModel/FormulaFactory.h"
#include "XMLWrapper/XMLNode.h"
#include "SimModel/ConstantFormula.h"
#include <assert.h>

#ifdef _WINDOWS_PRODUCTION
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

Formula* SumFormula::DE_Jacobian(const int iEquation)
{
	SumFormula * s = new SumFormula();

	Formula * * sum = new Formula*[_noOfSummands];
	for (int iFormula = 0; iFormula != _noOfSummands; iFormula++)
		sum[iFormula] = _summandFormulas[iFormula]->DE_Jacobian(iEquation);

	s->setFormula(_noOfSummands, sum);

	delete[] sum;
	return s;
}

Formula * SumFormula::clone()
{
	SumFormula * f = new SumFormula();

	f->_noOfSummands = _noOfSummands;
	f->_summandFormulas = new Formula*[_noOfSummands];
	for (int iFormula = 0; iFormula != _noOfSummands; iFormula++)
		f->_summandFormulas[iFormula] = _summandFormulas[iFormula]->clone();

	return f;
}

Formula * SumFormula::RecursiveSimplify()
{
	bool isConstant = true;
	int zeros = 0;
	for (int iFormula = 0; iFormula < _noOfSummands; iFormula++) {
		_summandFormulas[iFormula] = _summandFormulas[iFormula]->RecursiveSimplify();
		isConstant &= _summandFormulas[iFormula]->IsConstant(CONSTANT_CURRENT_RUN);
		zeros += _summandFormulas[iFormula]->IsZero();
	}

	if (isConstant)
	{
		ConstantFormula * f = new ConstantFormula(DE_Compute(NULL, 0.0, USE_SCALEFACTOR));
		delete this;
		return f;
	}
	else if (zeros) {
		int newIndex = 0;

		if (_noOfSummands - zeros == 1) {
			for (int iFormula = 0; iFormula < _noOfSummands; iFormula++)
				if (!_summandFormulas[iFormula]->IsZero())
					newIndex = iFormula;

			Formula * f = _summandFormulas[newIndex];
			_summandFormulas[newIndex] = NULL; // prevent deletion by destructor
			delete this;
			return f;
		}

		Formula * * newSummandFormulas = new Formula * [_noOfSummands - zeros];
		for (int iFormula = 0; iFormula < _noOfSummands; iFormula++) {
			if (_summandFormulas[iFormula]->IsZero())
			{
				delete _summandFormulas[iFormula];
				_summandFormulas[iFormula] = NULL; // prevent deletion by destructor
			}
			else
			{
				newSummandFormulas[newIndex] = _summandFormulas[iFormula];
				++newIndex;
			}
		}
		_noOfSummands = _noOfSummands - zeros;
		delete[] _summandFormulas;
		_summandFormulas = newSummandFormulas;
	}

	return this;
}

void SumFormula::setFormula(int noOfSummands, Formula * * summandFormulas)
{
	// free old memory if necessary
	if (_summandFormulas)
	{
		for (int i = 0; i<_noOfSummands; i++)
			delete _summandFormulas[i];
		delete[] _summandFormulas;
	}

	// set new pointers
	_noOfSummands = noOfSummands;
	_summandFormulas = new Formula *[noOfSummands];
	for (int i = 0; i < noOfSummands; i++)
		_summandFormulas[i] = summandFormulas[i];
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

void SumFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	for (int iFormula = 0; iFormula != _noOfSummands; iFormula++)
	{
		if (iFormula != 0)
			mrOut << "+";
		_summandFormulas[iFormula]->WriteCppCode(mrOut);
	}
}

void SumFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++)
	{
		_summandFormulas[iFormula]->AppendUsedVariables(usedVariblesIndices,variblesIndicesUsedInSwitchAssignments);
	}
}

void SumFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	for (int iFormula = 0; iFormula != _noOfSummands; iFormula++)
	{
		_summandFormulas[iFormula]->AppendUsedParameters(usedParameterIDs);
	}
}

void SumFormula::UpdateIndicesOfReferencedVariables()
{
	for (int iFormula = 0;iFormula != _noOfSummands;iFormula++)
	{
		_summandFormulas[iFormula]->UpdateIndicesOfReferencedVariables();
	}
}

void SumFormula::UpdateScaleFactorOfReferencedVariable(const int id, const double ODEScaleFactor)
{
	for (int iFormula = 0; iFormula != _noOfSummands; iFormula++)
	{
		_summandFormulas[iFormula]->UpdateScaleFactorOfReferencedVariable(id, ODEScaleFactor);
	}
}

}//.. end "namespace SimModelNative"