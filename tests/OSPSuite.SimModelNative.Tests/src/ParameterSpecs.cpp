#include <gtest/gtest.h>
#include "SimModel/Parameter.h"
#include "SimModel/ExplicitFormula.h"
#include "SimModel/TableFormula.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/Formula.h"
#include <ErrorData.h>
#include <vector>

using namespace SimModelNative;

class when_setting_table_points_into_nontable_parameter : public ::testing::Test
{
protected:
   Parameter _parameter;

   void SetUp() override
   {
      _parameter.SetFormula(new ExplicitFormula());

      std::vector<ValuePoint> valuePoints;
      valuePoints.push_back(ValuePoint(0, 0, false));
      valuePoints.push_back(ValuePoint(1, 2, false));
      valuePoints.push_back(ValuePoint(5, 3, true));
      valuePoints.push_back(ValuePoint(20, 6, false));
      valuePoints.push_back(ValuePoint(40, 6, false));
      valuePoints.push_back(ValuePoint(41, 8, false));

      _parameter.SetTablePoints(valuePoints);
   }

   double Calculate(double time)
   {
      return _parameter.GetValue(nullptr, time, USE_SCALEFACTOR);
   }
};

TEST_F(when_setting_table_points_into_nontable_parameter, parameter_should_be_table)
{
   EXPECT_TRUE(_parameter.IsTable());
}

TEST_F(when_setting_table_points_into_nontable_parameter, should_calculate_correct_value)
{
   EXPECT_DOUBLE_EQ(0.0, Calculate(-1));
   EXPECT_DOUBLE_EQ(0.0, Calculate(0));
   EXPECT_DOUBLE_EQ(1.0, Calculate(0.5));
   EXPECT_DOUBLE_EQ(2.0, Calculate(1));
   EXPECT_DOUBLE_EQ(2.5, Calculate(3));
   EXPECT_DOUBLE_EQ(3.0, Calculate(5));
   EXPECT_DOUBLE_EQ(4.0, Calculate(10));
   EXPECT_DOUBLE_EQ(6.0, Calculate(20));
   EXPECT_DOUBLE_EQ(6.0, Calculate(30));
   EXPECT_DOUBLE_EQ(6.0, Calculate(40));
   EXPECT_DOUBLE_EQ(7.0, Calculate(40.5));
   EXPECT_DOUBLE_EQ(8.0, Calculate(42));
}

class when_setting_initial_value_into_table_parameter : public ::testing::Test
{
protected:
   static constexpr double _initialValue = 999;

   Parameter _parameter;

   void SetUp() override
   {
      _parameter.SetFormula(new TableFormula());

      std::vector<ValuePoint> valuePoints;
      valuePoints.push_back(ValuePoint(0, 0, false));
      valuePoints.push_back(ValuePoint(1, 2, false));

      _parameter.SetTablePoints(valuePoints);

      _parameter.SetInitialValue(_initialValue);
   }

   double Calculate(double time)
   {
      return _parameter.GetValue(nullptr, time, USE_SCALEFACTOR);
   }
};

TEST_F(when_setting_initial_value_into_table_parameter, parameter_should_not_be_table)
{
   EXPECT_FALSE(_parameter.IsTable());
}

TEST_F(when_setting_initial_value_into_table_parameter, should_calculate_correct_value)
{
   EXPECT_DOUBLE_EQ(_initialValue, Calculate(-1));
   EXPECT_DOUBLE_EQ(_initialValue, Calculate(0));
   EXPECT_DOUBLE_EQ(_initialValue, Calculate(2));
}
