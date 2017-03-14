#ifndef _SimpleProductFormula_H_
#define _SimpleProductFormula_H_

#include "SimModel/Formula.h"
#include <vector>
#include <string>

namespace SimModelNative
{

class SimpleProductFormula : 	
	public Formula
{
	private:
		int m_ODEIndexVectorSize;
		int * m_ODEIndexVector;
		double * m_ODEScaleFactorVector;
		double m_K;
		std::vector<std::string> m_VariableNames;
		std::vector<QuantityReference> _quantityRefs;

		void UpdateFromQuantityReference(const QuantityReference & quantityReference);
	
	public:
		SimpleProductFormula ();
		virtual ~SimpleProductFormula ();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);

		virtual void Finalize();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);

		virtual void UpdateIndicesOfReferencedVariables();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
};

}//.. end "namespace SimModelNative"

#endif //_SimpleProductFormula_H_

