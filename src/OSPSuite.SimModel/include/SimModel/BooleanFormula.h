#ifndef _BooleanFormula_H_
#define _BooleanFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class BooleanFormula : 	
	public Formula
{
	protected:
		Formula * m_SecondOperandFormula;
		Formula * m_FirstOperandFormula;
	
		std::vector <double> SwitchTimePointFromComparisonFormula();


	public:
		BooleanFormula ();
		virtual ~BooleanFormula ();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
		virtual Formula * DE_Jacobian(const int iEquation);
		virtual Formula * clone() = 0;
		virtual Formula * RecursiveSimplify();
		void setFormula(Formula* FirstOperandFormula, Formula* SecondOperandFormula);

		virtual void Finalize();

		virtual std::vector <double> SwitchTimePoints();

		virtual bool IsZero(void);

		virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
		virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);
		void SwitchFormulaFromComparisonFormula(std::vector<Formula*> &vecExplicit, std::vector<Formula*> &vecImplicit);

		virtual void UpdateIndicesOfReferencedVariables();
};

class AndFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual bool IsZero(void); //special treatment for the AND-formula

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class EqualFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual std::vector <double> SwitchTimePoints();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class GreaterEqualFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual std::vector <double> SwitchTimePoints();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class GreaterFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual std::vector <double> SwitchTimePoints();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class LessEqualFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual std::vector <double> SwitchTimePoints();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class LessFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual std::vector <double> SwitchTimePoints();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class NotFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class OrFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

class UnequalFormula : 	
	public BooleanFormula
{
	public:
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual Formula* clone();
		virtual std::vector <double> SwitchTimePoints();

	protected:
		virtual void WriteFormulaMatlabCode (std::ostream & mrOut);
		virtual void WriteFormulaCppCode (std::ostream & mrOut);
};

}//.. end "namespace SimModelNative"

#endif //_BooleanFormula_H_

