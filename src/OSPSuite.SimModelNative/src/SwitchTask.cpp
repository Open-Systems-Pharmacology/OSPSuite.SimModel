#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/SwitchTask.h" 

namespace SimModelNative
{

	using namespace std;

	DoubleQueue SwitchTask::SwitchTimePoints(TObjectList<Switch>    & Switches)
	{
		DoubleQueue switchTimePoints;

		for(int switchIdx=0; switchIdx<Switches.size(); switchIdx++)
		{
			vector <double> singleSwitchTimePoints = Switches[switchIdx]->SwitchTimePoints();
			
			for(unsigned int pointIdx=0; pointIdx<singleSwitchTimePoints.size(); pointIdx++)
				switchTimePoints.push(singleSwitchTimePoints[pointIdx]);
		}

		return switchTimePoints;
	}

}//.. end "namespace SimModelNative"
