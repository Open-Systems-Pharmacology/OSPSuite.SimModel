#ifdef _WINDOWS
#pragma warning(disable:4786)
#pragma warning(disable:4244)
#endif

#include "SimModel/SimpleProductFormula.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/MathHelper.h"
#include "SimModel/ProductFormula.h"
#include "SimModel/ConstantFormula.h"
#include "SimModel/Species.h"
#include <assert.h>

#ifdef linux
#include <algorithm> //for std::find
#endif

namespace SimModelNative
{

using namespace std;

SimpleProductFormula::SimpleProductFormula ()
{
	m_ODEIndexVector=NULL;
	m_ODEScaleFactorVector = NULL;
	m_K=1.; //Constant not compulsory
	m_ODEIndexVectorSize=0;
}

SimpleProductFormula::~SimpleProductFormula ()
{
	if (m_ODEIndexVector!=NULL) delete [] m_ODEIndexVector;
	if (m_ODEScaleFactorVector!=NULL) delete [] m_ODEScaleFactorVector;
}

void SimpleProductFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	const char * ERROR_SOURCE = "SimpleProductFormula::LoadFromXMLNode";
	
	// Partial XML
	//
	//  <SimpleProduct>
	//    <Constant.../>
	//    <Variable.../>
	//    <Variable.../>
	//  </SimpleProduct>
	//

	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(FormulaName::SimpleProduct));
	
	//Get Constant Node if exists  return 1
	m_K = pNode.GetChildNodeValue(FormulaName::Constant, 1.0);
	
	//Loop all Child and get names of variables 
	for (XMLNode pChildNode = pNode.GetFirstChild(); 
		 !pChildNode.IsNull();
		 pChildNode = pChildNode.GetNextSibling()) 
	{
		if (pChildNode.HasName(FormulaName::Variable))
			m_VariableNames.push_back(pChildNode.GetValue());
	}
	
	//Redim Vector
	m_ODEIndexVector  = new int[m_VariableNames.size()];
	if (!m_ODEIndexVector)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Cannot allocate memory");
	
	
	m_ODEScaleFactorVector  = new double [m_VariableNames.size()];
	if (!m_ODEScaleFactorVector)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Cannot allocate memory");
	
	//Initialize them with valid values 
	for(unsigned int i=0;i<m_VariableNames.size();i++)
	{
		m_ODEIndexVector[i]=0;
		m_ODEScaleFactorVector[i]=1.;
	}

	//cache number of variables for performance optimization
	m_ODEIndexVectorSize = (unsigned int)m_VariableNames.size();
}

void SimpleProductFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{

}

void SimpleProductFormula::UpdateFromQuantityReference(const QuantityReference & quantityReference)
{
	//Set index of all variables which are involved in this product
	
	for (unsigned int i=0;i<m_VariableNames.size();i++)
	{
		//We found the species=> we save index in the index collection
		if (m_VariableNames[i]==quantityReference.GetAlias())
		{
			m_ODEIndexVector[i] = quantityReference.GetODEIndex();
			m_ODEScaleFactorVector[i] = quantityReference.GetODEScaleFactor ();
		}
	}
}

void SimpleProductFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	_quantityRefs.push_back(quantityReference);
	UpdateFromQuantityReference(quantityReference);

	//If the quantity is a species, add this formula to the list of entities that cache its scale factor (used for updating scale factors)
	if (quantityReference.IsSpecies())
	{
		Species * species = quantityReference.GetSpecies();
		species->AddEntityWithCachedScaleFactor(this);
	}
}

bool SimpleProductFormula::IsZero(void)
{
	return (m_K == 0.0);
}

double SimpleProductFormula::Safe_DE_Compute(const double* y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	//Formula is K1*A*B-C - K2*D*E*F				
	double dValue = m_K;
	double scaleFactor;
	for (int i = 0; i < m_ODEIndexVectorSize; i++)
	{
		switch (scaleFactorMode)
		{
		case USE_SCALEFACTOR:
			scaleFactor = m_ODEScaleFactorVector[i];
			break;
		case IGNORE_SCALEFACTOR:
			scaleFactor = 1.0;
			break;
		default:
			throw ErrorData(ErrorData::ED_ERROR, "SimpleProductFormula::DE_Compute", "Invalid scale factor mode passed" + FormulaInfoForErrorMessage());
		}

		dValue *= y[m_ODEIndexVector[i]] * scaleFactor;
	}

	// Return Formula
	return dValue;
}

double SimpleProductFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	//Formula is K1*A*B-C - K2*D*E*F				
	double dValue = m_K;
	double scaleFactor;
	for (int i=0;i<m_ODEIndexVectorSize;i++)
	{
		switch (scaleFactorMode)
		{
		case USE_SCALEFACTOR:
			scaleFactor = m_ODEScaleFactorVector[i];
			break;
		case IGNORE_SCALEFACTOR:
			scaleFactor = 1.0;
			break;
		default:
			throw ErrorData(ErrorData::ED_ERROR, "SimpleProductFormula::DE_Compute", "Invalid scale factor mode passed" + FormulaInfoForErrorMessage());
		}

		dValue *=  y[m_ODEIndexVector[i]] * scaleFactor;
	}
	
	// Return Formula
	return dValue;
}

void SimpleProductFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	if (preFactor == 0.0)
		return;
	
	if (m_K == 0.0)
		return;

	//---- calc Jacobian for each DE variable
	for(int varIdx = 0; varIdx < m_ODEIndexVectorSize; varIdx++)
	{
		//---- for the Variable with the index varIdx: 
		// dF/dy[varIdx] = m_K*Product_i(y_i*ScaleFactor_i) (with i=1..VariablesSize and i != varIdx)

		double prod = m_K;

		for (int i = 0; i < m_ODEIndexVectorSize; i++)
		{
			if (i == varIdx)
				continue;

			prod *= y[m_ODEIndexVector[i]] * m_ODEScaleFactorVector[i];

			if (prod == 0.0)
				break;
		}

		if (prod == 0.0)
			continue; //no contribution to jacobian from the current variable

		MATRIX_ELEM(jacobian,iEquation,m_ODEIndexVector[varIdx]) += preFactor * prod * m_ODEScaleFactorVector[varIdx];
	}

}

Formula* SimpleProductFormula::DE_Jacobian(const int iEquation)
{
	int * pos = std::find(m_ODEIndexVector, m_ODEIndexVector + m_ODEIndexVectorSize, iEquation);
	if ( pos == m_ODEIndexVector + m_ODEIndexVectorSize )
	{
		// not dependent
		return new ConstantFormula(0.0);
	}
	else if (m_ODEIndexVectorSize == 1)
	{
		// constant remaining
		return new ConstantFormula(m_K);
	}
	else
	{
		// remove factor iEquation
      auto p = new SimpleProductFormula();
		p->m_ODEIndexVectorSize = m_ODEIndexVectorSize - 1;
		p->m_K = m_K;
		p->m_ODEIndexVector = new int[m_ODEIndexVectorSize - 1];
		p->m_ODEScaleFactorVector = new double[m_ODEIndexVectorSize - 1];

		int last = pos - m_ODEIndexVector;
		for (int i = 0; i < last; i++) {
			p->m_ODEIndexVector[i] = m_ODEIndexVector[i];
			p->m_ODEScaleFactorVector[i] = m_ODEScaleFactorVector[i];
		}
		for (int i = last+1; i < m_ODEIndexVectorSize; i++) {
			p->m_ODEIndexVector[i-1] = m_ODEIndexVector[i];
			p->m_ODEScaleFactorVector[i-1] = m_ODEScaleFactorVector[i];
		}
		return p;
	}
}

Formula * SimpleProductFormula::clone()
{
	SimpleProductFormula * f = new SimpleProductFormula();
	f->m_ODEIndexVectorSize = m_ODEIndexVectorSize;
	f->m_K = m_K;

	f->m_ODEIndexVector = new int[m_ODEIndexVectorSize];
	std::copy(m_ODEIndexVector, m_ODEIndexVector + m_ODEIndexVectorSize, f->m_ODEIndexVector);

	f->m_ODEScaleFactorVector = new double[m_ODEIndexVectorSize];
	std::copy(m_ODEScaleFactorVector, m_ODEScaleFactorVector + m_ODEIndexVectorSize, f->m_ODEScaleFactorVector);

	return f;
}

Formula * SimpleProductFormula::RecursiveSimplify()
{
	if (IsZero())
	{
		Formula * f = new ConstantFormula(0.0);
		delete this;
		return f;
	}

	return this;
}

void SimpleProductFormula::Finalize()
{
	//nothing to do
}

void SimpleProductFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	mrOut<<MathHelper::ToString(m_K);
	for(int i=0;i<m_ODEIndexVectorSize;i++)
		mrOut<<"*"<<"y("<< m_ODEIndexVector[i]+1<<")"; //SimModel indexing starts at 0, Matlab-indexing at 1 
}

void SimpleProductFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	mrOut << MathHelper::ToString(m_K);
	for (int i = 0; i < m_ODEIndexVectorSize; i++)
		mrOut << "*" << "y[" << m_ODEIndexVector[i] << "]";
}

void SimpleProductFormula::AppendUsedVariables(set<int> & usedVariablesIndices, const set<int> & variablesIndicesUsedInSwitchAssignments)
{
	for(int i=0;i<m_ODEIndexVectorSize;i++)
	{
		usedVariablesIndices.insert(m_ODEIndexVector[i]);
	}
}

void SimpleProductFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	// DE variables only involved -> nothing to do here
}

void SimpleProductFormula::UpdateIndicesOfReferencedVariables()
{
	for(unsigned int i=0; i<_quantityRefs.size(); i++)
		UpdateFromQuantityReference(_quantityRefs[i]);
}

void SimpleProductFormula::UpdateScaleFactorOfReferencedVariable(const int odeIndex, const double ODEScaleFactor)
{
	for (unsigned int i = 0; i < m_ODEIndexVectorSize; i++) 
	{
		if (m_ODEIndexVector[i] == odeIndex) 
		{
			m_ODEScaleFactorVector[i] = ODEScaleFactor;
			//theoretically the same ODE variable can appear >1 times; so no break here!
		}
	}
}

}//.. end "namespace SimModelNative"