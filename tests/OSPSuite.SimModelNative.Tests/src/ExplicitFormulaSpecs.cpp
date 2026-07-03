#include <gtest/gtest.h>
#include "SimModel/ExplicitFormula.h"
#include "SimModel/QuantityReference.h"
#include "SimModel/Quantity.h"
#include "SimModel/Parameter.h"
#include "SimModel/Species.h"
#include "SimModel/Formula.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/TObjectVector.h"
#include <ErrorData.h>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace SimModelNative;

namespace
{
   // Render a numeric value as an equation string (replaces the C++/CLI helper's
   // dependency on XMLHelper::ToString).
   std::string FormatNumber(double value)
   {
      std::ostringstream os;
      os << std::setprecision(16) << value;
      return os.str();
   }
}

// --- Helper extenders (originally in ExplicitFormulaSpecsHelper.{h,cpp}) -------
// They provide access to protected members of the production classes via public
// inheritance so the explicit-formula behaviour can be exercised directly.

class ParameterExtender : public Parameter
{
public:
   void SetName(const std::string& name) { _name = name; }
};

class SpeciesExtender : public Species
{
public:
   void SetName(const std::string& name) { _name = name; }
};

class QuantityReferenceExtender : public QuantityReference
{
public:
   void SetQuantity(Quantity* quantity)
   {
      _quantity = quantity;

      if (dynamic_cast<Species*>(_quantity) != nullptr)
         _isSpecies = true;
      else if (dynamic_cast<Parameter*>(_quantity) != nullptr)
         _isParameter = true;
   }

   void SetIsTime(bool isTime) { _isTime = isTime; }
};

class ExplicitFormulaExtender : public ExplicitFormula
{
public:
   void SetEquation(const std::string& equation) { _equation = equation; }

   TObjectVector<QuantityReference>& QuantityRefs() { return _quantityRefs; }

   ParameterExtender* AddParameterReference(const std::string& name, const std::string& equation, bool isFixed = false)
   {
      return AddParameterReference(name, FormulaFrom(equation), isFixed);
   }

   ParameterExtender* AddParameterReference(const std::string& name, ExplicitFormula* formula, bool isFixed = false)
   {
      ParameterExtender* param = new ParameterExtender();
      param->SetName(name);
      param->SetFormula(formula);

      AddQuantityReference(param, isFixed);

      return param;
   }

   SpeciesExtender* AddSpeciesReference(const std::string& name, const std::string& initialValueEquation, bool isFixed = false)
   {
      return AddSpeciesReference(name, FormulaFrom(initialValueEquation), isFixed);
   }

   SpeciesExtender* AddSpeciesReference(const std::string& name, ExplicitFormula* initialValueFormula, bool isFixed = false)
   {
      SpeciesExtender* species = new SpeciesExtender();
      species->SetName(name);
      species->SetFormula(initialValueFormula);

      AddQuantityReference(species, isFixed);

      return species;
   }

   void AddSpeciesReference(SpeciesExtender* species)
   {
      AddQuantityReference(species, species->IsFixed());
   }

   void AddTimeReference()
   {
      QuantityReferenceExtender* quantityRef = new QuantityReferenceExtender();
      quantityRef->SetIsTime(true);
      quantityRef->SetAlias(csTime);

      _quantityRefs.push_back(quantityRef);
   }

   void AddQuantityReference(Quantity* quantity, bool isFixed = false)
   {
      quantity->SetIsFixed(isFixed);

      QuantityReferenceExtender* quantityRef = new QuantityReferenceExtender();
      quantityRef->SetQuantity(quantity);
      quantityRef->SetAlias(quantity->GetName());

      _quantityRefs.push_back(quantityRef);
   }

   bool Simplify(bool forCurrentRunOnly)
   {
      if (_formula == nullptr)
         ExplicitFormula::SetupFormula();

      return ExplicitFormula::Simplify(forCurrentRunOnly);
   }

protected:
   ExplicitFormulaExtender* FormulaFrom(const std::string& equation)
   {
      ExplicitFormulaExtender* formula = new ExplicitFormulaExtender();
      formula->SetEquation(equation);
      formula->Finalize();

      return formula;
   }
};

// --- Tests --------------------------------------------------------------------

TEST(when_creating_for_references_independent_equation, should_calculate_correct_value)
{
   const double pi = 2 * std::asin(1.0);

   ExplicitFormulaExtender formula;
   formula.SetEquation("sin(pi/2)*sqrt(4.0)");
   formula.Finalize();

   double value = formula.DE_Compute(nullptr, 0.0, USE_SCALEFACTOR);

   EXPECT_NEAR(std::sin(pi / 2) * std::sqrt(4.0), value, 1e-10);
}

class when_creating_for_parameter_and_species_references : public ::testing::Test
{
protected:
   double _p1 = 0, _p2 = 0, _x = 0, _y = 0, _value = 0;
   ExplicitFormulaExtender _formula;

   void SetUp() override
   {
      _formula.SetEquation("p1*x+p2*y");

      _p1 = 2; _p2 = 3; _x = 4; _y = 5;
      _value = _p1 * _x + _p2 * _y;

      _formula.AddParameterReference("p1", FormatNumber(_p1));
      _formula.AddParameterReference("p2", FormatNumber(_p2));

      SpeciesExtender* X = _formula.AddSpeciesReference("x", "-1"); // x value doesn't matter
      SpeciesExtender* Y = _formula.AddSpeciesReference("y", "-1"); // y value doesn't matter

      X->SetIsChangedBySwitch(true); // force using as variable, not as parameter
      Y->SetIsChangedBySwitch(true);

      X->SetODEIndex(0);
      Y->SetODEIndex(1);
   }
};

TEST_F(when_creating_for_parameter_and_species_references, should_calculate_correct_value_without_simplifying)
{
   _formula.Finalize();

   double yy[2] = {_x, _y};
   double value = _formula.DE_Compute(yy, 0.0, USE_SCALEFACTOR);

   EXPECT_DOUBLE_EQ(_value, value);
}

TEST_F(when_creating_for_parameter_and_species_references, should_calculate_correct_value_with_simplifying)
{
   EXPECT_FALSE(_formula.Simplify(false));

   _formula.Finalize();

   double yy[2] = {_x, _y};
   double value = _formula.DE_Compute(yy, 0.0, USE_SCALEFACTOR);

   EXPECT_DOUBLE_EQ(_value, value);
}

class when_creating_for_parameter_species_and_time : public ::testing::Test
{
protected:
   double _p2 = 0, _x = 0, _y = 0;
   ExplicitFormulaExtender _formula;
   ExplicitFormulaExtender* _p1Formula = nullptr;

   // p1 = Time + x
   // p2 = const
   // f(p1, p2, x, y) = p1*x + p2*y
   void SetUp() override
   {
      _formula.SetEquation("p1*x+p2*y");

      _p2 = 3; _x = 4; _y = 5;

      SpeciesExtender* X = _formula.AddSpeciesReference("x", "-1");
      SpeciesExtender* Y = _formula.AddSpeciesReference("y", "-1");

      X->SetIsChangedBySwitch(true);
      Y->SetIsChangedBySwitch(true);

      X->SetODEIndex(0);
      Y->SetODEIndex(1);

      _p1Formula = new ExplicitFormulaExtender();
      _p1Formula->AddTimeReference();
      _p1Formula->AddSpeciesReference(X);
      _p1Formula->SetEquation(std::string(csTime) + "+x");

      _formula.AddParameterReference("p1", _p1Formula);
      _formula.AddParameterReference("p2", FormatNumber(_p2));
   }
};

TEST_F(when_creating_for_parameter_species_and_time, should_calculate_correct_value_without_simplifying)
{
   _p1Formula->Finalize();
   _formula.Finalize();

   double yy[2] = {_x, _y}, time = 17.5;

   double p1 = _p1Formula->DE_Compute(yy, time, USE_SCALEFACTOR);
   EXPECT_DOUBLE_EQ(time + _x, p1);

   double value = _formula.DE_Compute(yy, time, USE_SCALEFACTOR);
   EXPECT_DOUBLE_EQ(p1 * _x + _p2 * _y, value);
}

TEST_F(when_creating_for_parameter_species_and_time, should_calculate_correct_value_with_simplifying)
{
   EXPECT_FALSE(_p1Formula->Simplify(false));
   EXPECT_FALSE(_formula.Simplify(false));

   _p1Formula->Finalize();
   _formula.Finalize();

   double yy[2] = {_x, _y}, time = 17.5;

   double p1 = _p1Formula->DE_Compute(yy, time, USE_SCALEFACTOR);
   EXPECT_DOUBLE_EQ(time + _x, p1);

   double value = _formula.DE_Compute(yy, time, USE_SCALEFACTOR);
   EXPECT_DOUBLE_EQ(p1 * _x + _p2 * _y, value);
}

class when_getting_switch_timepoints : public ::testing::Test
{
protected:
   ExplicitFormulaExtender _formula;
   ExplicitFormulaExtender* _p1Formula = nullptr;
   ExplicitFormulaExtender* _p2Formula = nullptr;

   void SetUp() override
   {
      // Setup for returning time points 1,2,3,4,5,6,7
      _formula.SetEquation("((p1>=2) AND (Time>3)) OR ((4=Time) OR (Time !=2) OR (Time<>1)) AND NOT ((Time>=p1) OR (Time<p2) OR (Time<=5) OR (Time<=x))");
      _formula.AddTimeReference();

      SpeciesExtender* X = _formula.AddSpeciesReference("x", "-1");
      X->SetIsChangedBySwitch(true);

      _p1Formula = new ExplicitFormulaExtender();
      _p1Formula->SetEquation("6");
      _formula.AddParameterReference("p1", _p1Formula, true);

      _p2Formula = new ExplicitFormulaExtender();
      _p2Formula->SetEquation("7");
      _formula.AddParameterReference("p2", _p2Formula, true);
   }
};

TEST_F(when_getting_switch_timepoints, should_return_correct_switch_timepoints)
{
   _p1Formula->Simplify(false);
   _p2Formula->Simplify(false);
   _formula.Simplify(false);

   _p1Formula->Finalize();
   _p2Formula->Finalize();
   _formula.Finalize();

   std::vector<double> switchTimePoints = _formula.SwitchTimePoints();

   ASSERT_EQ((size_t)7, switchTimePoints.size());

   DoubleQueue dq;
   for (size_t i = 0; i < switchTimePoints.size(); i++)
      dq.push(switchTimePoints[i]);

   for (int i = 1; i <= 7; i++)
   {
      EXPECT_DOUBLE_EQ((double)i, dq.top());
      dq.pop();
   }
}
