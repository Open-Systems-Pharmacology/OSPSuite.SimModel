#ifndef _QuantityReference_H_
#define _QuantityReference_H_

#include <string>
#include "SimModel/ObjectBase.h"
#include "SimModel/XMLLoader.h"
#include "SimModel/HierarchicalFormulaObject.h"

namespace SimModelNative
{

class Quantity;
class Species;
class Parameter;

class QuantityReference : 
	public XMLLoader
{
protected:
	long _quantityId;
	std::string _alias;
	std::string _parentFormulaInfo;

	Quantity * _quantity;

	bool _isTime;
	bool _isParameter;
	bool _isObserver;
	bool _isSpecies;
	bool _isReference;

public:
	QuantityReference();

	void SetupFrom(Parameter * parameter, const std::string & alias);

	virtual ~QuantityReference(void);

	void SetParentFormulaInfo(const std::string & parentFormulaInfo);

	void LoadFromXMLNode (const XMLNode & pNode);
	void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

	bool IsTime () const;

	bool IsParameter () const;
	bool IsObserver () const;
	bool IsSpecies () const;

	bool SimplifyQuantity (bool forCurrentRunOnly);

	std::string GetAlias(void) const; 
	void SetAlias(std::string alias);

	double GetODEScaleFactor () const;
	int GetODEIndex () const;

	double GetValue (const double * y, double time, ScaleFactorUsageMode scaleFactorMode);
	void DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor);
	Formula* DE_Jacobian(const int iEquation);

	//return reference quantity as hierarchical formula object
	// (returns NULL if it's not a HFObject)
	HierarchicalFormulaObject * GetHierarchicalFormulaObject(void);

	//returns embedded  quantity casted to (Species *)
	// (returns NULL, if not a species)
	Species * GetSpecies() const;

	bool IsConstant(bool forCurrentRunOnly);
	bool IsChangedBySwitch(void);

	void AppendUsedVariables(std::set<int> & usedVariblesIndices, const std::set<int> & variblesIndicesUsedInSwitchAssignments);
	void AppendUsedParameters(std::set<int> & usedParameterIDs);

	virtual void UpdateIndicesOfReferencedVariables();
};

}//.. end "namespace SimModelNative"

#endif //_QuantityReference_H_
