#include "SimModel/DESolver.h"
#include "SimModel/Simulation.h"
#include "SimModel/MathHelper.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/SimulationTask.h"

#include "DynamicLibrary.h"

#include <cmath>
#include <ctime>
#include <vector>

namespace SimModelNative
{

	SimModelSolverBase * DESolver::GetSolver ()
	{
		const char * ERROR_SOURCE = "DESolver::GetSolver";

		//load solver library SimModelSolver_<SolverName><SolverVersion>.dll
		std::string LibName = "OSPSuite.SimModelSolver_" + m_UsedSolver;

#ifdef _WINDOWS
		DynamicLibrary* library = DynamicLibraryFactory::GetLibrary(LibName + ".dll");
#else
      DynamicLibrary* library = DynamicLibraryFactory::GetLibrary(LibName);
#endif
      if(!library->IsLoaded() )
		{
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Solver " + m_UsedSolver + " not found");
		}

		//get function pointer to solver creation routine
		typedef SimModelSolverBase * (*GetSolverInterfaceFnType)(ISolverCaller *, int, int);
		GetSolverInterfaceFnType pGetSolverInterface = (GetSolverInterfaceFnType)library->GetFunctionAddress("GetSolverInterface");
		if(!pGetSolverInterface)
			throw LibName+" is not valid SimModel Solver";

		//create new solver instance for current problem size
		SimModelSolverBase * pSolver = (pGetSolverInterface)(this, m_ODE_NumUnknowns, _sensitivityParameters.size());

		return pSolver;
	}

	int DESolver::GetODE_NumUnknowns () const
	{
		return m_ODE_NumUnknowns;
	}

	void DESolver::SetODE_NumUnknowns (int p_ODE_NumUnknowns)
	{
		m_ODE_NumUnknowns=p_ODE_NumUnknowns;
	}

	DESolver::DESolver ()
	{
		_parentSim = NULL;
		m_ODE_NumUnknowns = 0;

		m_ODEVariables = NULL;
		m_UsedSolver = "CVODES"; //default solver
		
		m_AbsTolMin = 1e-12;
		m_RelTolMin = 1e-9;

		_showProgress = false;
		_noOfInfiniteWarnings = 0;

		_useBandLinearSolver = false;

		_lowerHalfBandWidth = 0;
		_upperHalfBandWidth = 0;
	}

	bool DESolver::UseBandLinearSolver()
	{
		return _useBandLinearSolver;
	}

	void DESolver::SetUseBandLinearSolver(bool useBandLinearSolver)
	{
		_useBandLinearSolver = useBandLinearSolver;
	}

	int DESolver::GetLowerHalfBandWidth()
	{
		return _lowerHalfBandWidth;
	}

	int DESolver::GetUpperHalfBandWidth()
	{
		return _upperHalfBandWidth;
	}

	void DESolver::SetLowerHalfBandWidth(int lowerHalfBandWidth)
	{
		_lowerHalfBandWidth = lowerHalfBandWidth;
	}

	void DESolver::SetUpperHalfBandWidth(int upperHalfBandWidth)
	{
		_upperHalfBandWidth = upperHalfBandWidth;
	}

	SimModelSolverBase * DESolver::SetupSolver(const double simStartTime, const double * initialvalues)
	{
		int i;

		//create new solver instance
		SimModelSolverBase * pSolver = this->GetSolver();

		//initial time
		pSolver->SetInitialTime(simStartTime);

		//set initial value of current solver
		vector <double> initialvalues_vec;
		for (i = 0; i < m_ODE_NumUnknowns; i++)
			initialvalues_vec.push_back(initialvalues[i]);
		pSolver->SetInitialValues(initialvalues_vec);

		//set initial values of sensitivity parameters
		vector <double> sensitivityParametersInitialvalues;
		for (i = 0; i < _sensitivityParameters.size(); i++)
		{
			double value = _sensitivityParameters[i]->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
			sensitivityParametersInitialvalues.push_back(value);
		}
		pSolver->SetSensitivityParametersInitialValues(sensitivityParametersInitialvalues);

		//tolerances
		pSolver->SetAbsTol(m_SolverProperties.GetAbsTol());
		pSolver->SetRelTol(m_SolverProperties.GetRelTol());

		//set options common for all solvers
		pSolver->SetMxStep(m_SolverProperties.GetMxStep());
		pSolver->SetH0(m_SolverProperties.GetH0());
		pSolver->SetHMin(m_SolverProperties.GetHMin());
		pSolver->SetHMax(m_SolverProperties.GetHMax());

		//set special solver options
		pSolver->SetOption("MAXORD", 5);
		pSolver->SetOption("MXHNIL", 10);
		pSolver->SetOption("LMM", BDF);
		pSolver->SetOption("ITER", NEWTON);

		//call main solver initialization routine
		pSolver->Init();

		//return created solver instance
		return pSolver;
	}

	void DESolver::Solve_ODE ()
	{
		const char * ERROR_SOURCE = "DESolver::Solve_ODE";

		SimModelSolverBase * pSolver = NULL;  //pointer to new solver instance
		double * initialvalues = NULL;        //(scaled) initial values of DE variables
		double * initialvaluesUnscaled = NULL;        //unscaled initial values of DE variables
		double * solution = NULL;             //solution vector of current problem
		double * solutionAboveAbsTol = NULL;  //solution vector of current problem where all values in [-AbsTol..AbsTol] are set to zero
		double ** sensitivityValues = NULL;   //sensitivity values for one time point [NumberOfUnknowns X NumberOfSensitivityParameters] 

		try
		{
			_rhs_outputs.clear();
			_jacobian_outputs.clear();

			int i;

			//simulation start time
			double simStartTime = _parentSim->GetStartTime();

			// Get number of unknowns (MUST be set BEFORE calling this->GetSolver())
			m_ODE_NumUnknowns = _parentSim->GetODENumUnknowns();

			//cache start execution time
			//used to check if execution time limit exceeded (if applies)
			double executionStartTime = clock() / (double)CLOCKS_PER_SEC;

			//output time points of the simulation
			vector <OutputTimePoint> outputTimePoints = SimulationTask::OutputTimePoints(_parentSim);
			int numberOfTimeSteps = (unsigned int)outputTimePoints.size();
			int numberOfSimulatedTimeSteps = SimulationTask::NumberOfSimulatedTimeSteps(outputTimePoints);

			//get scaled initial values for DE variables
			initialvalues = _parentSim->GetDEInitialValuesScaled();

			//get unscaled initial values
			initialvaluesUnscaled = _parentSim->GetDEInitialValues();

			//redim species/observers/time array of the simulation
			_parentSim->RedimAndInitValues(numberOfSimulatedTimeSteps+1,
				                           initialvalues, initialvaluesUnscaled); //+1 because of sim start time, 
			                                                       //which is not included in outputTimePoints

			//---- cache DE variables arranged by their ODE Index
			m_ODEVariables= new Species * [m_ODE_NumUnknowns];
			if (!m_ODEVariables)
				throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Cannot allocate memory for ODE variables");

			for(i=0; i<m_ODE_NumUnknowns; i++)
				m_ODEVariables[i] = _parentSim->GetDEVariableFromIndex(i);

			//cache sensitivity parameters
			_sensitivityParameters = _parentSim->SensitivityParameters();

			//---- allocate memory for solution and switch updated solution
			solution = new double [m_ODE_NumUnknowns];
			solutionAboveAbsTol = new double [m_ODE_NumUnknowns];

			if (!solution || !solutionAboveAbsTol)
				throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,"Cannot allocate memory for solution vector");

			bool switchJacobians = false;
			//---- perform initial switch update on <initialvalues>
			_parentSim->PerformSwitchUpdate(initialvalues, simStartTime, switchJacobians);

			//initialize solution vector with initial data
			for (i = 0; i < m_ODE_NumUnknowns; i++)
				solution[i] = initialvalues[i];
			
			//---- setup DE solver
			// If number of diff. eq. variables is =0 (no species or all specie constant)
			// don't create the solver (actually nothing to solve)
			// In this case, main loop will just fill output time vector and observers
			if (m_ODE_NumUnknowns > 0)
				pSolver = SetupSolver(simStartTime, initialvalues);

			//---- check if in interactive mode
			_showProgress = _parentSim->Options().ShowProgress();

			//get time limit for the simulation execution (0 means no time limit)
			double executionTimeLimit = _parentSim->Options().ExecutionTimeLimit();

			//index of the next reached output time point
			int TimeStepNumber = 0; 

			_noOfInfiniteWarnings = 0;
			
			//allocate space for sensitivities
			sensitivityValues = redimSensitivityMatrix();

			//---- main DE loop
			for(int timeStepIdx=0; timeStepIdx<numberOfTimeSteps; timeStepIdx++)
			{
				//check if execution time limit exceeded (if applies)
				if (executionTimeLimit > 0.0)
				{
					double currentTime = clock() / (double)CLOCKS_PER_SEC;
					if ((currentTime - executionStartTime) > executionTimeLimit)
						throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Simulation execution time limit exceeded");
				}

				//if in interactive mode:
				// - check if cancel is required
				// - calculate progress of the solving routine
				if (_showProgress)
				{
					myDoEvents();
					if (_parentSim->GetCancelFlag())
						break; //canceled by user

					_parentSim->SetProgress(timeStepIdx*100/numberOfTimeSteps);
				}

				//get next time point where the solution should be calculated
				OutputTimePoint outTimePoint = outputTimePoints[timeStepIdx];

				//---- perform one solver step
				// Try to get next solution vector at <outTimePoint.Time>
				// <solverOutputTime> returns the time reached by solver
				//(in case of success, <solverOutputTime> = <outTimePoint.Time>)
				double solverOutputTime;
				int iResultflag;

				if (m_ODE_NumUnknowns > 0)
				{
					iResultflag = pSolver->PerformSolverStep(outTimePoint.Time(), solution, sensitivityValues, solverOutputTime);

					// Check if solver was successful
					if (iResultflag != DE_NOERROR)
					{
						string DEErrorMsg = "Error solving ODE at time t="+XMLHelper::ToString(outTimePoint.Time())+": "+pSolver->GetSolverErrMsg(iResultflag);
						_parentSim->AddWarning(DEErrorMsg, outTimePoint.Time());

						//if StopOnWarning flag is set - stop the simulation and exit
						if (_parentSim->Options().StopOnWarnings())
							throw SimModelSolverErrorData(pSolver->GetErrorNumberFromSolverReturnValue(iResultflag), ERROR_SOURCE, DEErrorMsg);
					}
					else //in case of success, solution should be retrieved at exactly 'time' time point
						assert(solverOutputTime == outTimePoint.Time());
				}
				else
				{
					//no variables available (no species or all species are constant)
					//The just set solver output time = required output time
					solverOutputTime = outTimePoint.Time();
				}

				//---- check if solution at current time point should be saved
				if (outTimePoint.SaveSystemSolution())
				{
					//increase time step index
					TimeStepNumber++;

					//Update Observer values for this time step
					// (use solution where all values in [-AbsTol..AbsTol] are set to zero!
					for (i = 0; i < m_ODE_NumUnknowns; i++)
						solutionAboveAbsTol[i] = solution[i];

					SimulationTask::SetValuesBelowAbsTolLevelToZero(solutionAboveAbsTol, m_ODE_NumUnknowns, m_SolverProperties.GetAbsTol());
					_parentSim->SetObserverValues(TimeStepNumber, solutionAboveAbsTol, solverOutputTime, sensitivityValues);

					// Output solution at the current time step
					_parentSim->SetTimeValue(TimeStepNumber,solverOutputTime);

					//save solution at the current time step into the compartments
					// (for non-persistable variables: just overwrite the (only) value)
					for (i = 0; i < m_ODE_NumUnknowns; i++)
						m_ODEVariables[i]->SetValue(m_ODEVariables[i]->IsPersistable() ? TimeStepNumber : 0, solution[i]);

					//check for not allowed negative values
					//(must be done BEFORE rescaling the values back)
					if (_parentSim->Options().CheckForNegativeValues())
						SimulationTask::CheckForNegativeValues(m_ODEVariables, m_ODE_NumUnknowns, m_SolverProperties.GetAbsTol(), solverOutputTime);

					//save sensitivity values at the current time step for all variables
					storeSensitivityValues(TimeStepNumber, sensitivityValues);
				}

				bool switchJacobians = false;
				//---- perform switches
				bool switchUpdate = _parentSim->PerformSwitchUpdate(solution, solverOutputTime, switchJacobians);

				if((switchUpdate || outTimePoint.RestartSystem()) &&(m_ODE_NumUnknowns > 0))
				{
					//create double vector for new initial value
					std::vector <double> new_initialvalues_vec;
					for (i = 0; i < m_ODE_NumUnknowns; i++)
						new_initialvalues_vec.push_back(solution[i]);

					// Reset ODE system (we solve a new one)
					iResultflag = pSolver->ReInit(solverOutputTime, new_initialvalues_vec);
					if (switchJacobians)
					{
						for (int iEquation = 0; iEquation < m_ODE_NumUnknowns; iEquation++)
						{
							m_ODEVariables[iEquation]->ClearJacobians();
						}
					}

					if (iResultflag != DE_NOERROR)
						throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, pSolver->GetSolverErrMsg(iResultflag));
				}

			} // end of main DE loop

			//---- Simulation is finished. 
			//     We scale all values back
			//     Value range [-AbsTol..AbsTol] is set to zero
			//     Comparison Thresholds of variables and observers are calculated

			//calculate and set comparison thresholds. This must be done BEFORE rescaling!
			setComparisonThresholds();

			for (i = 0; i < m_ODE_NumUnknowns; i++)
			{
				//setting values below abstol to zero must be done BEFORE rescaling!
				m_ODEVariables[i]->SetValuesBelowAbsTolLevelToZero(m_SolverProperties.GetAbsTol());

				m_ODEVariables[i]->RescaleValues();
			}

			//---- clean up
			delete[] initialvalues; 
			initialvalues = NULL;
			delete[] initialvaluesUnscaled;
			initialvaluesUnscaled = NULL;
			delete[] solution;
			solution = NULL;
			delete[] solutionAboveAbsTol;
			solutionAboveAbsTol = NULL;
			delete[] m_ODEVariables;
			m_ODEVariables = NULL;
			delete pSolver;
			pSolver = NULL;

			if (sensitivityValues)
			{
				for (i = 0; i < m_ODE_NumUnknowns; i++)
					delete[] sensitivityValues[i];
				delete[] sensitivityValues;
				sensitivityValues = NULL;
			}
		}
		catch(...)
		{
			if (initialvalues) delete[] initialvalues;
			if (initialvaluesUnscaled) delete[] initialvaluesUnscaled;
			if (solution) delete[] solution;
			if(solutionAboveAbsTol) delete[] solutionAboveAbsTol;
			if (m_ODEVariables) delete[] m_ODEVariables;
			if (pSolver) delete pSolver;

			if (sensitivityValues)
			{
				for (int i = 0; i < m_ODE_NumUnknowns; i++)
					delete[] sensitivityValues[i];
				delete[] sensitivityValues;
			}

			//rethrow exception only if cancel flag is not set (otherwise: just exit)
			if (!_parentSim->GetCancelFlag())
				throw;
		}
	}

	//calculate and set comparison thresholds for variables and observers
	//this must be done BEFORE rescaling ode variables back with scale factors
	void DESolver::setComparisonThresholds()
	{
		//define threshold for all ODE variables as 10*AbsoluteTolerance
		//where AbsoluteTolerance is the (global) absolute tolerance used in the simulation
		const double variableThreshold = 10.0 * m_SolverProperties.GetAbsTol();

		double * odeVariableThresholds = new double[m_ODE_NumUnknowns];

		int i;

		//---- set threshold for ODE variables
		//     here we must iterate over the species list because
		//     m_ODEVariables contains only NON-CONSTANT variables
		for (i = 0; i < _parentSim->SpeciesList().size(); i++)
		{
			_parentSim->SpeciesList()[i]->SetComparisonThreshold(variableThreshold);
		}

		//---- fill array of ODE variable thresholds (used for observer threshold calculation)
		for (i = 0; i < m_ODE_NumUnknowns; i++)
		{
			odeVariableThresholds[i] = variableThreshold;
		}

		//---- set threshold for observers
		for (i = 0; i<_parentSim->Observers().size(); i++)
		{
			Observer * observer = _parentSim->Observers()[i];
			if (!observer->IsUsedInSimulation())
				continue;

         if(observer->IsConstantDuringCalculation())
            observer->SetComparisonThreshold(variableThreshold);
         else
         {
            //calculate observer threshold as f(ODEVariable_Thresholds, 0.0)
            //(where f(y,t) is the calculation formula of the observer
            double observerThreshold = observer->CalculateValue(odeVariableThresholds, 0.0, USE_SCALEFACTOR);

            observer->SetComparisonThreshold(observerThreshold);
         }
		}

		delete[] odeVariableThresholds;
	}

	void DESolver::LoadFromXMLNode (const XMLNode & pNode)
	{
		//XML SAMPLE

		// <Solver name="CVODE_1.0">
		//		<H0 id="H0"/>
		//      <HMax id="HMax"/>
		//      <HMin id="HMin"/>
		//      <AbsTol id="AbsTol"/>
		//      <MxStep id="MXStep"/>
		//      <RelTol id="RelTol"/>
		//      <UseJacobian id="UseJacobian"/>
		//	    <SolverProperties>
		//			<SolverProperty name="SomeProp" id="SomePropId"/>
		//		</SolverProperties>
		// </Solver>	

		// node might not exist in xml
		if (pNode.IsNull ())
			return;

		// Check if the current tag is actually the one we expect
		assert(pNode.HasName(XMLConstants::Solver));

		m_SolverProperties.LoadFromXMLNode(pNode);
	}

	void DESolver::myDoEvents ()
	{
		// Handles Windows events such as "Cancel".

#ifdef _WINDOWS
		// ================================= WINDOWS

		MSG myMSG;
		while (PeekMessage(&myMSG, 0, 0, 0, 1))
		{
			TranslateMessage(&myMSG);
			DispatchMessage(&myMSG);
		}
#endif
	}

	void DESolver::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
	{
		//ObjectBase::SetSimulation(pContainerObject->GetSimulation());

		// node might not exist in xml
		if (pNode.IsNull ())
			return;

		// Check if the current tag is actually the one we expect
		assert(pNode.HasName(XMLConstants::Solver));

		m_SolverProperties.XMLFinalizeInstance(pNode, sim);

		//cache parent simulation for using in main solver function
		_parentSim = sim;
	}

	Rhs_Return_Value DESolver::ODERhsFunction(double t, const double * y, const double * p, double * ydot, void * f_data)
	{
		//if in interactive mode:
		// - check if cancel is required
		if (_showProgress)
		{
			if (_parentSim->GetCancelFlag())
				throw "Cancelled by user"; //canceled by user
		}

		int i;

		// Set all components of RHS vector to zero
		for (i = 0; i < m_ODE_NumUnknowns; i++)
			ydot[i] = 0.;

		//set value of sensitivity parameters
		for (i = 0; i < _sensitivityParameters.size(); i++)
			_sensitivityParameters[i]->SetInitialValue(p[i]);

		//save solution at the current time step into the compartments
		for (i = 0; i < m_ODE_NumUnknowns; i++)
			m_ODEVariables[i]->DE_Rhs(ydot, y, t);
	
		//----for debug only
		//addRhsTimeValueTriple(t,y,ydot);

		//if (notAllowedNegativeValuesAppeared(t, y))
		//	return RHS_RECOVERABLE_ERROR;

		//check if any value in the RHS array has become not finite and add solver warning if so
		CheckForInfiniteValues(t, ydot);

		return RHS_OK;
	}

	//bool DESolver::notAllowedNegativeValuesAppeared(double t, const double *y)
	//{
	//	for (int i = 0; i < m_ODE_NumUnknowns; i++)
	//	{
	//		if (m_ODEVariables[i]->NegativeValuesAllowed())
	//			continue;

	//		if (y[i] < _lowerBoundForNonnegativeVariables)
	//		{
	//			_parentSim->AddWarning("At t=" + XMLHelper::ToString(t) +
	//				": variable #" + XMLHelper::ToString(i) +
	//				" (" + m_ODEVariables[i]->GetFullName() +
	//				") became negative", t);

	//			return true;
	//		}
	//	}

	//	return false;
	//}

	double ** DESolver::redimSensitivityMatrix()
	{
		if (!_sensitivityParameters.size() || !m_ODE_NumUnknowns)
			return NULL;

		double ** sensitivityValues = new double *[m_ODE_NumUnknowns];
		for (int j = 0; j < m_ODE_NumUnknowns; j++)
			sensitivityValues[j] = new double[_sensitivityParameters.size()];

		return sensitivityValues;
	}

	//<sensitivityValues> has dimensions [NoOf_ODE_Variables] x [NoOf_Sensitivity_Parameters]
	//sensitivityValues[i] contains sensitivity values for the i-th ODE Variable
	//The order of sensitivity values in sensitivityValues[i] is the same as the order of 
	// sensitivity parameters stored in each ODE variable (per construction)
	void DESolver::storeSensitivityValues(int timeStepNumber, double** sensitivityValues)
	{
		if (!_sensitivityParameters.size())
			return;

		for (int variableIdx = 0; variableIdx < m_ODE_NumUnknowns; variableIdx++)
		{
			Species * species = m_ODEVariables[variableIdx];
			species->SetSensitivityValues(timeStepNumber, sensitivityValues[variableIdx]);
		}
	}

	void DESolver::addRhsTimeValueTriple(double t, const double * y, const double * ydot)
	{
		int i;

		TimeValueTriple rhsvalues;
		
		rhsvalues.Time=t;
		rhsvalues.Y=0.0;
		rhsvalues.YDot=0.0;

		for(i=0; i<m_ODE_NumUnknowns; i++)
		{
			rhsvalues.Y += fabs(y[i]);
			rhsvalues.YDot += fabs(ydot[i]);
		}

		_rhs_outputs.push_back(rhsvalues);
	}

	void DESolver::CheckForInfiniteValues(double t, double * ydot)
	{
		bool infiniteValuesDetected = false;

		for (int i = 0; i < m_ODE_NumUnknowns; i++)
		{
			if ((MathHelper::IsNaN(ydot[i])) || (MathHelper::IsInf(ydot[i])) || (MathHelper::IsNegInf(ydot[i])))
			{
				//write the warning for the first 10 RHS function calls
				if (_noOfInfiniteWarnings < 10)
				{
					_parentSim->AddWarning("At t="+XMLHelper::ToString(t)+
										   ": variable #"+XMLHelper::ToString(i)+
										   " ("+m_ODEVariables[i]->GetFullName()+
										   ") is not finite",t);
				}

				infiniteValuesDetected = true;
			}
		}
		if (infiniteValuesDetected)
			_noOfInfiniteWarnings++;

		if (_noOfInfiniteWarnings > 1000)
			throw SimModelSolverErrorData(SimModelSolverErrorData::err_FAILURE, "DESolver::Solve_ODE",
			                              "System could not be solved: RHS is not finite");
	}

	Jacobian_Return_Value DESolver::ODEJacFunction(double t, const double * y, const double * p, const double * fy, double * * Jacobian, void * Jac_data)
	{ 
		const char * ERROR_SOURCE = "DESolver::ODEJacFunction";

		//ODE solver may not call this function, if Jacobian is not set
		if (!this->IsSet_ODEJacFunction ())
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "ODEJacFunction should not be called");

		//set value of sensitivity parameters
		for (int i = 0; i < _parentSim->SensitivityParameters().size(); i++)
			_parentSim->SensitivityParameters()[i]->SetInitialValue(p[i]);

		// Compute Jacobian
		for (int iEquation = 0; iEquation < m_ODE_NumUnknowns; iEquation++)
		{
			for (int otherVariable = 0; otherVariable < m_ODE_NumUnknowns; otherVariable++)
			{
				MATRIX_ELEM(Jacobian, iEquation, otherVariable) = m_ODEVariables[iEquation]->JacobianStateVariableFor(otherVariable)->DE_Compute(y, t, ScaleFactorUsageMode::USE_SCALEFACTOR);
			}
		}

		//----for debug only
		//addJacobianTimeValueTriple(t,y, (const double **)Jacobian);

		return JACOBIAN_OK;
	}

	Sensitivity_Rhs_Return_Value DESolver::ODESensitivityRhsFunction(double t, const double * y, double * ydot,
		int iS, const double * yS, double * ySdot, void * f_data)
	{
		//fill the RHS of the ODE variables first
		Rhs_Return_Value odeRHSReturnValue = ODERhsFunction(t, y, yS, ydot, f_data);

		if (odeRHSReturnValue == RHS_FAILED)
			return SENSITIVITY_RHS_FAILED;

		if (odeRHSReturnValue == RHS_RECOVERABLE_ERROR)
			return SENSITIVITY_RHS_RECOVERABLE_ERROR;

		//just in case (return value should be ok now)
		if (odeRHSReturnValue != RHS_OK)
			throw ErrorData(ErrorData::ED_ERROR, "ODESensitivityRhsFunction", "ODERhsFunction return unknown exit code");

//		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "ODESensitivityRhsFunction not implemented yet");

		//-------------------------------------------------
		// 1. Get cached values
		//-------------------------------------------------
		for (int iEquation = 0; iEquation < m_ODE_NumUnknowns; iEquation++)
		{
			ySdot[iEquation] = m_ODEVariables[iEquation]->JacobianParameterFor(_sensitivityParameters[iS]->GetId())->DE_Compute(y, t, ScaleFactorUsageMode::USE_SCALEFACTOR);
		}
	}

	void DESolver::addJacobianTimeValueTriple(double t, const double * y, const double * * Jacobian)
	{
		int i,j;

		TimeValueTriple jacobianValues;
		
		jacobianValues.Time=t;
		jacobianValues.Y=0.0;
		jacobianValues.YDot=0.0;

		for(i=0; i<m_ODE_NumUnknowns; i++)
		{
			jacobianValues.Y += fabs(y[i]);

			for(j=0; j<m_ODE_NumUnknowns; j++)
				jacobianValues.YDot += fabs(Jacobian[i][j]);
		}

		_jacobian_outputs.push_back(jacobianValues);
	}

	Rhs_Return_Value DESolver::DDERhsFunction (double t, const double * y, const double * * yd, double * ydot, void * f_data)
	{
		throw ErrorData(ErrorData::ED_ERROR, "DDERhsFunction", "DDERhsFunction not implemented yet");
	}

	void DESolver::DDEDelayFunction (double t, const double * y, double * delays, void * delays_data)
	{
		throw ErrorData(ErrorData::ED_ERROR, "DDEDelayFunction", "DDEDelayFunction not implemented yet");
	}

	bool DESolver::IsSet_ODERhsFunction ()
	{
		return true;
	}

	bool DESolver::IsSet_ODEJacFunction ()
	{
		return m_SolverProperties.GetUseJacobian();
	}

	bool DESolver::IsSet_ODESensitivityRhsFunction()
	{
		return IsSet_ODEJacFunction();
	}

	bool DESolver::IsSet_DDERhsFunction ()
	{
		return false;
	}

	const DESolverProperties & DESolver::GetSolverProperties() const
	{
		return m_SolverProperties;
	}

	bool DESolver::ReduceTolerances()
	{
		return m_SolverProperties.ReduceTolerances(m_AbsTolMin, m_RelTolMin);
	}

	const vector<TimeValueTriple> & DESolver::Rhs_outputs() const
	{
		return _rhs_outputs;
	}

	const vector<TimeValueTriple> & DESolver::Jacobian_outputs() const 
	{
		return _jacobian_outputs;
	}
}//.. end "namespace SimModelNative"