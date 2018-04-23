#ifndef _Managed_Simulation_H_
#define _Managed_Simulation_H_

#include "SimModelManaged/ParameterProperties.h"
#include "SimModelManaged/SpeciesProperties.h"
#include "SimModelManaged/SolverWarning.h"
#include "SimModelManaged/VariableValues.h"
#include "SimModelManaged/ManagedOutputSchema.h"
#include "SimModel/Simulation.h"

namespace SimModelNET
{
	using namespace System::Collections::Generic;

	public enum class MatlabCodeWriteMode
	{
		Formula = 1,
        Values  = 2
	};

	public enum class ParameterNamesWriteMode
    {
        Simple = 1,
        FullyQualified  = 2
	};

	
	public interface class ISimulation
	{
		///(internal) version
		property System::String^ Version
		{
			System::String^ get();
		}

		///load simulation from xml file
        void LoadFromXMLFile(System::String^ file);

		///load simulation from xml string
        void LoadFromXMLString(System::String^ xmlString);

        ///Export simulation to matlab 
        void WriteMatlabCode(System::String^ outDir, MatlabCodeWriteMode writeMode, ParameterNamesWriteMode parameterNamesWriteMode);

		///Export simulation to C++
		void WriteCppCode(System::String^ outDir, MatlabCodeWriteMode writeMode, ParameterNamesWriteMode parameterNamesWriteMode);

        ///list of (initial) properties of ALL parameters of the simulation
		property IList<IParameterProperties^>^ ParameterProperties
		{
			IList<IParameterProperties^>^ get();
		}

        ///list of (initial) properties of ALL Species of the simulation
		property IList<ISpeciesProperties^>^ SpeciesProperties
		{
			IList<ISpeciesProperties^>^ get();
		}

        ///parameters which should be varied
		property IList<IParameterProperties^>^ VariableParameters
		{
			IList<IParameterProperties^>^ get();
			void set(IList<IParameterProperties^>^);
		}

        ///species which should be varied
		property IList<ISpeciesProperties^>^ VariableSpecies
		{
			IList<ISpeciesProperties^>^ get();
			void set(IList<ISpeciesProperties^>^ );
		}

		///set parameter initial values for the next simulation run
        void SetParameterValues(IList<IParameterProperties^>^ parameterProperties);

		///set species initial properties for the next simulation run
		void SetSpeciesProperties(IList<ISpeciesProperties^>^ speciesProperties);

        ///finalize simulation (perform internal optimizations etc.)
        void FinalizeSimulation();

        ///sets if simulation progress should be calculated
		property bool ShowProgress
		{
			bool get();
			void set(bool showProgress);
		}

        ///Max. simulation runtime (will be cancelled with Exception if exceeded)
        ///0 = unlimited (default)
		property double ExecutionTimeLimit
		{
			double get();
			void set(double executionTimeLimit);
		}

        ///sets if warnings should be treated as errors
        ///(for example numerical solver-warnings etc.)
        ///Default is TRUE
		property bool StopOnWarnings
		{
			bool get();
			void set(bool stopOnWarnings);
		}

		///If solving of the diff. eq. systems fails with a convergence error <para></para>
        ///AND the value of AutoReduceTolerances is TRUE, both absolute and relative <para></para>
		///tolerance will be reduced by 10, until either the system was successfully solved <para></para>
		///or the predefined lower bounds for both tolerances were reached
		///
        ///Default is TRUE
		property bool AutoReduceTolerances
		{
			bool get();
			void set(bool autoReduceTolerances);
		}

        void RunSimulation();

        ///simulation progress in % during calculation
		property int Progress
		{
			int get();
		}
    
        ///cancels current simulation run
        void Cancel();

        ///(numerical) warnings of ODE/DDE solver
		property IList<ISolverWarning^>^ SolverWarnings
		{
			IList<ISolverWarning^>^ get();
		}

        ///Output time raster
		property array<double>^ SimulationTimes
		{
			array<double>^ get();
		}

        ///Species- and Observer-values
		property IList<IValues^>^ AllValues
		{
			IList<IValues^>^ get();
		}

        ///Output for given entity id
        IValues^ ValuesFor(System::String^ entityId);

		///Returns true, if (automatic) tolerance reduction was used to solve the system
		property bool ToleranceWasReduced
		{
			bool get();
		}

		///Absolute tolerance used for the solution of the system
		property double UsedAbsoluteTolerance
		{
			double get();
		}

		///Relative tolerance used for the solution of the system
		property double UsedRelativeTolerance
		{
			double get();
		}

		///Enables/disables checking for negative values of positive ODE variables
		property bool CheckForNegativeValues
		{
			bool get();
			void set(bool checkForNegativeValues);
		}

		///Get sensitivity values for given variable or observer by given parameter
		array<double>^ SensitivityValuesFor(System::String^ entityId, System::String^ parameterEntityId);

		///Get sensitivity values for given variable or observer by given parameter
		array<double>^ SensitivityValuesByPathFor(System::String^ entityPath, System::String^ parameterPath);

		void ReleaseMemory();

		///Output Schema
		property IOutputSchema^ OutputSchema
		{
			IOutputSchema^ get();
			void set(IOutputSchema^);
		}
	};

	public ref class Simulation : public ISimulation
	{
	private:
		SimModelNative::Simulation * _simulation;

		std::vector<SimModelNative::ParameterInfo> * _vecAllParameters;
		std::vector<SimModelNative::ParameterInfo> * _vecVariableParameters;
		std::vector<SimModelNative::SpeciesInfo>   * _vecAllSpecies;
		std::vector<SimModelNative::SpeciesInfo>   * _vecVariableSpecies;

		bool _toleranceWasReduced, _autoReduceTolerances;
		double _newAbsTol, _newRelTol;

		void CleanUp();

	internal:
		SimModelNative::Simulation * GetNativeSimulation();
		void ValidateXMLStringWithSchema(System::String^ xmlString);
		array<double>^ SensitivityValuesByPathFor(std::string entityPath, std::string parameterPath);
	public:

		//constructor
		Simulation();

		// destructor
		~Simulation();

		// finalizer
		!Simulation(); 

		///(internal) version
		property System::String^ Version
		{
			virtual System::String^ get();
		}

		///load simulation from xml file
        virtual void LoadFromXMLFile(System::String^ file);

		///load simulation from xml string
        virtual void LoadFromXMLString(System::String^ xmlString);

        ///Export simulation to matlab 
        virtual void WriteMatlabCode(System::String^ outDir, MatlabCodeWriteMode writeMode, ParameterNamesWriteMode parameterNamesWriteMode);

		///Export simulation to C++
		virtual void WriteCppCode(System::String^ outDir, MatlabCodeWriteMode writeMode, ParameterNamesWriteMode parameterNamesWriteMode);

        ///list of (initial) properties of ALL parameters of the simulation
		property IList<IParameterProperties^>^ ParameterProperties
		{
			virtual IList<IParameterProperties^>^ get();
		}

        ///list of (initial) properties of ALL Species of the simulation
		property IList<ISpeciesProperties^>^ SpeciesProperties
		{
			virtual IList<ISpeciesProperties^>^ get();
		}

        ///parameters which should be varied
		property IList<IParameterProperties^>^ VariableParameters
		{
			virtual IList<IParameterProperties^>^ get();
			virtual void set(IList<IParameterProperties^>^);
		}

        ///species which should be varied
		property IList<ISpeciesProperties^>^ VariableSpecies
		{
			virtual IList<ISpeciesProperties^>^ get();
			virtual void set(IList<ISpeciesProperties^>^ );
		}

		///set parameter initial values for the next simulation run
        virtual void SetParameterValues(IList<IParameterProperties^>^ parameterProperties);

		///set species initial properties for the next simulation run
		virtual void SetSpeciesProperties(IList<ISpeciesProperties^>^ speciesProperties);

        ///finalize simulation (perform internal optimizations etc.)
        virtual void FinalizeSimulation();

        ///sets if simulation progress should be calculated
		property bool ShowProgress
		{
			virtual bool get();
			virtual void set(bool showProgress);
		}

        ///Max. simulation runtime (will be cancelled with Exception if exceeded)
        ///0 = unlimited (default)
		property double ExecutionTimeLimit
		{
			virtual double get();
			virtual void set(double executionTimeLimit);
		}

        ///sets if warnings should be treated as errors
        ///(for example numerical solver-warnings etc.)
        ///Default is TRUE
		property bool StopOnWarnings
		{
			virtual bool get();
			virtual void set(bool stopOnWarnings);
		}

		///If solving of the diff. eq. systems fails with a convergence error <para></para>
        ///AND the value of AutoReduceTolerances is TRUE, both absolute and relative <para></para>
		///tolerance will be reduced by 10, until either the system was successfully solved <para></para>
		///or the predefined lower bounds for both tolerances were reached
		///
        ///Default is TRUE
		property bool AutoReduceTolerances
		{
			virtual bool get();
			virtual void set(bool autoReduceTolerances);
		}

        virtual void RunSimulation();

        ///simulation progress in % during calculation
		property int Progress
		{
			virtual int get();
		}
    
        ///cancels current simulation run
        virtual void Cancel();

        ///(numerical) warnings of ODE/DDE solver
		property IList<ISolverWarning^>^ SolverWarnings
		{
			virtual IList<ISolverWarning^>^ get();
		}

        ///Output time raster
		property array<double>^ SimulationTimes
		{
			virtual array<double>^ get();
		}

        ///Species- and Observer-values
		property IList<IValues^>^ AllValues
		{
			virtual IList<IValues^>^ get();
		}

        ///Output curve for given entity id
        virtual IValues^ ValuesFor(System::String^ entityId);

		///Returns true, if (automatic) tolerance reduction was used to solve the system
		property bool ToleranceWasReduced
		{
			virtual bool get();
		}

		///Absolute tolerance used for the solution of the system
		property double UsedAbsoluteTolerance
		{
			virtual double get();
		}

		///Relative tolerance used for the solution of the system
		property double UsedRelativeTolerance
		{
			virtual double get();
		}

		///Enables/disables checking for negative values of positive ODE variables
		property bool CheckForNegativeValues
		{
			virtual bool get();
			virtual void set(bool checkForNegativeValues);
		}

		///Get sensitivity values for given variable by given parameter
		virtual array<double>^ SensitivityValuesFor(System::String^ entityId, System::String^ parameterEntityId);

		///Get sensitivity values for given variable by given parameter
		virtual array<double>^ SensitivityValuesByPathFor(System::String^ entityPath, System::String^ parameterPath);

		virtual void ReleaseMemory();

		///Output Schema
		property IOutputSchema^ OutputSchema
		{
			virtual IOutputSchema^ get();
			virtual void set(IOutputSchema^);
		}
	};

}


#endif //_Managed_Simulation_H_

