#include "SimModelManaged/ParameterProperties.h"
#include "SimModelManaged/Conversions.h"

namespace SimModelNET
{
	using namespace std;

	ParameterProperties::ParameterProperties(SimModelNative::ParameterInfo & parameterInfo, const string & objectPathDelimiter)
		: AdjustableEntityProperties(parameterInfo.GetId(), CPPToNETConversions::MarshalString(parameterInfo.GetEntityId()), 
			CPPToNETConversions::MarshalString(parameterInfo.PathWithoutRoot(objectPathDelimiter)), CPPToNETConversions::MarshalString(parameterInfo.GetFullName()), 
			CPPToNETConversions::MarshalString(parameterInfo.GetDescription()), CPPToNETConversions::MarshalString(parameterInfo.GetUnit()), 
			parameterInfo.IsFormula(), CPPToNETConversions::MarshalString(parameterInfo.GetFormulaEquation()))
	{
		_tablePoints = gcnew List <IValuePoint^ >();
		_canBeVaried = parameterInfo.CanBeVaried();
		_calculateSensitivity = parameterInfo.CalculateSensitivity();

		std::vector <SimModelNative::ValuePoint> tablePoints = parameterInfo.GetTablePoints();
		for(size_t i=0; i<tablePoints.size(); i++)
		{
			SimModelNative::ValuePoint & srcTablePoint = tablePoints[i];

			IValuePoint^ tablePoint = gcnew ValuePoint(srcTablePoint.X, srcTablePoint.Y, srcTablePoint.RestartSolver);
			_tablePoints->Add(tablePoint);
		}
	}

	bool ParameterProperties::CanBeVaried::get()
	{
		return _canBeVaried;
	}

	void ParameterProperties::CanBeVaried::set(bool canBeVaried)
	{
		_canBeVaried = canBeVaried;
	}

	bool ParameterProperties::CalculateSensitivity::get()
	{
		return _calculateSensitivity;
	}

	void ParameterProperties::CalculateSensitivity::set(bool calculateSensitivity)
	{
		_calculateSensitivity = calculateSensitivity;
	}

	ValuePoint::ValuePoint(double x, double y, bool restartSolver)
	{
		_x = x;
		_y = y;
		_restartSolver = restartSolver;
	}

	double ValuePoint::X::get()
	{
		return _x;
	}

	void ValuePoint::X::set(double x)
	{
		_x = x;
	}

	double ValuePoint::Y::get()
	{
		return _y;
	}

	void ValuePoint::Y::set(double y)
	{
		_y = y;
	}

	bool ValuePoint::RestartSolver::get()
	{
		return _restartSolver;
	}

	void ValuePoint::RestartSolver::set(bool restartSolver)
	{
		_restartSolver = restartSolver;
	}

	bool ParameterProperties::IsTable::get()
	{
		return (_tablePoints->Count>0);
	}
	
	IList <IValuePoint^ >^ ParameterProperties::TablePoints::get()
	{
		return _tablePoints;
	}

	void ParameterProperties::TablePoints::set(IList <IValuePoint^ >^ tablePoints)
	{
		_tablePoints = tablePoints;
	}

}
