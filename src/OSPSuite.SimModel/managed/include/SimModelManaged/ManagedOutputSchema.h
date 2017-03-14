#ifndef _Managed_Output_Schema_H_
#define _Managed_Output_Schema_H_

#include "SimModel/OutputSchema.h"

namespace SimModelNET
{
	using namespace System::Collections::Generic;

	public interface class IOutputInterval
	{
		property double StartTime
		{
			double get();
			void set(double);
		}

		property double EndTime
		{
			double get();
			void set(double);
		}

		property int NumberOfTimePoints
		{
			int get();
			void set(int);
		}
	};

	public interface class IOutputSchema
	{
		property IReadOnlyList<IOutputInterval^>^ OutputIntervals
		{
			IReadOnlyList<IOutputInterval^>^ get();
		}

		void AddInterval(IOutputInterval^);
		void AddInterval(double startTime, double endTime, int numberOfTimePoints);
	};

	public ref class OutputInterval : public IOutputInterval
	{
	private:
		SimModelNative::OutputInterval * _outputInterval;
	internal:

	public:
		//constructor
		OutputInterval();
		OutputInterval(double startTime, double endTime, int numberOfTimePoints);

		// destructor
		~OutputInterval();

		// finalizer
		!OutputInterval();

		property double StartTime
		{
			virtual double get();
			virtual void set(double);
		}

		property double EndTime
		{
			virtual double get();
			virtual void set(double);
		}

		property int NumberOfTimePoints
		{
			virtual int get();
			virtual void set(int);
		}
	};

	public ref class OutputSchema : public IOutputSchema
	{
	private:
		SimModelNative::OutputSchema * _outputSchema;

	internal:
		IReadOnlyList<IOutputInterval^>^ OutputIntervalsFor(SimModelNative::OutputSchema &);

	public:
		//constructor
		OutputSchema();

		// destructor
		~OutputSchema();

		// finalizer
		!OutputSchema();

		property IReadOnlyList<IOutputInterval^>^ OutputIntervals
		{
			virtual IReadOnlyList<IOutputInterval^>^ get();
		}

		virtual void AddInterval(IOutputInterval^);
		virtual void AddInterval(double startTime, double endTime, int numberOfTimePoints);
	};
}

#endif //_Managed_Output_Schema_H_
