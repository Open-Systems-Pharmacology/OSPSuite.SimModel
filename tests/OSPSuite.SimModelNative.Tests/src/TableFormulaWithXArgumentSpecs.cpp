#include <gtest/gtest.h>
#include "SimModel/TableFormula.h"
#include "SimModel/TableFormulaWithXArgument.h"
#include "SimModel/Parameter.h"
#include "SimModel/Quantity.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/TObjectVector.h"
#include "TableFormulaTestExtender.h"
#include <ErrorData.h>
#include <vector>

using namespace SimModelNative;

// Provide access to protected members of TableFormulaWithXArgument via public inheritance.
class TableFormulaWithXArgumentExtender : public TableFormulaWithXArgument
{
public:
   void SetTableObject(Quantity* tableObject) { _tableObject = tableObject; }
   void SetXArgumentObject(Quantity* xArgumentObject) { _XArgumentObject = xArgumentObject; }
   void SetTableFormula(TableFormula* tableFormula) { _tableFormula = tableFormula; }
   TableFormula* GetTableFormula() { return _tableFormula; }
};

class when_creating_for_given_table_with_non_time_dependent_xargument : public ::testing::Test
{
protected:
   TableFormulaWithXArgumentExtender _formula;
   TableFormulaTestExtender _tableFormula;
   Parameter _xArgumentObject;
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

      _formula.SetXArgumentObject(&_xArgumentObject);
      _formula.SetTableFormula(&_tableFormula);

      _tableObject.SetFormula(&_tableFormula);
      _formula.SetTableObject(&_tableObject);
   }

   void UseDerivedValues(bool useDerivedValues)
   {
      _formula.GetTableFormula()->SetUseDerivedValues(useDerivedValues);
   }

   void CheckValue(double xArgument, double expectedValue)
   {
      _xArgumentObject.SetConstantValue(xArgument);

      const double dummyTime = 0.0; // time does not matter for an x argument which is not time dependent
      EXPECT_DOUBLE_EQ(expectedValue, _formula.DE_Compute(nullptr, dummyTime, USE_SCALEFACTOR));
   }
};

TEST_F(when_creating_for_given_table_with_non_time_dependent_xargument, should_calculate_correct_values_in_direct_mode)
{
   UseDerivedValues(false);

   CheckValue(-1, 0.0);
   CheckValue(0, 0.0);
   CheckValue(0.5, 1.0);
   CheckValue(1, 2.0);
   CheckValue(3, 2.5);
   CheckValue(20, 6.0);
   CheckValue(30, 6.0);
   CheckValue(40.5, 7.0);
   CheckValue(41, 8.0);
   CheckValue(50, 8.0);
}

TEST_F(when_creating_for_given_table_with_non_time_dependent_xargument, should_calculate_correct_values_in_derived_mode)
{
   UseDerivedValues(true);
   _tableFormula.CallCacheValues();

   CheckValue(-1, 0.0);
   CheckValue(0, 2.0);
   CheckValue(0.5, 2.0);
   CheckValue(1, 0.25);
   CheckValue(3, 0.25);
   CheckValue(20, 0.0);
   CheckValue(30, 0.0);
   CheckValue(40.5, 2.0);
   CheckValue(41, 0.0);
   CheckValue(50, 0.0);
}

TEST_F(when_creating_for_given_table_with_non_time_dependent_xargument, should_return_empty_restart_timepoints_set)
{
   std::vector<double> restartTimePoints = _formula.RestartTimePoints();
   EXPECT_TRUE(restartTimePoints.empty());
}
