#ifndef _HierarchicalFormulaObject_H_
#define _HierarchicalFormulaObject_H_

#include <vector>
#include "ErrorData.h"
#include "SimModel/GlobalConstants.h"
#include "SimModel/Quantity.h"
#include "SimModel/TObjectVector.h"

namespace SimModelNative
{

class HierarchicalFormulaObject : 
	public Quantity
{	
	private:
		//Depth in the hierarchy graph:
		//  = HFOBJECT_TOP_LEVEL     -> independent object
		//  = HFOBJECT_TOP_LEVEL + 1 -> depends only on hierarchical formula objects 
		//                              with HierarchyLevel=HFOBJECT_TOP_LEVEL etc.
		int _hierarchyLevel;

		//Index in the total list of all HierarchicalFormulaObjects 
		//of the simulation (0..N-1)
		int _hierarchicalObjectIndex;

		//Total list of all HierarchicalFormulaObjects of the parent simulation
		TObjectVector < HierarchicalFormulaObject > _simHierarchicalObjects;
	
	public:
		HierarchicalFormulaObject ();
		virtual ~HierarchicalFormulaObject();
		void SetObjectIndex (int pIndex);
		int GetObjectIndex ();
		int GetHierarchyLevel ();
		void SetHierarchicalObjects (TObjectVector < HierarchicalFormulaObject > & pHObjects);
		bool AdjustHierarchyLevel ();
		void CheckCyclicDependencies ();

		virtual std::vector < HierarchicalFormulaObject * > GetUsedHierarchicalFormulaObjects () = 0;
};

}//.. end "namespace SimModelNative"


#endif //_HierarchicalFormulaObject_H_

