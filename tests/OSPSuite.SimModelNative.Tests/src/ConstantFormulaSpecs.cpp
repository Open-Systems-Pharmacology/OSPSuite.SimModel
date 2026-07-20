#include <gtest/gtest.h>
#include "SimModel/ConstantFormula.h"
#include "SimModel/MathHelper.h"

using namespace SimModelNative;

TEST(when_creating_a_constant_formula_with_default_constructor, should_be_zero_initialized)
{
   ConstantFormula formula;
   EXPECT_DOUBLE_EQ(0.0, formula.DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));
}

TEST(when_creating_a_constant_formula_with_a_value, should_store_and_return_the_value)
{
   ConstantFormula formula(3.14);
   EXPECT_DOUBLE_EQ(3.14, formula.DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));
}

TEST(when_computing_the_value_of_a_constant_formula, should_ignore_time_and_y_vector)
{
   ConstantFormula formula(42.0);
   double y[] = { 1.0, 2.0, 3.0 };

   EXPECT_DOUBLE_EQ(42.0, formula.DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));
   EXPECT_DOUBLE_EQ(42.0, formula.DE_Compute(y, 100.0, IGNORE_SCALEFACTOR));
   EXPECT_DOUBLE_EQ(42.0, formula.DE_Compute(y, -5.0, USE_SCALEFACTOR));
}

TEST(when_checking_if_a_constant_formula_is_zero, should_return_true_for_zero_value)
{
   ConstantFormula formula(0.0);
   EXPECT_TRUE(formula.IsZero());
}

TEST(when_checking_if_a_constant_formula_is_non_zero, should_return_false_for_non_zero_value)
{
   ConstantFormula formula(1.0);
   EXPECT_FALSE(formula.IsZero());

   ConstantFormula negFormula(-0.001);
   EXPECT_FALSE(negFormula.IsZero());
}

TEST(when_checking_if_a_constant_formula_is_constant, should_always_return_true)
{
   ConstantFormula formula(5.0);
   EXPECT_TRUE(formula.IsConstant(true));
   EXPECT_TRUE(formula.IsConstant(false));
}

TEST(when_setting_a_new_value_on_a_constant_formula, should_change_the_computed_value)
{
   ConstantFormula formula(1.0);
   EXPECT_DOUBLE_EQ(1.0, formula.DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));

   formula.SetValue(99.0);
   EXPECT_DOUBLE_EQ(99.0, formula.DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));
}

TEST(when_cloning_a_constant_formula, should_create_an_independent_copy)
{
   ConstantFormula original(7.5);
   Formula* cloned = original.clone();

   EXPECT_DOUBLE_EQ(7.5, cloned->DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));

   original.SetValue(0.0);
   EXPECT_DOUBLE_EQ(7.5, cloned->DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));

   delete cloned;
}

TEST(when_computing_the_jacobian_of_a_constant_formula, should_return_zero_constant)
{
   ConstantFormula formula(42.0);
   Formula* jacobian = formula.DE_Jacobian(0);

   EXPECT_DOUBLE_EQ(0.0, jacobian->DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));

   delete jacobian;
}

TEST(when_getting_switch_time_points_of_a_constant_formula, should_return_an_empty_vector)
{
   ConstantFormula formula(1.0);
   std::vector<double> points = formula.SwitchTimePoints();
   EXPECT_TRUE(points.empty());
}

TEST(when_creating_a_constant_formula_with_a_negative_value, should_compute_correct_value_and_not_be_zero)
{
   ConstantFormula formula(-100.0);
   EXPECT_DOUBLE_EQ(-100.0, formula.DE_Compute(nullptr, 0.0, IGNORE_SCALEFACTOR));
   EXPECT_FALSE(formula.IsZero());
}

TEST(when_recursively_simplifying_a_constant_formula, should_return_itself)
{
   ConstantFormula formula(5.0);
   Formula* simplified = formula.RecursiveSimplify();
   EXPECT_EQ(&formula, simplified);
}
