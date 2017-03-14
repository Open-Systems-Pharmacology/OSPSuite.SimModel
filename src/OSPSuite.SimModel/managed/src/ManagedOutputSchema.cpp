#include "SimModelManaged/ManagedOutputSchema.h"
#include "SimModelManaged/ExceptionHelper.h"

namespace SimModelNET
{
	using namespace OSPSuite::Utility::Xml;
	using namespace std;

	////internal constructor
	//OutputInterval::OutputInterval(SimModelNative::OutputInterval * outputInterval)
	//{
	//	_outputInterval = outputInterval;
	//}

	//constructor
	OutputInterval::OutputInterval()
	{
		_outputInterval = new SimModelNative::OutputInterval();
	}

	//constructor
	OutputInterval::OutputInterval(double startTime, double endTime, int numberOfTimePoints)
	{
		_outputInterval = new SimModelNative::OutputInterval(startTime, endTime, numberOfTimePoints, SimModelNative::OutputIntervalDistribution::Equidistant);
	}

	// destructor
	OutputInterval::~OutputInterval()
	{
		if (_outputInterval)
			delete(_outputInterval);
		_outputInterval = NULL;
	}

	// finalizer
	OutputInterval::!OutputInterval()
	{
		if (_outputInterval)
			delete(_outputInterval);
		_outputInterval = NULL;
	}

	double OutputInterval::StartTime::get()
	{
		double startTime = 0.0;

		try
		{
			startTime = _outputInterval->StartTime();
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}

		return startTime;
	}
	void OutputInterval::StartTime::set(double startTime)
	{
		try
		{
			_outputInterval->SetStartTime(startTime);
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}
	}

	double OutputInterval::EndTime::get()
	{
		double endTime = 0.0;

		try
		{
			endTime = _outputInterval->EndTime();
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}

		return endTime;
	}
	void OutputInterval::EndTime::set(double endTime)
	{
		try
		{
			_outputInterval->SetEndTime(endTime);
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}
	}

	int OutputInterval::NumberOfTimePoints::get()
	{
		int numberOfTimePoints = 0;

		try
		{
			numberOfTimePoints = _outputInterval->NumberOfTimePoints();
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}

		return numberOfTimePoints;
	}
	void OutputInterval::NumberOfTimePoints::set(int numberOfTimePoints)
	{
		try
		{
			_outputInterval->NumberOfTimePoints(NumberOfTimePoints);
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}
	}

	//OutputSchema::OutputSchema(SimModelNative::OutputSchema *outputSchema)
	//{
	//	_outputSchema = outputSchema;
	//}

	//constructor
	OutputSchema::OutputSchema()
	{
		_outputSchema = new SimModelNative::OutputSchema();
	}

	// destructor
	OutputSchema::~OutputSchema()
	{
		if (_outputSchema)
			delete(_outputSchema);
		_outputSchema = NULL;
	}

	// finalizer
	OutputSchema::!OutputSchema()
	{
		if (_outputSchema)
			delete(_outputSchema);
		_outputSchema = NULL;
	}

	IReadOnlyList<IOutputInterval^>^ OutputSchema::OutputIntervalsFor(SimModelNative::OutputSchema & outputSchema)
	{
		List<IOutputInterval^>^ outputIntervals;

		try
		{
			outputIntervals = gcnew List<IOutputInterval^>();

			for (int i = 0; i<outputSchema.OutputIntervals().size(); i++)
			{
				SimModelNative::OutputInterval * interval = outputSchema.OutputIntervals()[i];

				outputIntervals->Add(gcnew OutputInterval(interval->StartTime(), interval->EndTime(), interval->NumberOfTimePoints()));
			}
		}
		catch (ErrorData & ED)
		{
			ExceptionHelper::ThrowExceptionFrom(ED);
		}
		catch (System::Exception^)
		{
			throw;
		}
		catch (...)
		{
			ExceptionHelper::ThrowExceptionFromUnknown();
		}

		return outputIntervals->AsReadOnly();

	}

	IReadOnlyList<IOutputInterval^>^ OutputSchema::OutputIntervals::get()
	{
		return OutputIntervalsFor(*_outputSchema);
	}

	void OutputSchema::AddInterval(IOutputInterval^ outputInterval)
	{
		AddInterval(outputInterval->StartTime, outputInterval->EndTime, outputInterval->NumberOfTimePoints);
	}

	void OutputSchema::AddInterval(double startTime, double endTime, int numberOfTimePoints)
	{
		_outputSchema->OutputIntervals().push_back(new SimModelNative::OutputInterval(startTime, endTime, numberOfTimePoints, SimModelNative::OutputIntervalDistribution::Equidistant));
	}

}