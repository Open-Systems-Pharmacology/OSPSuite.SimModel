#ifndef _ParameterProperties_H_
#define _ParameterProperties_H_

#include "SimModelManaged/AdjustableEntityProperties.h"
#include "SimModel/ParameterInfo.h"

namespace SimModelNET
{
	using namespace System::Collections::Generic;

	//public interfaces
	public interface class IValuePoint
	{
		property double X
		{
			double get();
			void set(double);
		}
		property double Y
		{
			double get();
			void set(double);
		}
		property bool RestartSolver
		{
			bool get();
			void set(bool);
		}
	};

	public interface class IParameterProperties : public IAdjustableEntityProperties
	{
		property bool CanBeVaried
		{
			bool get();
		}

		property bool CalculateSensitivity
		{
			bool get();
			void set(bool);
		}

		property bool IsTable
		{
			bool get();
		}
		
		property IList <IValuePoint^ >^	TablePoints
		{
			IList <IValuePoint^ >^ get();
			void set(IList <IValuePoint^ >^);
		}

		property bool UsedInSimulation
		{
			bool get();
		}
	};

	public ref class ValuePoint : IValuePoint
	{
	private:
		double _x;
		double _y;
		bool   _restartSolver;
	public:
		ValuePoint(double x, double y, bool restartSolver);
		
		property double X
		{
			virtual double get();
			virtual void set(double);
		}
		property double Y
		{
			virtual double get();
			virtual void set(double);
		}
		property bool RestartSolver
		{
			virtual bool get();
			virtual void set(bool);
		}
	};

	ref class ParameterProperties : 
		public AdjustableEntityProperties,
		public IParameterProperties
	{
	private:
		bool _canBeVaried;
		bool _calculateSensitivity;
		IList <IValuePoint^ >^ _tablePoints;
		bool _usedInSimulation;
		
	internal:
		ParameterProperties(SimModelNative::ParameterInfo & parameterInfo, const std::string & objectPathDelimiter);
		
	public:

		property bool CanBeVaried
		{
			virtual bool get();
		internal:
			void set(bool canBeVaried);
		}

		property bool UsedInSimulation
		{
			virtual bool get();
		internal:
			void set(bool usedInSimulation);
		}

		property bool CalculateSensitivity
		{
			virtual bool get();
			virtual void set(bool);
		}

		property bool IsTable
		{
			virtual bool get();
		}
		
		virtual property IList <IValuePoint^ >^	TablePoints
		{
			IList <IValuePoint^ >^ get();
			void set(IList <IValuePoint^ >^);
		}

	};
}

#endif //_ParameterProperties_H_

