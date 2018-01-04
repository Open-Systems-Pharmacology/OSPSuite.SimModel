#ifdef WIN32
#pragma warning( disable : 4691)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelSpecs/SpecsHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/OutputSchema.h"
#include "SimModel/TObjectVector.h"
#include "SimModel/SimulationTask.h"

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace SimModelNET;

	using namespace SimModelNative;
	using namespace std;

	//provide access to protected functions via public inheritance
	class SimulationTaskExtender : public SimulationTask
	{
	public:
		static std::vector <OutputTimePoint> OutputTimePoints(DoubleQueue & userOutputTimePoints, 
															  DoubleQueue & switchTimePoints,
															  DoubleQueue & tableFormulaRestartTimePoints,
															  double simulationStartTime)
		{
			return SimulationTask::OutputTimePoints(userOutputTimePoints, 
											        switchTimePoints,
													tableFormulaRestartTimePoints,
											        simulationStartTime);
		}
	};

	ref class SimulationTaskWrapper
	{
	public:
		SimulationTaskExtender * SimulationTask;
		SimulationTaskWrapper(){SimulationTask=new SimulationTaskExtender();}
		~SimulationTaskWrapper(){delete SimulationTask;}
	};

	public ref class concern_for_simulation_task abstract : ContextSpecification<SimulationTaskWrapper^>
    {
    public:
		virtual void GlobalContext() override
		{
			try
			{
				_t0 = 0.0;
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}

	protected:
		double _t0;

        virtual void Context() override
        {
			try
			{
				sut=gcnew SimulationTaskWrapper();
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
        }
    };

	public ref class when_creating_timepoint_list : public concern_for_simulation_task
    {
	protected:
		void CheckTimePoints(vector <OutputTimePoint> & outputTimePoints, unsigned int size,
			                 double times[], bool   saveSolution[],
							 bool isSwitchTimePoint[], bool restartNeeded[])
		{
			BDDExtensions::ShouldBeEqualTo(outputTimePoints.size(), (size_t)size);

			for(unsigned int i=0; i<outputTimePoints.size(); i++)
			{
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].Time(), times[i]);
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].SaveSystemSolution(), saveSolution[i]);
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].IsSwitchTimePoint(), isSwitchTimePoint[i]);
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].RestartSystem(), restartNeeded[i]);
			}
		}
			                 
	public:
 
		[TestAttribute]   
		void should_return_usertimepoints_for_empty_switch_timepoints()
		{
			DoubleQueue userOutputTimePoints;
			DoubleQueue switchTimePoints;
			DoubleQueue tableFormulaRestartTimePoints;

			userOutputTimePoints.push(10);
			userOutputTimePoints.push(30);
			userOutputTimePoints.push(20);

			vector <OutputTimePoint> outputTimePoints = 
				sut->SimulationTask->OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);
			
			BDDExtensions::ShouldBeEqualTo(outputTimePoints.size(), userOutputTimePoints.size());

			for(unsigned int i=0; i<outputTimePoints.size(); i++)
			{
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].Time(), userOutputTimePoints.top());
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].SaveSystemSolution(), true);
				BDDExtensions::ShouldBeEqualTo(outputTimePoints[i].IsSwitchTimePoint(), false);

				userOutputTimePoints.pop();
			}
		}

		[TestAttribute]   
		void should_return_correct_timepoints_1()
		{
			//first relevant switch point after first relevant user timepoint
			//last relevant switch point before last relevant user timepoint
			DoubleQueue userOutputTimePoints;
			DoubleQueue switchTimePoints;
			DoubleQueue tableFormulaRestartTimePoints;

			userOutputTimePoints.push(_t0-5);
			userOutputTimePoints.push(_t0+10);
			userOutputTimePoints.push(_t0+30);
			userOutputTimePoints.push(_t0+20);

			switchTimePoints.push(_t0-3);
			switchTimePoints.push(_t0+15);
			switchTimePoints.push(_t0+25);
			
			tableFormulaRestartTimePoints.push(_t0+28);
			tableFormulaRestartTimePoints.push(_t0+25);

			//result should be: 
			// {10, Save,     Not Switch, Not Restart}
			// {15, Not Save, Switch,     Not Restart}
			// {20, Save,     Not Switch, Not Restart}
			// {25, Not Save, Switch,     Restart}
			// {28, Not Save, Switch,     Restart}
			// {30, Save,     Not Switch, Not Restart}

			double times[] =        {_t0+10, _t0+15, _t0+20, _t0+25, _t0+28, _t0+30};
			bool   saveSolution[] =     {true,   false,  true,   false,  false, true};
			bool isSwitchTimePoint[] =  {false,  true,   false,  true,   true,  false}; 
			bool restart[] =            {false,  false,  false,  true,   true,  false};

			vector <OutputTimePoint> outputTimePoints = 
				sut->SimulationTask->OutputTimePoints(userOutputTimePoints, switchTimePoints, tableFormulaRestartTimePoints, _t0);
			
			CheckTimePoints(outputTimePoints, 6, times, saveSolution, isSwitchTimePoint,restart);

			BDDExtensions::ShouldBeEqualTo(SimulationTask::NumberOfSimulatedTimeSteps(outputTimePoints), 3);
		}

		[TestAttribute]   
		void should_return_correct_timepoints_2()
		{
			//first relevant switch point = first relevant user timepoint
			//last relevant switch point before last relevant user timepoint
			DoubleQueue userOutputTimePoints;
			DoubleQueue switchTimePoints;
			DoubleQueue tableFormulaRestartTimePoints;

			userOutputTimePoints.push(_t0-5);
			userOutputTimePoints.push(_t0+10);
			userOutputTimePoints.push(_t0+30);
			userOutputTimePoints.push(_t0+20);

			switchTimePoints.push(_t0-3);
			switchTimePoints.push(_t0+10);
			switchTimePoints.push(_t0+25);

			//result should be: 
			// {10, Save,     Switch,     Not Restart}
			// {20, Save,     Not Switch, Not Restart}
			// {25, Not Save, Switch,     Not Restart}
			// {30, Save,     Not Switch, Not Restart}

			double times[] =        {_t0+10, _t0+20, _t0+25, _t0+30};
			bool   saveSolution[] =     {true,   true,   false,  true};
			bool isSwitchTimePoint[] =  {true,   false,  true,   false};
			bool restart[] =            {false,  false,  false,  false};

			vector <OutputTimePoint> outputTimePoints = 
				sut->SimulationTask->OutputTimePoints(userOutputTimePoints, switchTimePoints,tableFormulaRestartTimePoints, _t0);
			
			CheckTimePoints(outputTimePoints, 4, times, saveSolution, isSwitchTimePoint,restart);
		}

		[TestAttribute]
		void should_return_correct_timepoints_3()
		{
			//first relevant switch point before first relevant user timepoint
			//last relevant switch point before last relevant user timepoint
			DoubleQueue userOutputTimePoints;
			DoubleQueue switchTimePoints;
			DoubleQueue tableFormulaRestartTimePoints;

			userOutputTimePoints.push(_t0-5);
			userOutputTimePoints.push(_t0+10);
			userOutputTimePoints.push(_t0+30);
			userOutputTimePoints.push(_t0+20);

			switchTimePoints.push(_t0-3);
			switchTimePoints.push(_t0+5);
			switchTimePoints.push(_t0+25);

			//result should be: 
			// {5,  Not Save, Switch,     Not Restart}
			// {10, Save,     Not Switch, Not Restart}
			// {20, Save,     Not Switch, Not Restart}
			// {25, Not Save, Switch,     Not Restart}
			// {30, Save,     Not Switch, Not Restart}

			double times[] =        {_t0+5, _t0+10, _t0+20, _t0+25, _t0+30};
			bool   saveSolution[] = {false,  true,  true,   false,  true};
			bool isSwitchTimePoint[] =  {true,  false,  false,  true,   false}; 
			bool restart[] =            {false, false,  false,  false,  false};

			vector <OutputTimePoint> outputTimePoints = 
				sut->SimulationTask->OutputTimePoints(userOutputTimePoints, switchTimePoints,tableFormulaRestartTimePoints, _t0);
			
			CheckTimePoints(outputTimePoints, 5, times, saveSolution, isSwitchTimePoint,restart);
		}
		[TestAttribute]   
		void should_return_correct_timepoints_4()
		{
			//first relevant switch point after first relevant user timepoint
			//last relevant switch point = last relevant user timepoint
			DoubleQueue userOutputTimePoints;
			DoubleQueue switchTimePoints;
			DoubleQueue tableFormulaRestartTimePoints;

			userOutputTimePoints.push(_t0-5);
			userOutputTimePoints.push(_t0+10);
			userOutputTimePoints.push(_t0+30);
			userOutputTimePoints.push(_t0+20);

			switchTimePoints.push(_t0-3);
			switchTimePoints.push(_t0+15);
			switchTimePoints.push(_t0+30);

			//result should be: 
			// {10, Save,     Not Switch, Not Restart}
			// {15, Not Save, Switch,     Not Restart}
			// {20, Save,     Not Switch, Not Restart}
			// {30, Save,     Switch,     Not Restart}

			double times[] =        {_t0+10, _t0+15, _t0+20, _t0+30};
			bool   saveSolution[] = {true,   false,  true,   true};
			bool isSwitchTimePoint[] =  {false,  true,   false,  true}; 
			bool restart[] =            {false, false,   false,  false};

			vector <OutputTimePoint> outputTimePoints = 
				sut->SimulationTask->OutputTimePoints(userOutputTimePoints, switchTimePoints,tableFormulaRestartTimePoints, _t0);
			
			CheckTimePoints(outputTimePoints, 4, times, saveSolution, isSwitchTimePoint,restart);
		}

		[TestAttribute]   
		void should_return_correct_timepoints_5()
		{
			//first relevant switch point after first relevant user timepoint
			//last relevant switch point after last relevant user timepoint

			DoubleQueue userOutputTimePoints;
			DoubleQueue switchTimePoints;
			DoubleQueue tableFormulaRestartTimePoints;

			userOutputTimePoints.push(_t0-5);
			userOutputTimePoints.push(_t0+10);
			userOutputTimePoints.push(_t0+30);
			userOutputTimePoints.push(_t0+20);

			switchTimePoints.push(_t0-3);
			switchTimePoints.push(_t0+15);
			switchTimePoints.push(_t0+35);

			//result should be: 
			// {10, Save,     Not Switch, Not Restart}
			// {15, Not Save, Switch,     Not Restart}
			// {20, Save,     Not Switch, Not Restart}
			// {30, Save,     Not Switch, Not Restart}

			double times[] =        {_t0+10, _t0+15, _t0+20, _t0+30};
			bool   saveSolution[] = {true,   false,  true,   true};
			bool isSwitchTimePoint[] =  {false,  true,   false,  false}; 
			bool restart[] =            {false, false,   false,  false};

			vector <OutputTimePoint> outputTimePoints = 
				sut->SimulationTask->OutputTimePoints(userOutputTimePoints, switchTimePoints,tableFormulaRestartTimePoints, _t0);
			
			CheckTimePoints(outputTimePoints, 4, times, saveSolution, isSwitchTimePoint,restart);
		}

	};
}