#ifndef _ExplicitFormula_H_
#define _ExplicitFormula_H_

#include "SimModel/Formula.h"
#include "SimModel/TObjectVector.h"
#include "FuncParser/ParsedFunction.h"
#include "XMLWrapper/XMLNode.h"
#include <string>

#ifdef _WINDOWS
#pragma warning( disable : 4279)
#endif

namespace SimModelNative
{

class Simulation;

class ExplicitFormula :
	public Formula
{
private:
	bool      _isGloballySimplified;

	FuncParserNative::ParsedFunction _funcParser;
	void AddQuantityRefsFromXMLNode(XMLNode refListNode, Simulation * sim);

	XMLNode GetRateNode(FuncParserNative::ParsedFunction & parsedFunction);

protected:
	Formula * _formula;

	TObjectVector<QuantityReference> _quantityRefs;
	std::string _equation;

	void SetupFormula();
	void CreateFormulaFromEquation(const std::vector<std::string> & variableNames, 
		                           const std::vector<std::string> & parameterNames,
		                           const std::vector<double> & parameterValues,
		                           const std::vector<std::string> & parameterNotToSimplifyNames,
								   bool simplifyParameter);
	
	void WriteFormulaMatlabCode (std::ostream & mrOut);
	void WriteFormulaCppCode (std::ostream & mrOut);
	bool UseBracketsForODESystemGeneration ();

public:
	ExplicitFormula(void);
	virtual ~ExplicitFormula(void);

	virtual void LoadFromXMLNode (const XMLNode & pNode);
	virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	bool Simplify(bool forCurrentRunOnly);

	std::vector < HierarchicalFormulaObject * > GetUsedHierarchicalFormulaObjects();

	double DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode);
	void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
	virtual Formula * DE_Jacobian(const int iEquation);
	virtual Formula * clone();
	virtual Formula * RecursiveSimplify();
	void SetQuantityReference (const QuantityReference & quantityReference);

	//returns true for formulas like "2.5" or "2*sin(pi/3)"
	bool IsRefIndependent(double & value);

	virtual bool IsZero(void);

	virtual void Finalize();

	std::vector <double> SwitchTimePoints();
	virtual bool IsConstant(bool forCurrentRunOnly);

	std::string Equation();

	virtual void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
	virtual void AppendUsedParameters(std::set<int> & usedParameterIDs);

	virtual void UpdateIndicesOfReferencedVariables();
	//Update the value to ODEScaleFactor of the scale factor of the variable with the id referenced by this formula.
	virtual void UpdateScaleFactorOfReferencedVariable(const int id, const double ODEScaleFactor);
};

}//.. end "namespace SimModelNative"


#endif //_ExplicitFormula_H_
