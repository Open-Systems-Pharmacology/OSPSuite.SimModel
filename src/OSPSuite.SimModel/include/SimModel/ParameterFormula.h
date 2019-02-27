#ifndef _ParameterFormula_H_
#define _ParameterFormula_H_

#include "SimModel/Formula.h"
#include "SimModel/QuantityReference.h"

namespace SimModelNative
{

class ParameterFormula : 	
	public Formula
{
	private:
		std::string m_Name;
		QuantityReference _quantityRef;
	
	public:
		ParameterFormula ();
		ParameterFormula (long formulaId, const std::string & name, Parameter * parameter, const std::string & alias);

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
		virtual Formula * DE_Jacobian(const int iEquation);
		virtual Formula * clone();
		virtual Formula * RecursiveSimplify();

		virtual void Finalize();

		virtual bool IsTime();

		virtual bool IsConstant(bool forCurrentRunOnly);

		virtual bool IsZero(void);

		virtual std::vector <double> SwitchTimePoints();

		std::string Equation();

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
		virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);
		virtual void InsertNewParameters(std::map<std::string, ParameterFormula *> & mapNewP);

		virtual void UpdateIndicesOfReferencedVariables();
		//Update the value to ODEScaleFactor of the scale factor of the variable with the id referenced by this formula.
		virtual void UpdateScaleFactorOfReferencedVariable(const int quantity_id, const double ODEScaleFactor);
	
	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
		bool UseBracketsForODESystemGeneration ();
};

}//.. end "namespace SimModelNative"


#endif //_ParameterFormula_H_

