#ifndef _ConstantFormula_H_
#define _ConstantFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class ConstantFormula : 	
	public Formula
{
	private:
		double m_Value;
	
	public:
		ConstantFormula();
		ConstantFormula(double value);

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

		void SetQuantityReference (const QuantityReference & quantityReference);

		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);

		void SetValue (double p_Value);

		virtual bool IsConstant(bool forCurrentRunOnly);

		virtual std::vector <double> SwitchTimePoints();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);

		virtual void UpdateIndicesOfReferencedVariables();
	
	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		bool UseBracketsForODESystemGeneration ();
};

}//.. end "namespace SimModelNative"

#endif //_ConstantFormula_H_

