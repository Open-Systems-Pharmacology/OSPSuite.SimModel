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
   static std::vector<OutputTimePoint> OutputTimePoints(DoubleQueue userOutputTimePoints,
                                                        DoubleQueue switchTimePoints,
                                                        DoubleQueue tableFormulaRestartTimePoints,
                                                        double simulationStartTime)
   {
      return SimulationTask::OutputTimePoints(userOutputTimePoints,
                                              switchTimePoints,
                                              tableFormulaRestartTimePoints,
                                              simulationStartTime);
   }
};

class when_creating_timepoint_list : public ::testing::Test
{
protected:
   double _t0 = 0.0;

   void CheckTimePoints(const std::vector<OutputTimePoint>& outputTimePoints, unsigned int size,
                        const double times[], const bool saveSolution[],
                        const bool isSwitchTimePoint[], const bool restartNeeded[])
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
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);

   ASSERT_EQ(userOutputTimePoints.size(), outputTimePoints.size());

   for (unsigned int i = 0; i < outputTimePoints.size(); i++)
   {
      EXPECT_DOUBLE_EQ(userOutputTimePoints.top(), outputTimePoints[i].Time());
      EXPECT_TRUE(outputTimePoints[i].SaveSystemSolution());
      EXPECT_FALSE(outputTimePoints[i].IsSwitchTimePoint());

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

   userOutputTimePoints.push(_t0 - 5);
   userOutputTimePoints.push(_t0 + 10);
   userOutputTimePoints.push(_t0 + 30);
   userOutputTimePoints.push(_t0 + 20);

   switchTimePoints.push(_t0 - 3);
   switchTimePoints.push(_t0 + 15);
   switchTimePoints.push(_t0 + 25);

   tableFormulaRestartTimePoints.push(_t0 + 28);
   tableFormulaRestartTimePoints.push(_t0 + 25);

   double times[] =             {_t0 + 10, _t0 + 15, _t0 + 20, _t0 + 25, _t0 + 28, _t0 + 30};
   bool saveSolution[] =        {true,     false,    true,     false,    false,    true};
   bool isSwitchTimePoint[] =   {false,    true,     false,    true,     true,     false};
   bool restart[] =             {false,    false,    false,    true,     true,     false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);

   CheckTimePoints(outputTimePoints, 6, times, saveSolution, isSwitchTimePoint, restart);

   EXPECT_EQ(3, SimulationTask::NumberOfSimulatedTimeSteps(outputTimePoints));
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_2)
{
   // first relevant switch point = first relevant user timepoint
   // last relevant switch point before last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(_t0 - 5);
   userOutputTimePoints.push(_t0 + 10);
   userOutputTimePoints.push(_t0 + 30);
   userOutputTimePoints.push(_t0 + 20);

   switchTimePoints.push(_t0 - 3);
   switchTimePoints.push(_t0 + 10);
   switchTimePoints.push(_t0 + 25);

   double times[] =             {_t0 + 10, _t0 + 20, _t0 + 25, _t0 + 30};
   bool saveSolution[] =        {true,     true,     false,    true};
   bool isSwitchTimePoint[] =   {true,     false,    true,     false};
   bool restart[] =             {false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);

   CheckTimePoints(outputTimePoints, 4, times, saveSolution, isSwitchTimePoint, restart);
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_3)
{
   // first relevant switch point before first relevant user timepoint
   // last relevant switch point before last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(_t0 - 5);
   userOutputTimePoints.push(_t0 + 10);
   userOutputTimePoints.push(_t0 + 30);
   userOutputTimePoints.push(_t0 + 20);

   switchTimePoints.push(_t0 - 3);
   switchTimePoints.push(_t0 + 5);
   switchTimePoints.push(_t0 + 25);

   double times[] =             {_t0 + 5, _t0 + 10, _t0 + 20, _t0 + 25, _t0 + 30};
   bool saveSolution[] =        {false,   true,     true,     false,    true};
   bool isSwitchTimePoint[] =   {true,    false,    false,    true,     false};
   bool restart[] =             {false,   false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);

   CheckTimePoints(outputTimePoints, 5, times, saveSolution, isSwitchTimePoint, restart);
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_4)
{
   // first relevant switch point after first relevant user timepoint
   // last relevant switch point = last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(_t0 - 5);
   userOutputTimePoints.push(_t0 + 10);
   userOutputTimePoints.push(_t0 + 30);
   userOutputTimePoints.push(_t0 + 20);

   switchTimePoints.push(_t0 - 3);
   switchTimePoints.push(_t0 + 15);
   switchTimePoints.push(_t0 + 30);

   double times[] =             {_t0 + 10, _t0 + 15, _t0 + 20, _t0 + 30};
   bool saveSolution[] =        {true,     false,    true,     true};
   bool isSwitchTimePoint[] =   {false,    true,     false,    true};
   bool restart[] =             {false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);

   CheckTimePoints(outputTimePoints, 4, times, saveSolution, isSwitchTimePoint, restart);
}

TEST_F(when_creating_timepoint_list, should_return_correct_timepoints_5)
{
   // first relevant switch point after first relevant user timepoint
   // last relevant switch point after last relevant user timepoint
   DoubleQueue userOutputTimePoints;
   DoubleQueue switchTimePoints;
   DoubleQueue tableFormulaRestartTimePoints;

   userOutputTimePoints.push(_t0 - 5);
   userOutputTimePoints.push(_t0 + 10);
   userOutputTimePoints.push(_t0 + 30);
   userOutputTimePoints.push(_t0 + 20);

   switchTimePoints.push(_t0 - 3);
   switchTimePoints.push(_t0 + 15);
   switchTimePoints.push(_t0 + 35);

   double times[] =             {_t0 + 10, _t0 + 15, _t0 + 20, _t0 + 30};
   bool saveSolution[] =        {true,     false,    true,     true};
   bool isSwitchTimePoint[] =   {false,    true,     false,    false};
   bool restart[] =             {false,    false,    false,    false};

   std::vector<OutputTimePoint> outputTimePoints =
      SimulationTaskExtender::OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);

   CheckTimePoints(outputTimePoints, 4, times, saveSolution, isSwitchTimePoint, restart);
}
