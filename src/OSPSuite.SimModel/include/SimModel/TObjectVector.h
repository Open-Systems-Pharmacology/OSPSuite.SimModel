#ifndef _TObjectVector_H_
#define _TObjectVector_H_

#include <string>
#include <assert.h>

namespace SimModelNative
{

template< class T >
class TObjectVector
{	
	private:
		T * * m_Vector;
		int m_size;
	
	public:
		TObjectVector ();
		void push_back (T * pObject);
		void clear ();
		T * operator [ ] (int index) const;
		int size () const;
		T * * GetVector () const;

		//Clears pointer vector ONLY (the elements itself will NOT be deleted).
		//Required because of elements can be used in >1 ObjectVector simultaneously ==> memory leaks
		//(something like smart pointers would be required for better memory management)
		void FreeVector (void );
};

template < class T >
 TObjectVector<T>::TObjectVector ()
{
	m_size = 0;
	m_Vector=NULL;
}

template < class T >
void TObjectVector<T>::push_back (T * pObject)
{
	if (m_size++==0)
		m_Vector = (T**)malloc(sizeof(T*));
	else
		m_Vector = (T**)realloc(m_Vector,sizeof(T*)*m_size);
	
	//We have allocated the new memory
	//0-Based array 
	m_Vector[m_size-1]=pObject;
}

template < class T >
void TObjectVector<T>::clear ()
{
	if (m_size==0) return;
	
	// Free memory taken by objects
	for(int i=0;i<m_size;i++)
		if (m_Vector[i]!=NULL) delete m_Vector[i];
	
	free(m_Vector);
	m_Vector = NULL;
	m_size=0;
}

template < class T >
T * TObjectVector<T>::operator [ ] (int index) const
{
	assert((index>=0) && (index<m_size));
	return m_Vector[index];
}

template < class T >
int TObjectVector<T>::size () const
{
    return m_size;
}

template < class T >
T * * TObjectVector<T>::GetVector () const
{
	return m_Vector;
}

template < class T >
void TObjectVector<T>::FreeVector (void )
{
	if(m_Vector)
	{
		//clear pointer vector only, leave the elements untouched
		free(m_Vector);
		m_Vector = NULL;
		m_size = 0;
	}
}

}//.. end "namespace SimModelNative"

#endif //_TObjectVector_H_

