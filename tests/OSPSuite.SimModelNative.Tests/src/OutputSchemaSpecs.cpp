#include <gtest/gtest.h>
#include "SimModel/OutputSchema.h"
#include "SimModel/SimModelTypeDefs.h"
#include "SimModel/TObjectVector.h"
#include <ErrorData.h>

using namespace SimModelNative;

// --- OutputTimePoint tests ---

TEST(when_creating_an_output_time_point_with_all_values, should_store_them_correctly)
{
   OutputTimePoint tp(1.5, true, false, true);
   EXPECT_DOUBLE_EQ(1.5, tp.Time());
   EXPECT_TRUE(tp.SaveSystemSolution());
   EXPECT_FALSE(tp.IsSwitchTimePoint());
   EXPECT_TRUE(tp.RestartSystem());
}

TEST(when_creating_an_output_time_point_with_all_flags_false, should_store_false_for_all_flags)
{
   OutputTimePoint tp(0.0, false, false, false);
   EXPECT_DOUBLE_EQ(0.0, tp.Time());
   EXPECT_FALSE(tp.SaveSystemSolution());
   EXPECT_FALSE(tp.IsSwitchTimePoint());
   EXPECT_FALSE(tp.RestartSystem());
}

TEST(when_creating_an_output_time_point_with_all_flags_true, should_store_true_for_all_flags)
{
   OutputTimePoint tp(100.0, true, true, true);
   EXPECT_TRUE(tp.SaveSystemSolution());
   EXPECT_TRUE(tp.IsSwitchTimePoint());
   EXPECT_TRUE(tp.RestartSystem());
}

// --- OutputInterval tests ---

TEST(when_creating_an_output_interval_with_default_constructor, should_set_zeros_and_equidistant_distribution)
{
   OutputInterval interval;
   EXPECT_DOUBLE_EQ(0.0, interval.StartTime());
   EXPECT_DOUBLE_EQ(0.0, interval.EndTime());
   EXPECT_EQ(0, interval.NumberOfTimePoints());
   EXPECT_EQ(Equidistant, interval.IntervalDistribution());
}

TEST(when_creating_an_output_interval_with_parameters, should_store_all_provided_values)
{
   OutputInterval interval(1.0, 10.0, 5, Equidistant);
   EXPECT_DOUBLE_EQ(1.0, interval.StartTime());
   EXPECT_DOUBLE_EQ(10.0, interval.EndTime());
   EXPECT_EQ(5, interval.NumberOfTimePoints());
   EXPECT_EQ(Equidistant, interval.IntervalDistribution());
}

TEST(when_setting_properties_on_an_output_interval, should_return_the_updated_values)
{
   OutputInterval interval;
   interval.SetStartTime(2.0);
   interval.SetEndTime(8.0);
   interval.NumberOfTimePoints(10);
   interval.SetOutputIntervalDistribution(Logarithmic);

   EXPECT_DOUBLE_EQ(2.0, interval.StartTime());
   EXPECT_DOUBLE_EQ(8.0, interval.EndTime());
   EXPECT_EQ(10, interval.NumberOfTimePoints());
   EXPECT_EQ(Logarithmic, interval.IntervalDistribution());
}

TEST(when_getting_time_points_for_an_equidistant_interval, should_return_evenly_spaced_points)
{
   OutputInterval interval(0.0, 10.0, 6, Equidistant);
   std::vector<double> points = interval.TimePoints();

   ASSERT_EQ(6u, points.size());
   EXPECT_DOUBLE_EQ(0.0, points[0]);
   EXPECT_DOUBLE_EQ(2.0, points[1]);
   EXPECT_DOUBLE_EQ(4.0, points[2]);
   EXPECT_DOUBLE_EQ(6.0, points[3]);
   EXPECT_DOUBLE_EQ(8.0, points[4]);
   EXPECT_DOUBLE_EQ(10.0, points[5]);
}

TEST(when_getting_time_points_for_an_interval_with_two_points, should_return_start_and_end_values)
{
   OutputInterval interval(0.0, 100.0, 2, Equidistant);
   std::vector<double> points = interval.TimePoints();

   ASSERT_EQ(2u, points.size());
   EXPECT_DOUBLE_EQ(0.0, points[0]);
   EXPECT_DOUBLE_EQ(100.0, points[1]);
}

TEST(when_getting_time_points_for_an_interval_where_start_equals_end, should_return_a_single_point)
{
   OutputInterval interval(5.0, 5.0, 1, Equidistant);
   std::vector<double> points = interval.TimePoints();

   ASSERT_EQ(1u, points.size());
   EXPECT_DOUBLE_EQ(5.0, points[0]);
}

TEST(when_getting_time_points_for_an_interval_with_start_greater_than_end, should_throw_error_data)
{
   OutputInterval interval(10.0, 5.0, 3, Equidistant);
   EXPECT_THROW(interval.TimePoints(), ErrorData);
}

TEST(when_getting_time_points_for_an_interval_with_less_than_two_points, should_throw_error_data)
{
   OutputInterval interval(0.0, 10.0, 1, Equidistant);
   EXPECT_THROW(interval.TimePoints(), ErrorData);
}

TEST(when_getting_time_points_for_a_logarithmic_interval, should_throw_error_data)
{
   OutputInterval interval(1.0, 10.0, 5, Logarithmic);
   EXPECT_THROW(interval.TimePoints(), ErrorData);
}

TEST(when_getting_time_points_for_an_equidistant_interval_with_three_points, should_return_correct_midpoint)
{
   OutputInterval interval(0.0, 1.0, 3, Equidistant);
   std::vector<double> points = interval.TimePoints();

   ASSERT_EQ(3u, points.size());
   EXPECT_DOUBLE_EQ(0.0, points[0]);
   EXPECT_DOUBLE_EQ(0.5, points[1]);
   EXPECT_DOUBLE_EQ(1.0, points[2]);
}

// --- OutputSchema::AllTimePoints (migrated from the original C++/CLI specs) ---

TEST(when_retrieving_points_from_output_schema_with_overlapping_intervals, should_retrieve_all_unique_points_in_increasing_order)
{
   OutputSchema schema;

   TObjectVector<OutputInterval>& intervals = schema.OutputIntervals();
   intervals.push_back(new OutputInterval(10, 20, 3, Equidistant)); // 10, 15, 20
   intervals.push_back(new OutputInterval(20, 30, 3, Equidistant)); // 20, 25, 30
   intervals.push_back(new OutputInterval(0, 10, 3, Equidistant));  // 0, 5, 10

   DoubleQueue timePointsQueue = schema.AllTimePoints<float>();

   // should return [0 5 10 15 20 25 30]
   ASSERT_EQ((size_t)7, timePointsQueue.size());
   for (int i = 0; i <= 6; i++)
   {
      EXPECT_DOUBLE_EQ(5.0 * i, timePointsQueue.top());
      timePointsQueue.pop();
   }
}
