#ifndef _SimModelXMLHelper_H_
#define _SimModelXMLHelper_H_

#include "SimModel/ObjectBase.h"
#include "SimModel/TObjectList.h"
#include "SimModel/TObjectVector.h"
#include "XMLWrapper/XMLNode.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/GlobalConstants.h"

#ifdef _WINDOWS
#pragma warning( disable : 4279)
#endif

namespace SimModelNative
{

template< class T >
class SimModelXMLHelper
{
public:

	static void ObjectListLoadFromXMLNode (TObjectList<T> & objectList, 
	                                       const XMLNode & pNode);

	static void ObjectListXMLFinalizeInstance (TObjectList<T> & objectList, 
				    					  	   const XMLNode & pNode, 
										       Simulation * sim);

	static void ObjectVectorLoadFromXMLNode(TObjectVector<T> & objectVector, 
	                                        const XMLNode & pNode);

	static void ObjectVectorXMLFinalizeInstance (TObjectVector<T> & objectVector, 
				    					  	     const XMLNode & pNode, 
										         Simulation * sim);
};

template < class T >
void SimModelXMLHelper<T>::ObjectListLoadFromXMLNode (TObjectList<T> & objectList, 
                                                      const XMLNode & pNode)
{
	//Empty list 
	if (pNode.IsNull()) return;

	for (XMLNode pChild = pNode.GetFirstChild(); !pChild.IsNull();pChild = pChild.GetNextSibling()) 
	{
		// Get Object
		T* newObj = new T();
		//Load From XML NODE 
		newObj->LoadFromXMLNode(pChild);

		//check if the id of the object is unique
		if( objectList.Exists(newObj->GetId()))
			throw ErrorData(ErrorData::ED_ERROR,"TObjectList::LoadFromXMLNode", "Object id is not unique in List "+ XMLHelper::ToString(newObj->GetId()));
	
		// Save object
		objectList.Add(newObj);
	}
}

template < class T >
void SimModelXMLHelper<T>::ObjectVectorLoadFromXMLNode(TObjectVector<T> & objectVector, 
                                                       const XMLNode & pNode)
{
	//Empty list 
	if (pNode.IsNull()) return;

	for (XMLNode pChild = pNode.GetFirstChild(); !pChild.IsNull();pChild = pChild.GetNextSibling()) 
	{
		// Get Object
		T* newObj = new T();
		//Load From XML NODE 
		newObj->LoadFromXMLNode(pChild);
	
		// Save object
		objectVector.push_back(newObj);
	}
}

template < class T >
void SimModelXMLHelper<T>::ObjectListXMLFinalizeInstance (TObjectList<T> & objectList, 
                                                          const XMLNode & pNode, 
                                                          Simulation * sim)
{
	//Empty list 
	if (pNode.IsNull()) return;
	
	for (XMLNode pChild = pNode.GetFirstChild(); !pChild.IsNull();pChild = pChild.GetNextSibling()) 
	{
		T* newObj = objectList.GetObjectById((long)pChild.GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID));
		assert(newObj != NULL); // Has been created with LoadFromXML	
	
		newObj->XMLFinalizeInstance(pChild,sim);
	}
}

template < class T >
void SimModelXMLHelper<T>::ObjectVectorXMLFinalizeInstance (TObjectVector<T> & objectVector, 
				    	 				  	                const XMLNode & pNode, 
										                    Simulation * sim)
{
	//Empty vector 
	if (pNode.IsNull()) return;
	
	int i=0;
	for (XMLNode pChild = pNode.GetFirstChild(); !pChild.IsNull();pChild = pChild.GetNextSibling(), i++) 
	{
		T* newObj = objectVector[i];
		assert(newObj != NULL); // Has been created with LoadFromXML	
	
		newObj->XMLFinalizeInstance(pChild,sim);
	}
}

}//.. end "namespace SimModelNative"

#endif //_SimModelXMLHelper_H_
