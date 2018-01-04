#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/DESolverProperties.h"
#include "SimModel/Simulation.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

DESolverProperties::DESolverProperties ()
{
	//object references to solver parameters
	m_H0_ref = NULL;
	m_HMin_ref = NULL;
	m_HMax_ref = NULL;

	m_MxStep_ref = NULL;

	m_AbsTol_ref = NULL;
	m_RelTol_ref = NULL;

	m_UseJacobian_ref = NULL;
}

DESolverProperties::~DESolverProperties ()
{
}

void DESolverProperties::LoadFromXMLNode (const XMLNode & pNode)
{
	//nothing to do, all properties will be set in XMLFinalizeInstance
}

void DESolverProperties::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	m_H0_ref   = LoadByPropertyName(sim, pNode, XMLConstants::H0);
	m_HMin_ref = LoadByPropertyName(sim, pNode, XMLConstants::HMin);
	m_HMax_ref = LoadByPropertyName(sim, pNode, XMLConstants::HMax);

	m_MxStep_ref = LoadByPropertyName(sim, pNode, XMLConstants::MxStep);

	m_AbsTol_ref = LoadByPropertyName(sim, pNode, XMLConstants::AbsTol);
	m_RelTol_ref = LoadByPropertyName(sim, pNode, XMLConstants::RelTol);

	m_UseJacobian_ref = LoadByPropertyName(sim, pNode, XMLConstants::UseJacobian);
}

Quantity * DESolverProperties::LoadByPropertyName(Simulation * sim,
												  const XMLNode & pNode,
												  const std::string name)
{
	//referenced quantity must exist and must be a parameter
	Quantity * parameter = sim->Parameters().GetObjectById((long)pNode.GetChildNode(name).GetAttribute(XMLConstants::Id, INVALID_QUANTITY_ID));

	if (parameter == NULL)
		throw ErrorData(ErrorData::ED_ERROR, "DESolverProperties::XMLFinalizeInstance",
		                "Solver property " + name + " references non existing parameter");

	if (!parameter->IsConstant(true))
		throw ErrorData(ErrorData::ED_ERROR, "DESolverProperties::XMLFinalizeInstance",
		                "Solver property " + name + " references parameter which is not constant");

	return parameter;
}


double DESolverProperties::GetH0 () const
{
    return m_H0_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
}

double DESolverProperties::GetHMin () const
{
    return m_HMin_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
}

double DESolverProperties::GetHMax () const
{
    return m_HMax_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
}

long DESolverProperties::GetMxStep () const
{
    return (long)m_MxStep_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
}


double DESolverProperties::GetAbsTol () const
{
    return m_AbsTol_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
}

double DESolverProperties::GetRelTol () const
{
    return m_RelTol_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
}


bool DESolverProperties::GetUseJacobian () const
{
    return m_UseJacobian_ref->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR)==1;
}

bool DESolverProperties::ReduceTolerances(double absTolMin, double relTolMin)
{
	double absTol = GetAbsTol();
	double relTol = GetRelTol();

	//check if at least one tolerance is above its lower bound
	if((absTol <= absTolMin) && (relTol<=relTolMin))
		return false;

	if(absTol > absTolMin)
	{
		absTol = min(absTolMin, absTol / 10.0);
		m_AbsTol_ref->SetInitialValue(absTol);
	}

	if(relTol > relTolMin)
	{
		relTol = min(relTolMin, relTol / 10.0);
		m_RelTol_ref->SetInitialValue(relTol);
	}

	return true;
}

}//.. end "namespace SimModelNative"
