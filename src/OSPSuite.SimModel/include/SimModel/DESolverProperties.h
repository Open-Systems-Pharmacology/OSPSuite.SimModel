#ifndef _DESolverProperties_H_
#define _DESolverProperties_H_

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif

#include "SimModel/XMLLoader.h"
#include "SimModel/Quantity.h"

namespace SimModelNative
{

class DESolverProperties :
	public XMLLoader
{
	private:
		Quantity * m_H0_ref;
		Quantity * m_HMin_ref;
		Quantity * m_HMax_ref;

		Quantity * m_MxStep_ref;

		Quantity * m_AbsTol_ref;
		Quantity * m_RelTol_ref;

		Quantity * m_UseJacobian_ref;

		Quantity * LoadByPropertyName(Simulation * sim, const XMLNode & pNode, const std::string name);

	public:
		DESolverProperties ();
		virtual ~DESolverProperties ();
		void LoadFromXMLNode (const XMLNode & pNode);
		void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

		double GetH0 () const;
		double GetHMin () const;
		double GetHMax () const;

		long GetMxStep () const;
		double GetAbsTol () const;
		double GetRelTol () const;

		bool GetUseJacobian () const;

		bool ReduceTolerances(double absTolMin, double relTolMin);
};

}//.. end "namespace SimModelNative"


#endif //_DESolverProperties_H_

