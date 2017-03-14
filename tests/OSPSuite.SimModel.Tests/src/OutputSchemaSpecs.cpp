#ifdef WIN32
#pragma warning( disable : 4691)
#pragma warning( disable : 4244)
#endif

#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelSpecs/SpecsHelper.h"
#include "SimModelManaged/Conversions.h"
#include "SimModel/OutputSchema.h"
#include "SimModel/TObjectVector.h"

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace SimModelNET;

	ref class OutputSchemaWrapper
	{
	public:
		SimModelNative::OutputSchema * OutputSchema;
		OutputSchemaWrapper(){OutputSchema=new SimModelNative::OutputSchema();}
		~OutputSchemaWrapper(){delete OutputSchema;}
	};

	using namespace SimModelNative;

	public ref class concern_for_output_schema abstract : ContextSpecification<OutputSchemaWrapper^>
    {
    public:
		virtual void GlobalContext() override
		{
			try
			{
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
		}

	protected:

        virtual void Context() override
        {
			try
			{
				sut=gcnew OutputSchemaWrapper();
				
				TObjectVector<SimModelNative::OutputInterval> & OutputIntervals = sut->OutputSchema->OutputIntervals();

				OutputIntervals.push_back(new SimModelNative::OutputInterval(10, 20, 3, Equidistant)); //10,15,20
				OutputIntervals.push_back(new SimModelNative::OutputInterval(20, 30, 3, Equidistant)); //20, 25, 30
				OutputIntervals.push_back(new SimModelNative::OutputInterval(0, 10, 3, Equidistant));  //0,5,10
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
        }
    };

	public ref class when_retrieving_points : public concern_for_output_schema
    {
	protected:   
		array<double>^ _timePoints;

		virtual void Because() override
        {
			try
			{
				DoubleQueue timePointsQueue = sut->OutputSchema->AllTimePoints<float>();
				unsigned int queueSize = timePointsQueue.size();
				_timePoints = gcnew array<double>(queueSize);

				for(unsigned int i=0; i<queueSize; i++)
				{
					_timePoints[i] = timePointsQueue.top();
					timePointsQueue.pop();
				}
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
			catch(System::Exception^ )
			{
				throw;
			}
			catch(...)
			{
				ExceptionHelper::ThrowExceptionFromUnknown();
			}
        }

    public:
        [TestAttribute]
        void should_retrieve_all_points()
        {
			try
			{
				//should return [0 5 10 15 20 25 30]
				BDDExtensions::ShouldBeEqualTo(_timePoints->Length, 7);
				for(int i=0; i<=6; i++)
					BDDExtensions::ShouldBeEqualTo(_timePoints[i], 5.0*i);
			}
			catch(ErrorData & ED)
			{
				ExceptionHelper::ThrowExceptionFrom(ED);
			}
        }
    };


}