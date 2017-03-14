#ifndef _SwitchTask_H_
#define _SwitchTask_H_

#include "SimModel/Switch.h"
#include "SimModel/TObjectList.h"
#include "SimModel/SimModelTypeDefs.h"

namespace SimModelNative
{

class SwitchTask
{
public:
	//returns all (potential) switch time points which can be estimated
	//BEFORE the simulation run.
	//e.g. if switch condition is something like
	// (Time==P1) Or (Time==P2) or (Time==P3)
	//where P1 and P2 are CONSTANT during the simulation run and P3 is not, then
	// DoubleQueue {P1.Value, P2.Value} will be returned
	static DoubleQueue SwitchTimePoints(TObjectList<Switch>    & Switches);
};

}//.. end "namespace SimModelNative"

#endif //_SwitchTask_H_
