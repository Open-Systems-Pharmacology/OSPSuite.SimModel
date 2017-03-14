#ifndef _ExplicitFormulaSpecsHelper_H_
#define _ExplicitFormulaSpecsHelper_H_

#include "SimModel/ExplicitFormula.h"
#include "SimModel/QuantityReference.h"
#include "SimModel/Quantity.h"
#include "SimModel/Parameter.h"
#include "SimModel/Observer.h"
#include "SimModel/Species.h"
#include "SimModel/Formula.h"

#include <vector>
#include <string>

namespace UnitTests
{
	using namespace std;
    using namespace OSPSuite::BDDHelper;
    using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace System::IO;
	using namespace System;

	class ParameterExtender :
		public SimModelNative::Parameter
	{
	public:
		void SetName(const std::string & name);
	};

	class SpeciesExtender :
		public SimModelNative::Species
	{
	public:
		void SetName(const std::string & name);
		void SetODEIndex(int odeIndex);
		SimModelNative::ExplicitFormula * GetFormula();
	};

	class ObserverExtender :
		public SimModelNative::Observer
	{
	public:
		void SetName(const std::string & name);
	};



	//provide access to protected functions via public inheritance
	class QuantityReferenceExtender : public SimModelNative::QuantityReference
	{
	public:
		SimModelNative::Quantity * GetQuantity();
		void SetQuantity(SimModelNative::Quantity * quantity);

		bool IsTime();
		void SetIsTime(bool istime);
	};

	//provide access to protected functions via public inheritance
	class ExplicitFormulaExtender : public SimModelNative::ExplicitFormula
	{
	protected:
		ExplicitFormulaExtender * FormulaFrom(const std::string & equation);
	public:
		//const string Equation();
		void SetEquation(const string & equation);

		SimModelNative::TObjectVector<SimModelNative::QuantityReference> & QuantityRefs();

		ParameterExtender * AddParameterReference(const std::string & name, 
			                                      const std::string & equation,
			                                      bool isFixed = false);
		ParameterExtender * AddParameterReference(const std::string & name, 
			                                      ExplicitFormula * formula,
			                                      bool isFixed = false);
		void AddParameterReference(ParameterExtender * parameter);

		ObserverExtender * AddObserverReference(const std::string & name, 
			                                    const std::string & equation,
			                                    bool isFixed = false);
		ObserverExtender * AddObserverReference(const std::string & name, 
			                                    ExplicitFormula * formula,
			                                    bool isFixed = false);
		void AddObserverReference(ObserverExtender * parameter);

		SpeciesExtender * AddSpeciesReference(const std::string & name, 
			                                  const std::string & initialValueEquation,
								              bool isFixed = false);
		SpeciesExtender * AddSpeciesReference(const std::string & name, 
			                                  ExplicitFormula * initialValueFormula,
								              bool isFixed = false);
		void AddSpeciesReference(SpeciesExtender * parameter);

		void AddTimeReference();

		void AddQuantityReference(SimModelNative::Quantity * quantity, bool isFixed = false);

		bool Simplify(bool forCurrentRunOnly);
	};

}


#endif //_ExplicitFormulaSpecsHelper_H_
