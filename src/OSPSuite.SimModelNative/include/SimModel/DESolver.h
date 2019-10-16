#ifndef _DESolver_H_
#define _DESolver_H_

#ifdef _WINDOWS
#pragma warning(disable:4786)
#endif
#include "TObjectList.h"

enum { ADAMS, BDF };           //lmm
enum { FUNCTIONAL, NEWTON };   //iter

#include "SimModel/XMLLoader.h"
#include "SimModel/ObjectBase.h"
#include "SimModelSolverBase/SimModelSolverBase.h"
#include "SolverCallerInterface/SolverCaller.h"
#include "SimModel/DESolverProperties.h"
#include "SimModel/Parameter.h"

namespace SimModelNative
{

class Species;
class Simulation;

typedef struct TimeYYDot
{
	double Time;
	double Y;
	double YDot;
}TimeValueTriple;

class DESolver :
	public ObjectBase,
	public ISolverCaller
{
	private:
		Simulation * _parentSim;

		DESolverProperties m_SolverProperties;
		Species ** m_ODEVariables;

		//Optimization purpose
		int m_ODE_NumUnknowns;

		std::string m_UsedSolver;
		SimModelSolverBase * GetSolver ();

		SimModelSolverBase * SetupSolver(const double simStartTime, const double * initialvalues);
		
		void myDoEvents ();

		double m_AbsTolMin;
		double m_RelTolMin;
		
		bool _showProgress;
		int _noOfInfiniteWarnings;
		void CheckForInfiniteValues(double t, double * ydot);

		//if set to true, DE system variables will be rearranged in order to
		//minimize the bandwidth of the system. After that, band linear solver
		//will be used by the DE solver (if supported)
		//
		//Default value is false!
		bool _useBandLinearSolver;

		//---- lower and upper half band range of the ODE system
	    //The half-bandwidths are set such that the nonzero locations (i, j) 
	    //in the banded Jacobian satisfy 
        // -lowerHalfBandWidth <= j-i <= upperHalfBandWidth
		int _lowerHalfBandWidth;
		int _upperHalfBandWidth;

		TObjectList<Parameter> _sensitivityParameters; //cache for speedup

		double ** redimSensitivityMatrix(void);

		void storeSensitivityValues(int timeStepNumber, double ** sensitivityValues);

		//calculate and set comparison thresholds for variables and observers
		void setComparisonThresholds();

protected:

	//---- for debug purposes only
	std::vector<TimeValueTriple> _rhs_outputs;
	std::vector<TimeValueTriple> _jacobian_outputs;
	void addRhsTimeValueTriple(double t, const double * y, const double * ydot);
	void addJacobianTimeValueTriple(double t, const double * y, const double * * Jacobian);

	public:
		DESolver ();

		void LoadFromXMLNode (const XMLNode & pNode);
		void XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim);

		int GetODE_NumUnknowns () const;
		void SetODE_NumUnknowns (int p_ODE_NumUnknowns);

		void Solve_ODE ();

		Rhs_Return_Value ODERhsFunction(double t, const double * y, const double * p, double * ydot, void * f_data);
		Jacobian_Return_Value ODEJacFunction(double t, const double * y, const double * p, const double * fy, double * * Jacobian, void * Jac_data);
		Sensitivity_Rhs_Return_Value ODESensitivityRhsFunction(double t, const double * y, double * ydot,
			int iS, const double * yS, double * ySdot, void * f_data);
		Rhs_Return_Value DDERhsFunction (double t, const double * y, const double * * yd, double * ydot, void * f_data);
		void DDEDelayFunction (double t, const double * y, double * delays, void * delays_data);
		bool IsSet_ODERhsFunction ();
		bool IsSet_ODEJacFunction ();
		bool IsSet_ODESensitivityRhsFunction();
		bool IsSet_DDERhsFunction ();

		void UnloadSolvers();

		const DESolverProperties & GetSolverProperties() const;

		//if solving of DEQ-system failed with convergence failure, both
		//  absolute and relative tolerances are reduced by factor 10.
		//If no further adjustment possible (both reached their lower bound), 
		//  the function returns false. In this case, DEQ-solving fails finally
		bool ReduceTolerances();

		//----for debug purposes only
		const std::vector<TimeValueTriple> & Rhs_outputs() const;
		const std::vector<TimeValueTriple> & Jacobian_outputs() const;

		bool UseBandLinearSolver();
		void SetUseBandLinearSolver(bool useBandLinearSolver);

		int GetLowerHalfBandWidth();
		int GetUpperHalfBandWidth();

		void SetLowerHalfBandWidth(int lowerHalfBandWidth);
		void SetUpperHalfBandWidth(int upperHalfBandWidth);

};

}//.. end "namespace SimModelNative"


#endif //_DESolver_H_

