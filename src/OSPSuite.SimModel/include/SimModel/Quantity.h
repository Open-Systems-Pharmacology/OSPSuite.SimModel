#ifndef _Quantity_H_
#define _Quantity_H_

#include <string>
#include "SimModel/ObjectBase.h"
#include "SimModel/XMLLoader.h"
#include "SimModel/QuantityInfo.h"
#include <set>

namespace SimModelNative
{

class Formula;
class HierarchicalFormulaObject;

//For now, Quantity = FormulaUsableObject (Species, Observer, Parameter)
class Quantity : 
	public ObjectBase
{
private:
	void setPathWithoutRoot(const std::string & objectPathDelimiter);

protected:
	//quantity name
	std::string _name;
	//path+name
	std::string _fullName; 
	
	//path without root element, e.g. "Organism|Weight" (whereas full path would be "S1|Organism|Weight")
	std::string _pathWithoutRoot;

	std::string _description;

	//original path to the parent container of the quantity
	//not required by simmodel itself - useful only for 
	//user-friendly quantity identification e.g. from Matlab
	std::string _containerPath;

	//unit - for informative purposes only
	std::string _unit;

	//should be saved as simulation output or not
	bool _isPersistable;

	//can be changed by switch during simulation run
	bool _isChangedBySwitch;

	bool _isFixed;

	//Is (directly!) used in condition formula or one of formula changes
	//(Only required for Matlab ODE export)
	bool _isUsedBySwitch;

	//formula used for parameter calculation during the simulation
	//(either original formula or some new one defined by e.g. switch)
	Formula * _valueFormula;

	//value of formula during simulation in case it's constant
	double  _value;

	//reference to the formula used at the begin of simulation
	//(e.g. formula from simulation XML file)
	Formula * _originalValueFormula;

	//value of original formula in case it's constant
	double  _originalValue;

	//id of original formula - only required for updating formula in the XML Node
	// (in case the original formula was set to null during simplifying)
	long _originalFormulaID;

	//replaces formulas without references 
	//(constant values and arithmetic expressions like "2*sin(pi/3)+3" )
	//with their numerical values
	void ReplaceRefIndependentFormula(void);
	
	//name of the formula attribute in the XML for given quantity type, 
	//e.g. "formulaId" or "initialFormulaId"
	virtual std::string getFormulaXMLAttributeName() = 0;

	double getDoubleFromSpecialValue(std::string & sValue, const char * ERROR_SOURCE);

	void DeleteFormula();

public:
	Quantity(void);
	virtual ~Quantity(void);

public:
	SIM_EXPORT std::string GetName(void);
	SIM_EXPORT std::string GetDescription(void);
	std::string GetContainerPath(void);
	std::string GetUnit(void);
	SIM_EXPORT std::string GetFullName(void);
	std::string GetPathWithoutRoot(void);

	bool IsPersistable(void);

	bool IsChangedBySwitch(void);
	void SetIsChangedBySwitch(bool changedBySwitch);

	bool IsFixed(void);
	void SetIsFixed(bool isFixed);

	virtual bool IsConstant(bool forCurrentRunOnly);

	//Reset changes after one simulation run is performed
	virtual void ResetState(void);

	virtual double GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode) = 0;
	virtual void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)=0;

	virtual bool Simplify (bool forCurrentRunOnly);

	virtual void LoadFromXMLNode (const XMLNode & pNode);
	virtual void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	//sets new initial value BEFORE the start of next simulation run
	// (e.g. from SetParameterValues or from SetODEVariableProperties)
	//this resets original value as well
	void SetInitialValue(double value);

	//sets new value (e.g. if changed by switch)
	void SetConstantValue(double value);

	HierarchicalFormulaObject * GetHierarchicalFormulaObject(void);

	bool IsFormulaEqualTo(Formula * formula);
	void SetFormula(Formula * formula);

	bool IsUsedBySwitch(void);
	void SetIsUsedBySwitch(void);

	//will be called between Load and Finalize of the parent simulation
	void InitialFillInfo(QuantityInfo & info);

	//will be called after the finalize of the parent simulation
	void FillInfo(QuantityInfo & info,const double * speciesInitialValues, double simulationStartTime);

	//Used before saving the simulation to XML - replaces the formula of
	//NOT FIXED quantity by its constant value
	void UpdateFormulaInXMLNode(XMLNode & pFormulaListNode, XMLNode & pQuantitiesListNode);

	long GetFormulaId(void);

	void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);

	virtual void UpdateIndicesOfReferencedVariables();
};

}//.. end "namespace SimModelNative"

#endif //_Quantity_H_
