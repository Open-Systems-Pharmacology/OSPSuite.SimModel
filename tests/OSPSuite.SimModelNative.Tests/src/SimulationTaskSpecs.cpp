#include <gtest/gtest.h>
#include "SimModel/SimulationTask.h"
#include "SimModel/OutputSchema.h"
#include "SimModel/SimModelTypeDefs.h"
#include <vector>

using namespace SimModelNative;

// Provide access to the protected static OutputTimePoints overload via public inheritance.
class SimulationTaskExtender : public SimulationTask
{
public:
   static std::vector<OutputTimePoint> OutputTimePoints(const DoubleQueue& userOutputTimePoints,
                                                        const DoubleQueue& switchTimePoints,
                                                        const DoubleQueue& tableFormulaRestartTimePoints)
   {
      return SimulationTask::OutputTimePoints(userOutputTimePoints,
                                              switchTimePoints,
                                              tableFormulaRestartTimePoints);
   }
};

class when_creating_timepoint_list : public ::testing::Test
{
protected:

   void CheckTimePoints(const std::vector<OutputTimePoint>& outputTimePoints, unsigned int size,
                        const double times[], const bool saveSolution[],
                        const bool isSwitchTimePoint[], const bool restartNeeded[]) const
   {
      ASSERT_EQ((size_t)size, outputTimePoints.size());

      for (unsigned int i = 0; i < outputTimePoints.size(); i++)
      {
         EXPECT_DOUBLE_EQ(times[i], outputTimePoints[i].Time());
         EXPECT_EQ(saveSolution[i], outputTimePoints[i].SaveSystemSolution());
         EXPECT_EQ(isSwitchTimePoint[i], outputTimePoints[i].IsSwitchTimePoint());
         EXPECT_EQ(restartNeeded[i], outputTimePoints[i].RestartSystem());
      }
   }
};

TEST_F(when_creating_timepoint_list, should_return_usertimepoints_for_empty_switch_timepoints)
{
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(10);
   userOutputTimePoints.push(30);
   userOutputTimePoints.push(20);

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints);

   ASSERT_EQ(userOutputTimePoints.size(), outputTimePoints.size());

   for (auto outputTimePoint : outputTimePoints)
   {
      EXPECT_DOUBLE_EQ(userOutputTimePoints.top(), outputTimePoint.Time());
      EXPECT_TRUE(outputTimePoint.SaveSystemSolution());
      EXPECT_FALSE(outputTimePoint.IsSwitchTimePoint());

      userOutputTimePoints.pop();
   }
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_1)
{
   // first relevant switch point after first relevant user timepoint
   // last relevant switch point before last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(- 5);
   userOutputTimePoints.push(10);
   userOutputTimePoints.push(30);
   userOutputTimePoints.push(20);

   switchTimePoints.push(- 3);
   switchTimePoints.push(15);
   switchTimePoints.push(25);

   tableFormulaRestartTimePoints.push(28);
   tableFormulaRestartTimePoints.push(25);

   double times[] =             {-3, 10, 15, 20, 25, 28, 30};
   bool saveSolution[] =        { false,  true,     false,    true,     false,    false,    true};
   bool isSwitchTimePoint[] =   { true,   false,    true,     false,    true,     true,     false};
   bool restart[] =             { false,  false,    false,    false,    true,     true,     false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints);

   CheckTimePoints(outputTimePoints, 7, times, saveSolution, isSwitchTimePoint, restart);

   EXPECT_EQ(3, SimulationTask::NumberOfSimulatedTimeSteps(outputTimePoints)); //ignore negative output time point
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_2)
{
   // first relevant switch point = first relevant user timepoint
   // last relevant switch point before last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(- 5);
   userOutputTimePoints.push(10);
   userOutputTimePoints.push(30);
   userOutputTimePoints.push(20);

   switchTimePoints.push(- 3);
   switchTimePoints.push(10);
   switchTimePoints.push(25);

   double times[] =             {-3, 10, 20, 25, 30};
   bool saveSolution[] =        {false,  true,     true,     false,    true};
   bool isSwitchTimePoint[] =   {true,   true,     false,    true,     false};
   bool restart[] =             {false,  false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints);

   CheckTimePoints(outputTimePoints, 5, times, saveSolution, isSwitchTimePoint, restart);
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_3)
{
   // first relevant switch point before first relevant user timepoint
   // last relevant switch point before last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(- 5);
   userOutputTimePoints.push(10);
   userOutputTimePoints.push(30);
   userOutputTimePoints.push(20);

   switchTimePoints.push(- 3);
   switchTimePoints.push(5);
   switchTimePoints.push(25);

   double times[] =             {-3, 5, 10, 20, 25, 30};
   bool saveSolution[] =        {false,  false,   true,     true,     false,    true};
   bool isSwitchTimePoint[] =   {true,   true,    false,    false,    true,     false};
   bool restart[] =             {false,  false,   false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints);

   CheckTimePoints(outputTimePoints, 6, times, saveSolution, isSwitchTimePoint, restart);
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_4)
{
   // first relevant switch point after first relevant user timepoint
   // last relevant switch point = last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(- 5);
   userOutputTimePoints.push(10);
   userOutputTimePoints.push(30);
   userOutputTimePoints.push(20);

   switchTimePoints.push(- 3);
   switchTimePoints.push(15);
   switchTimePoints.push(30);

   double times[] =             {-3, 10, 15, 20, 30};
   bool saveSolution[] =        {false,  true,     false,    true,     true};
   bool isSwitchTimePoint[] =   {true,   false,    true,     false,    true};
   bool restart[] =             {false,  false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints);

   CheckTimePoints(outputTimePoints, 5, times, saveSolution, isSwitchTimePoint, restart);
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_5)
{
   // first relevant switch point after first relevant user timepoint
   // last relevant switch point after last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(- 5);
   userOutputTimePoints.push(10);
   userOutputTimePoints.push(30);
   userOutputTimePoints.push(20);

   switchTimePoints.push(- 3);
   switchTimePoints.push(15);
   switchTimePoints.push(35);

   double times[] =             {-3, 10, 15, 20, 30};
   bool saveSolution[] =        {false,  true,     false,    true,     true};
   bool isSwitchTimePoint[] =   {true,   false,    true,     false,    false};
   bool restart[] =             {false,  false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints);

   CheckTimePoints(outputTimePoints, 5, times, saveSolution, isSwitchTimePoint, restart);
}
