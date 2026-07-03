#include <gtest/gtest.h>
#include "SimModel/TableFormula.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/TObjectVector.h"
#include "TableFormulaTestExtender.h"
#include <ErrorData.h>
#include <vector>

using namespace SimModelNative;

class when_creating_for_given_table : public ::testing::Test
{
protected:
   TableFormulaTestExtender _formula;

   void SetUp() override
   {
      TObjectVector<ValuePoint>& valuePoints = _formula.ValuePoints();
      valuePoints.push_back(new ValuePoint(0, 0, false));
      valuePoints.push_back(new ValuePoint(1, 2, false));
      valuePoints.push_back(new ValuePoint(5, 3, true));
      valuePoints.push_back(new ValuePoint(20, 6, false));
      valuePoints.push_back(new ValuePoint(40, 6, false));
      valuePoints.push_back(new ValuePoint(41, 8, false));

      _formula.CallCacheValues();
   }

   double Calculate(double x)
   {
      return _formula.DE_Compute(nullptr, x, USE_SCALEFACTOR);
   }
};

TEST_F(when_creating_for_given_table, should_calculate_correct_value)
{
   EXPECT_DOUBLE_EQ(0.0, Calculate(-1));
   EXPECT_DOUBLE_EQ(2.0, Calculate(0));
   EXPECT_DOUBLE_EQ(2.0, Calculate(0.5));
   EXPECT_DOUBLE_EQ(0.25, Calculate(1));
   EXPECT_DOUBLE_EQ(0.25, Calculate(3));
   EXPECT_DOUBLE_EQ(0.2, Calculate(5));
   EXPECT_DOUBLE_EQ(0.2, Calculate(10));
   EXPECT_DOUBLE_EQ(0.0, Calculate(20));
   EXPECT_DOUBLE_EQ(0.0, Calculate(30));
   EXPECT_DOUBLE_EQ(2.0, Calculate(40));
   EXPECT_DOUBLE_EQ(2.0, Calculate(40.5));
   EXPECT_DOUBLE_EQ(0.0, Calculate(42));
}

TEST_F(when_creating_for_given_table, should_set_restart_timepoints)
{
   std::vector<double> restartTimePoints = _formula.RestartTimePoints();

   ASSERT_EQ(3u, restartTimePoints.size());
   EXPECT_DOUBLE_EQ(0.0, restartTimePoints[0]);
   EXPECT_DOUBLE_EQ(5.0, restartTimePoints[1]);
   EXPECT_DOUBLE_EQ(40.0, restartTimePoints[2]);
}
