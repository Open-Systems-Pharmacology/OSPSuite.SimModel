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
	
	public:
		UnaryFunctionFormula (std::string funcName);
		virtual ~UnaryFunctionFormula ();

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

class AcosFormula : 	
	public UnaryFunctionFormula
{
	public:
		AcosFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class AsinFormula : 	
	public UnaryFunctionFormula
{
	public:
		AsinFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class AtanFormula : 	
	public UnaryFunctionFormula
{
	public:
		AtanFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class CoshFormula : 	
	public UnaryFunctionFormula
{
	public:
		CoshFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class CosFormula : 	
	public UnaryFunctionFormula
{
	public:
		CosFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class ExpFormula : 	
	public UnaryFunctionFormula
{
	public:
		ExpFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class LnFormula : 	
	public UnaryFunctionFormula
{
	public:
		//	FuncName passed in ctor because both "Ln" and "Log" are accepted
		LnFormula (std::string & funcName);
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class Log10Formula : 	
	public UnaryFunctionFormula
{
	public:
		Log10Formula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class SinhFormula : 	
	public UnaryFunctionFormula
{
	public:
		SinhFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class SinFormula : 	
	public UnaryFunctionFormula
{
	public:
		SinFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class SqrtFormula : 	
	public UnaryFunctionFormula
{
	public:
		SqrtFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class TanhFormula : 	
	public UnaryFunctionFormula
{
	public:
		TanhFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};

class TanFormula : 	
	public UnaryFunctionFormula
{
	public:
		TanFormula ();
	
	protected:
		double EvalFunction (double arg);
		double GetJacobianMultiplier (double arg);
};


}//.. end "namespace SimModelNative"


#endif //_UnaryFunctionFormula_H_

