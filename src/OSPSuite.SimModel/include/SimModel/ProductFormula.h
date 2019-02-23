#ifndef _ProductFormula_H_
#define _ProductFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class ProductFormula : 	
	public Formula
{
	private:
		Formula * * _multiplierFormulas;
		int _noOfMultipliers;
	
	public:
		ProductFormula ();
		virtual ~ProductFormula ();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
		virtual Formula * DE_Jacobian(const int iEquation);
		virtual Formula * clone();
		virtual Formula * RecursiveSimplify();
		void setFormula(int noOfMultipliers, Formula * * multiplierFormulas);

		virtual void Finalize();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
		virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

		virtual void UpdateIndicesOfReferencedVariables();
		//Update the value to ODEScaleFactor of the scale factor of the variable with the id referenced by this formula.
		virtual void UpdateScaleFactorOfReferencedVariable(const int id, const double ODEScaleFactor);
	
	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

}//.. end "namespace SimModelNative"

#endif //_ProductFormula_H_

