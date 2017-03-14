#include "SimModelSpecs/ExplicitFormulaSpecsHelper.h"
#include <direct.h>
#include "SimModelManaged/Conversions.h"

namespace UnitTests
{
    using namespace OSPSuite::BDDHelper;
    using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace System::IO;
	using namespace System;
	using namespace SimModelNET;
	using namespace SimModelNative;


	void ParameterExtender::SetName(const std::string & name)
	{
		_name = name;
	}

	void SpeciesExtender::SetName(const std::string & name)
	{
		_name = name;
	}

	void SpeciesExtender::SetODEIndex(int odeIndex)
	{
		m_ODEIndex = odeIndex;
	}

	SimModelNative::ExplicitFormula * SpeciesExtender::GetFormula()
	{
		return dynamic_cast<SimModelNative::ExplicitFormula *>(_valueFormula);
	}

	void ObserverExtender::SetName(const std::string & name)
	{
		_name = name;
	}

	Quantity * QuantityReferenceExtender::GetQuantity()
	{
		return _quantity;
	}

	void QuantityReferenceExtender::SetQuantity(SimModelNative::Quantity * quantity)
	{
		_quantity = quantity;

		if (dynamic_cast<Species *>(_quantity) != NULL)
			_isSpecies = true;

		else if (dynamic_cast<Parameter *>(_quantity) != NULL)
			_isParameter = true;

		else if (dynamic_cast<Observer *>(_quantity) != NULL)
			_isObserver = true;
	}

	bool QuantityReferenceExtender::IsTime()
	{
		return _isTime;
	}

	void QuantityReferenceExtender::SetIsTime(bool istime)
	{
		_isTime = istime;
	}

	void ExplicitFormulaExtender::SetEquation(const string & equation)
	{
		_equation = equation;
	}

	TObjectVector<QuantityReference> & ExplicitFormulaExtender::QuantityRefs()
	{
		return _quantityRefs;
	}

	ExplicitFormulaExtender * ExplicitFormulaExtender::FormulaFrom(const std::string & equation)
	{
		ExplicitFormulaExtender * formula = new ExplicitFormulaExtender();
		formula->SetEquation(equation);
		formula->Finalize();

		return formula;
	}

	void ExplicitFormulaExtender::AddQuantityReference(SimModelNative::Quantity * quantity,
		                                               bool isFixed)
	{
		quantity->SetIsFixed(isFixed);

		QuantityReferenceExtender * quantityRef = new QuantityReferenceExtender();
		quantityRef->SetQuantity(quantity);
		
		//set alias=name
		quantityRef->SetAlias(quantity->GetName());

		_quantityRefs.push_back(quantityRef);
	}

	bool ExplicitFormulaExtender::Simplify(bool forCurrentRunOnly)
	{
		if(_formula == NULL)
			ExplicitFormula::SetupFormula();

		return ExplicitFormula::Simplify(forCurrentRunOnly);
	}

	ParameterExtender *  ExplicitFormulaExtender::AddParameterReference(
		const string & name, const string & equation, bool isFixed)
	{
		return AddParameterReference(name, FormulaFrom(equation), isFixed);
	}

	ParameterExtender * ExplicitFormulaExtender::AddParameterReference(
		const string & name, ExplicitFormula * formula, bool isFixed)
	{
		ParameterExtender * param = new ParameterExtender();
		param->SetName(name);
		param->SetFormula(formula);

		AddQuantityReference(param, isFixed);

		return param;
	}

	void ExplicitFormulaExtender::AddParameterReference(ParameterExtender * parameter)
	{
		AddQuantityReference(parameter, parameter->IsFixed());
	}

	ObserverExtender *  ExplicitFormulaExtender::AddObserverReference(
		const std::string & name, const std::string & equation, bool isFixed)
	{
		return AddObserverReference(name, FormulaFrom(equation), isFixed);
	}

	ObserverExtender * ExplicitFormulaExtender::AddObserverReference(
		const std::string & name, ExplicitFormula * formula, bool isFixed)
	{
		ObserverExtender * obs = new ObserverExtender();
		obs->SetName(name);
		obs->SetFormula(formula);

		AddQuantityReference(obs, isFixed);

		return obs;
	}

	void ExplicitFormulaExtender::AddObserverReference(ObserverExtender * observer)
	{
		AddQuantityReference(observer, observer->IsFixed());
	}

	SpeciesExtender *  ExplicitFormulaExtender::AddSpeciesReference(
		const std::string & name, const std::string & initialValueEquation, bool isFixed)
	{
		return AddSpeciesReference(name, FormulaFrom(initialValueEquation), isFixed);
	}

	SpeciesExtender *  ExplicitFormulaExtender::AddSpeciesReference(
		const std::string & name, ExplicitFormula * initialValueFormula, bool isFixed)
	{
		SpeciesExtender * species = new SpeciesExtender();
		species->SetName(name);
		species->SetFormula(initialValueFormula);

		AddQuantityReference(species, isFixed);

		return species;
	}

	void ExplicitFormulaExtender::AddSpeciesReference(SpeciesExtender * species)
	{
		AddQuantityReference(species, species->IsFixed());
	}

	void ExplicitFormulaExtender::AddTimeReference()
	{
		QuantityReferenceExtender * quantityRef = new QuantityReferenceExtender();
		quantityRef->SetIsTime(true);

		//set alias="Time"
		quantityRef->SetAlias(csTime);

		_quantityRefs.push_back(quantityRef);
	}
}