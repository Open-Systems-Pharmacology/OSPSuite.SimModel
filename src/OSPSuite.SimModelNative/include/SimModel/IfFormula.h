#ifndef _IfFormula_H_
#define _IfFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class IfFormula : 	
	public Formula
{
	private:
		Formula * m_IfStatement;
		Formula * m_ThenStatement;
		Formula * m_ElseStatement;
	
	public:
		IfFormula ();
		virtual ~IfFormula ();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
		virtual Formula * DE_Jacobian(const int iEquation);
		virtual Formula * clone();
		virtual Formula * RecursiveSimplify();
		void setFormula(Formula * IfStatement, Formula * ThenStatement, Formula * ElseStatement);

		virtual void Finalize();

		virtual std::vector <double> SwitchTimePoints();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariablesIndices, const std::set<int> & variablesIndicesUsedInSwitchAssignments);
		virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

		virtual void UpdateIndicesOfReferencedVariables();
	
	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};


}//.. end "namespace SimModelNative"


#endif //_IfFormula_H_

