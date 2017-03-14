#ifndef _ManagedSolverWarning_H_
#define _ManagedSolverWarning_H_

namespace SimModelNET
{
	//public interface
	public interface class ISolverWarning
	{
		property double OutputTime
		{
			virtual double get();
		}

		property System::String^ Warning
		{
			virtual System::String^ get();
		}
	};

	ref class SolverWarning : public ISolverWarning
	{
	private:
		double _outputTime;
		System::String^ _warning;
	internal:
		SolverWarning(double outputTime, System::String^ warning);
	public:
		property double OutputTime
		{
			virtual double get();
		}

		property System::String^ Warning
		{
			virtual System::String^ get();
		}
	};
}

#endif //_ManagedSolverWarning_H_

