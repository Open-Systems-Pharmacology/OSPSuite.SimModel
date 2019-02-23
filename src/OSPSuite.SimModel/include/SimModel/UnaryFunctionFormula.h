#ifndef _UnaryFunctionFormula_H_
#define _UnaryFunctionFormula_H_

#include "SimModel/Formula.h"

namespace SimModelNative
{

class UnaryFunctionFormula : 	
	public Formula
{
	protected:
		std::string m_FunctionName;
		Formula * m_ArgumentFormula;

		// Evaluate function value for given double argument
		virtual double EvalFunction (double arg) = 0;
		
		// For the unary function f(Arg), returns f'(Arg)
		// (d[f(Arg)] / d[x_i] = f'(Arg) * d[Arg] / d[x_i])
		virtual double GetJacobianMultiplier (double arg) = 0;
		virtual Formula* GetJacobianMultiplier (Formula *m_ArgumentFormula) = 0;
	
	public:
		UnaryFunctionFormula (std::string funcName);
		virtual ~UnaryFunctionFormula ();

		virtual void LoadFromXMLNode (const XMLNode & pNode);
		virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);
		virtual void SetQuantityReference (const QuantityReference & quantityReference);
		virtual double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
		virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
		virtual Formula * DE_Jacobian(const int iEquation);
		virtual Formula * clone()=0;
		virtual Formula * RecursiveSimplify();

		void setFormula(Formula* argumentFormula);

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

class AcosFormula : 	
	public UnaryFunctionFormula
{
	public:
		AcosFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class AsinFormula : 	
	public UnaryFunctionFormula
{
	public:
		AsinFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class AtanFormula : 	
	public UnaryFunctionFormula
{
	public:
		AtanFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class CoshFormula : 	
	public UnaryFunctionFormula
{
	public:
		CoshFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class CosFormula : 	
	public UnaryFunctionFormula
{
	public:
		CosFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class ExpFormula : 	
	public UnaryFunctionFormula
{
	public:
		ExpFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class LnFormula : 	
	public UnaryFunctionFormula
{
	public:
		//	FuncName passed in ctor because both "Ln" and "Log" are accepted
		LnFormula (std::string & funcName);
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class Log10Formula : 	
	public UnaryFunctionFormula
{
	public:
		Log10Formula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class SinhFormula : 	
	public UnaryFunctionFormula
{
	public:
		SinhFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class SinFormula : 	
	public UnaryFunctionFormula
{
	public:
		SinFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class SqrtFormula : 	
	public UnaryFunctionFormula
{
	public:
		SqrtFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class TanhFormula : 	
	public UnaryFunctionFormula
{
	public:
		TanhFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};

class TanFormula : 	
	public UnaryFunctionFormula
{
	public:
		TanFormula ();
		virtual Formula* clone();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
		Formula* GetJacobianMultiplier(Formula *m_ArgumentFormula);
};


}//.. end "namespace SimModelNative"


#endif //_UnaryFunctionFormula_H_

