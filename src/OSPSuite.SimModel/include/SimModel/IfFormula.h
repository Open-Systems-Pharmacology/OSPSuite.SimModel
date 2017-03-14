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

		virtual void Finalize();

		virtual std::vector <double> SwitchTimePoints();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);

		virtual void UpdateIndicesOfReferencedVariables();
	
	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
};


}//.. end "namespace SimModelNative"


#endif //_IfFormula_H_

