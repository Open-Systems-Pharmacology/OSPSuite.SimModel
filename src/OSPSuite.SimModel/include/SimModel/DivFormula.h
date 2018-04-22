#ifndef _DivFormula_H_
#define _DivFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class DivFormula : 	
	public Formula
{
	private:
		Formula * m_DenominatorFormula;
		Formula * m_NumeratorFormula;
	
	public:
		virtual ~DivFormula ();
		DivFormula ();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
		virtual Formula * DE_Jacobian(const int iEquation);
		virtual Formula * clone();
		virtual Formula * RecursiveSimplify();
		void setFormula(Formula* numeratorFormula, Formula* denominatorFormula);

		virtual void Finalize();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
		virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

		virtual void UpdateIndicesOfReferencedVariables();
	
	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};


}//.. end "namespace SimModelNative"


#endif //_DivFormula_H_

