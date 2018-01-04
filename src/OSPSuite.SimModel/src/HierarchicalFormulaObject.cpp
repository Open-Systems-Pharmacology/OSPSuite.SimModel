#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/HierarchicalFormulaObject.h"
#include <assert.h>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

HierarchicalFormulaObject::HierarchicalFormulaObject ()
{
	_hierarchicalObjectIndex = HFOBECT_INVALID_INDEX;
	_hierarchyLevel = HFOBJECT_TOP_LEVEL; //per Default, HFOBJECT_TOP_LEVEL means independent object
}

HierarchicalFormulaObject::~HierarchicalFormulaObject()
{
}

void HierarchicalFormulaObject::SetObjectIndex (int pIndex)
{
	_hierarchicalObjectIndex = pIndex;
}

int HierarchicalFormulaObject::GetObjectIndex ()
{
	assert (_hierarchicalObjectIndex != HFOBECT_INVALID_INDEX);
	
	return _hierarchicalObjectIndex;
}

int HierarchicalFormulaObject::GetHierarchyLevel ()
{
	return _hierarchyLevel;
}

void HierarchicalFormulaObject::SetHierarchicalObjects (TObjectVector < HierarchicalFormulaObject > & pHObjects)
{
	_simHierarchicalObjects = pHObjects;
}

bool HierarchicalFormulaObject::AdjustHierarchyLevel ()
{
	//------------------------------------------------------------------
	// Check if one of (direct or indirect) used objects has greater
	// level of hierarchy and increase own hierarchy level if so
	// Return true, if hierarchy level was adjusted, otherwise false
	//------------------------------------------------------------------
	bool RetVal = false;
	
	vector <HierarchicalFormulaObject *> UsedHObjects = GetUsedHierarchicalFormulaObjects();
	
	for (unsigned int i=0; i<UsedHObjects.size(); i++)
	{
		HierarchicalFormulaObject * UsedHObj = UsedHObjects[i];
		if (_hierarchyLevel <= UsedHObj->GetHierarchyLevel())
		{
			_hierarchyLevel = UsedHObj->GetHierarchyLevel() + 1;
			RetVal = true;
		}
	}
	
	return RetVal;
}

void HierarchicalFormulaObject::CheckCyclicDependencies ()
{
	const char * ERROR_SOURCE = "HierarchicalFormulaObject::CheckCyclicDependencies";
	
	bool * IsUsedFlags = NULL;
	
	try
	{	

		// ---------------------------------------------------------------------
		// Fill the list of all direct and indirect HF objects used by the current one
		// If the object itself will appear in this list - it is a cyclic dep.
		// ---------------------------------------------------------------------

		vector <HierarchicalFormulaObject *> UsedHObjects = GetUsedHierarchicalFormulaObjects(); //direct used objects
		HierarchicalFormulaObject * HObject;
		unsigned int i, j;
		unsigned int simHObjectsSize;

		//cache for speedup
		simHObjectsSize = _simHierarchicalObjects.size();

		//for each found used object - set a flag in the <IsUsedFlags> to true
		IsUsedFlags = new bool [simHObjectsSize];
		for(i=0; i<simHObjectsSize; i++)
			IsUsedFlags[i] = false;
		
		for(i=0; i<UsedHObjects.size(); i++)
		{
			HObject = UsedHObjects[i];
			IsUsedFlags[HObject->GetObjectIndex()] = true;
		}
		
		//loop until no new indirect used objects of current object are found
		//(or until a cyclic dependency found)
		bool newUsedHFObjectsFound = true;
		bool CyclicDepFound = false;
		while(newUsedHFObjectsFound)
		{
			newUsedHFObjectsFound = false;
			
			// --------------------------------------------------------------------------------------------
			// for each (direct or indirect) used object:
			// get its used objects and check if they are already registered (IsUsedFlags[Idx] = true)
			// --------------------------------------------------------------------------------------------
			for (i=0; i<simHObjectsSize; i++)
			{
				HObject = _simHierarchicalObjects[i];
				if (IsUsedFlags[HObject->GetObjectIndex()])
				{
					UsedHObjects = HObject->GetUsedHierarchicalFormulaObjects();
					for (j=0; j<UsedHObjects.size(); j++)
					{
						HierarchicalFormulaObject * HObject2 = UsedHObjects[j];
						if (!IsUsedFlags[HObject2->GetObjectIndex()]) //found new (indirect) used object
						{
							IsUsedFlags[HObject2->GetObjectIndex()] = true;
							newUsedHFObjectsFound = true;	
						}
					}
				}
			}
			
			//check if current object depends on itself
			if (IsUsedFlags[_hierarchicalObjectIndex])
			{
				CyclicDepFound = true;
				break;
			}
		}
		
		delete[] IsUsedFlags;
		IsUsedFlags = NULL;
		
		if(CyclicDepFound) 
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Cyclic dependencies found for "+GetFullName());
		
	}
	catch(ErrorData &)
	{
		if (IsUsedFlags)
			delete[] IsUsedFlags;
		throw;
	}
	catch(...)
	{
		if (IsUsedFlags)
			delete[] IsUsedFlags;
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Unknown Error");
	}
}

}//.. end "namespace SimModelNative"
