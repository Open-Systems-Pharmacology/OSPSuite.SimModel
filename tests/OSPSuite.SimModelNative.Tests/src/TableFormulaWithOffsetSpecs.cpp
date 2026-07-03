#include <gtest/gtest.h>
#include "SimModel/TableFormula.h"
#include "SimModel/TableFormulaWithOffset.h"
#include "SimModel/Parameter.h"
#include "SimModel/Quantity.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/TObjectVector.h"
#include "TableFormulaTestExtender.h"
#include <ErrorData.h>
#include <vector>

using namespace SimModelNative;

// Provide access to protected members of TableFormulaWithOffset via public inheritance.
class TableFormulaWithOffsetExtender : public TableFormulaWithOffset
{
public:
   void SetTableObject(Quantity* tableObject) { _tableObject = tableObject; }
   void SetOffsetObject(Quantity* offsetObject) { _offsetObject = offsetObject; }
   void SetTableFormula(TableFormula* tableFormula) { _tableFormula = tableFormula; }
};

class when_creating_for_given_table_with_offset : public ::testing::Test
{
protected:
   static constexpr double _offset = 300.0;

   TableFormulaWithOffsetExtender _formula;
   TableFormulaTestExtender _tableFormula;
   Parameter _offsetObject;
   Parameter _tableObject;

   void SetUp() override
   {
      TObjectVector<ValuePoint>& valuePoints = _tableFormula.ValuePoints();
      valuePoints.push_back(new ValuePoint(0, 0, false));
      valuePoints.push_back(new ValuePoint(1, 2, false));
      valuePoints.push_back(new ValuePoint(5, 3, true));
      valuePoints.push_back(new ValuePoint(20, 6, false));
      valuePoints.push_back(new ValuePoint(40, 6, false));
      valuePoints.push_back(new ValuePoint(41, 8, false));

      _tableFormula.CallCacheValues();

      _offsetObject.SetConstantValue(_offset);
      _formula.SetOffsetObject(&_offsetObject);
      _formula.SetTableFormula(&_tableFormula);

      _tableObject.SetFormula(&_tableFormula);
      _formula.SetTableObject(&_tableObject);
   }

   double Calculate(double x)
   {
      return _formula.DE_Compute(nullptr, x, USE_SCALEFACTOR);
   }
};

TEST_F(when_creating_for_given_table_with_offset, should_calculate_correct_value)
{
   EXPECT_DOUBLE_EQ(0.0, Calculate(-1 + _offset));
   EXPECT_DOUBLE_EQ(2.0, Calculate(0 + _offset));
   EXPECT_DOUBLE_EQ(2.0, Calculate(0.5 + _offset));
   EXPECT_DOUBLE_EQ(0.25, Calculate(1 + _offset));
   EXPECT_DOUBLE_EQ(0.25, Calculate(3 + _offset));
   EXPECT_DOUBLE_EQ(0.2, Calculate(5 + _offset));
   EXPECT_DOUBLE_EQ(0.2, Calculate(10 + _offset));
   EXPECT_DOUBLE_EQ(0.0, Calculate(20 + _offset));
   EXPECT_DOUBLE_EQ(0.0, Calculate(30 + _offset));
   EXPECT_DOUBLE_EQ(2.0, Calculate(40 + _offset));
   EXPECT_DOUBLE_EQ(2.0, Calculate(40.5 + _offset));
   EXPECT_DOUBLE_EQ(0.0, Calculate(42 + _offset));
}

TEST_F(when_creating_for_given_table_with_offset, should_set_restart_timepoints)
{
   std::vector<double> restartTimePoints = _formula.RestartTimePoints();

   ASSERT_EQ(3u, restartTimePoints.size());
   EXPECT_DOUBLE_EQ(0.0 + _offset, restartTimePoints[0]);
   EXPECT_DOUBLE_EQ(5.0 + _offset, restartTimePoints[1]);
   EXPECT_DOUBLE_EQ(40.0 + _offset, restartTimePoints[2]);
}
