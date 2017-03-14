#ifndef _TObjectList_H_
#define _TObjectList_H_

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <string>
#include <vector>
#include <assert.h>

#include "ErrorData.h"
#include "SimModel/GlobalConstants.h"
#include <stdlib.h>

namespace SimModelNative
{

template< class T >
class TObjectList
{
	private:
		T * * m_List;
		long * _objectIds;
		std::vector <std::string> _entityIds;
		int m_size;
	
	public:
		TObjectList ();
		T * GetObjectById (const long id) const;
		T * GetObjectByEntityId(const std::string & entityId);
		T * operator [ ] (int index) const;
		void clear ();
		bool Exists (const long id) const;
		void Add (T * pObject);
		int size () const;
		
		//Clears pointer vector ONLY (the elements itself will NOT be deleted).
		//Required because of elements can be used in >1 ObjectVector simultaneously ==> memory leaks
		//(something like smart pointers would be required for better memory management)
		void FreeVector (void );
		long GetMaxObjectId(void);
};

template < class T >
 TObjectList<T>::TObjectList ()
{
	m_size=0;
	m_List = NULL;
	_objectIds = NULL;
}

template < class T >
T * TObjectList<T>::GetObjectByEntityId(const std::string & entityId)
{
	//Loop ids array and get Index of object with given id 
	for (int i=0;i<m_size;i++)
		if (_entityIds[i]==entityId) return m_List[i];
		
	// Not Found
	return NULL;
}

template < class T >
T * TObjectList<T>::GetObjectById (const long id) const
{
	//Loop ids array and get Index of object with given id 
	for (int i=0;i<m_size;i++)
		if (_objectIds[i]==id) return m_List[i];
		
	// Not Found
	return NULL;
}

template < class T >
T * TObjectList<T>::operator [ ] (int index) const
{
	assert((index>=0) && (index<m_size));
	return m_List[index];
}


template < class T >
void TObjectList<T>::clear ()
{
	if (m_size==0) return;
	
	// Free memory taken by objects
	for(int i=0;i<m_size;i++)
		if (m_List[i]!=NULL) delete m_List[i];
	
	free(m_List);
	m_size=0;

	free(_objectIds);
	_objectIds=NULL;

	m_List = NULL;
}

template < class T >
bool TObjectList<T>::Exists (const long id) const
{
 	for(int i=0;i<m_size;i++)
 		if (_objectIds[i]== id) return true;
 	
 	//Not found
 	return false;
}

template < class T >
void TObjectList<T>::Add (T * pObject)
{
	//First object add, we are using malloc
	//We increment the size after if 
	
	if (m_size++==0)
	{
		m_List = (T**)malloc(sizeof(T*));
		_objectIds = (long *) malloc(sizeof(long));
	}
	else
	{
		m_List = (T**)realloc(m_List,sizeof(T*)*m_size);
		_objectIds = (long *) realloc(_objectIds, sizeof(long)*m_size);
	}
	
	//We have allocated the new memory
	//0-Based array 
	m_List[m_size-1]=pObject;
	
	//Add id in array of ids
	long objId = pObject->GetId();
	if (objId == INVALID_QUANTITY_ID)
		throw ErrorData(ErrorData::ED_ERROR, "TObjectList::Add", "Cannot add object with empty id");
	_objectIds[m_size-1]=objId;

	_entityIds.push_back(pObject->GetEntityId());
}

template < class T >
int TObjectList<T>::size () const
{
	return m_size;
}

template < class T >
void TObjectList<T>::FreeVector (void )
{
	if (m_List)
	{
		//clear pointer vector (leave the elements untouched)
		free(m_List);
		m_List = NULL;
		
		free(_objectIds);
		_objectIds = NULL;
		
		m_size = 0;
	}
}

template < class T >
long TObjectList<T>::GetMaxObjectId(void)
{
	if (m_size == 0)
		return INVALID_QUANTITY_ID;

	long maxId = _objectIds[0];

	for(int i=1; i<m_size; i++)
	{
		if (_objectIds[i] > maxId)
			maxId = _objectIds[i];
	}

	return maxId;
}

}//.. end "namespace SimModelNative"


#endif //_TObjectList_H_

