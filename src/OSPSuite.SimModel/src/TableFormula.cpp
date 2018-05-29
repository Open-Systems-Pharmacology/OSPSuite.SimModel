#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/TableFormula.h"
#include "SimModel/ConstantFormula.h"

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{
using namespace std;

TableFormula::TableFormula(void)
{
	//for the moment, ALWAYS use derived values (no XML attribute for that)
	_useDerivedValues = true;

	_numberOfValuePoints = 0;
	_X_values = NULL;
	_Y_values = NULL;
	
	_derivedValues = NULL;
}

TableFormula::~TableFormula(void)
{
	_valuePoints.clear();

	if(_X_values)
	{
		delete [] _X_values;
		_X_values = NULL;
	}

	if(_Y_values)
	{
		delete [] _Y_values;
		_Y_values = NULL;
	}

	if(_derivedValues)
	{
		delete [] _derivedValues;
		_derivedValues = NULL;
	}

}

void TableFormula::LoadFromXMLNode (const XMLNode & pNode)
{
	// Check if the current tag is actually the one we expect
	assert(pNode.HasName(XMLConstants::TableFormula));

	//common object base part
	ObjectBase::LoadFromXMLNode(pNode);

	//---- table formula specific part
	_useDerivedValues = (pNode.GetAttribute(XMLConstants::UseDerivedValues,_useDerivedValues ? 1:0)==1);

	XMLNode pAllPointsNode = pNode.GetChildNode(XMLConstants::PointList);

	for (XMLNode pPointNode = pAllPointsNode.GetFirstChild(); !pPointNode.IsNull();pPointNode = pPointNode.GetNextSibling()) 
	{
		ValuePoint * valuePoint = new ValuePoint();

		valuePoint->X = pPointNode.GetAttribute(XMLConstants::ValuePointX, 0.0);
		valuePoint->Y = pPointNode.GetAttribute(XMLConstants::ValuePointY, 0.0);
		valuePoint->RestartSolver = (pPointNode.GetAttribute(XMLConstants::RestartSolver, valuePoint->RestartSolver ? 1:0)==1);

		_valuePoints.push_back(valuePoint);
	}
	
	CacheValues();
}

void TableFormula::CacheValues(void)
{
	const char * ERROR_SOURCE = "TableFormula::CacheValues";

	_numberOfValuePoints = _valuePoints.size();

	//for derived mode, at least 2 points are required
	if (_useDerivedValues && (_numberOfValuePoints < 2))
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
		                "Table formula must have at least 2 value points" + FormulaInfoForErrorMessage());

	//for not derived mode: at least 1 point required
	if (_numberOfValuePoints == 0)
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
		                "Table formula must have at least 1 value point" + FormulaInfoForErrorMessage());

	//---- cache values for speedup
	if (_X_values != NULL)
	{
		delete[] _X_values; 
		_X_values = NULL;
	}
	if (_Y_values != NULL)
	{
		delete[] _Y_values; 
		_Y_values = NULL;
	}

	_X_values = new double [_numberOfValuePoints];
	_Y_values = new double [_numberOfValuePoints];

	int i;

	for(i=0;i<_numberOfValuePoints;i++)
	{
		_X_values[i] = _valuePoints[i]->X;
		_Y_values[i] = _valuePoints[i]->Y;
	}
	
	//check that X-array is increasing
	for(i=0;i<_numberOfValuePoints-1;i++)
	{
		if(_X_values[i] >= _X_values[i+1])
			throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE,
  		                "X values of table formula must be strictly increasing" + FormulaInfoForErrorMessage());
	}
	
	//---- ALWAYS add first X-point into the restart times list
	_restartTimes.clear();
	_restartTimes.push_back(_X_values[0]);

	//---- add all points with restartFlag=true to the restart times list 
	//(beginning with the 2nd one!)
	for(i=1;i<_numberOfValuePoints;i++)
	{
		if(_valuePoints[i]->RestartSolver)
			_restartTimes.push_back(_X_values[i]);
	}

	//---- cache derived values for each INTERVAL [X_i; X_i+1)
	//     thus the number of derived values is 1 less than number of X values
	if (_derivedValues != NULL)
	{
		delete[] _derivedValues; 
		_derivedValues = NULL;
	}

	_derivedValues = new double [_numberOfValuePoints-1];

	for(i=0;i<_numberOfValuePoints-1;i++)
	{
		_derivedValues[i] = (_Y_values[i+1] - _Y_values[i]) / (_X_values[i+1] - _X_values[i]);
	}

	if (_useDerivedValues)
	{
		//---- add the start points of the intervals - beginning with the 2-nd interval, if:
		//      a) the derivative value in previous interval was zero and
		//      b) the derivative value in current interval is not zero and
		//      c) interval start point is not already in the restart times list
		for(i=1;i<_numberOfValuePoints-1;i++)
		{
			if (_derivedValues[i-1] != 0.0)
				continue;

			if (_derivedValues[i] == 0.0)
				continue;

			if(_valuePoints[i]->RestartSolver)
				continue; //interval start point was already added to the restart list

			_restartTimes.push_back(_X_values[i]);
		}
	}
}

void TableFormula::XMLFinalizeInstance (const XMLNode & pNode, Simulation * sim)
{
	ObjectBase::XMLFinalizeInstance(pNode, sim);
}

bool TableFormula::Simplify(bool forCurrentRunOnly)
{

	int i;

	//---- for derived values mode: can be simplified only if all derivatives are zero
	if (_useDerivedValues)
	{
		for(i=0;i<_numberOfValuePoints-1;i++)
		{
			if (_derivedValues[i] != 0.0)
				return false;
		}

		return true;
	}

	//---- for non-derived mode: can be simplified only if all y values are equal
	for(i=0;i<_numberOfValuePoints-1;i++)
	{
		if (_Y_values[i] != _Y_values[i+1])
			return false;
	}

	return true;
}

string TableFormula::Equation()
{
	return "Table formula"; //no explicit equation available
}

double TableFormula::DE_Compute (const double * y, const double time, ScaleFactorUsageMode scaleFactorMode)
{
	const char * ERROR_SOURCE = "TableFormula::DE_Compute";

	int i;

	if (_useDerivedValues)
	{
		if ((time < _X_values[0]) || (time >= _X_values[_numberOfValuePoints-1]))
			return 0.0;

		for(i=1; i<_numberOfValuePoints;i++)
		{
			if (time < _X_values[i])
				return _derivedValues[i-1];
		}

		//this point should never be reached
		throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Error occurred during calculating of table formula value" + FormulaInfoForErrorMessage());
	}

	//---- formula is in not-derived mode - return interpolated value
	if(time <= _X_values[0])
		return _Y_values[0];

	if(time >= _X_values[_numberOfValuePoints-1])
		return _Y_values[_numberOfValuePoints-1];

	for(i=1; i<_numberOfValuePoints;i++)
	{
		if (time < _X_values[i])
			return _Y_values[i-1] + (time - _X_values[i-1]) * _derivedValues[i-1];
	}

	//this point should never be reached
	throw ErrorData(ErrorData::ED_ERROR, ERROR_SOURCE, "Error occurred during calculating of table formula value" + FormulaInfoForErrorMessage());
}

void TableFormula::DE_Jacobian (double * * jacobian, const double * y, const double time, const int iEquation, const double preFactor)
{
	//nothing to do for the table formula (as long as it is ref independent)
}

Formula* TableFormula::DE_Jacobian(const int iEquation)
{
	return new ConstantFormula(0.0);
}

Formula * TableFormula::clone()
{
	return new ConstantFormula(0.0);
	TableFormula * f = new TableFormula();

	// TODO: fix

	return f;
}

Formula * TableFormula::RecursiveSimplify()
{
	return this;
}

void TableFormula::SetQuantityReference (const QuantityReference & quantityReference)
{
	//nothing to do for the table formula (as long as it is ref independent)
}

bool TableFormula::IsZero(void)
{
	const bool forCurrentRunOnly = false;
	if (!Simplify(forCurrentRunOnly))
		return false;

	return (DE_Compute(NULL, _X_values[0], USE_SCALEFACTOR) == 0.0);
}

void TableFormula::WriteFormulaMatlabCode (std::ostream & mrOut)
{
	int i;

	//---- derived values mode
	if (_useDerivedValues)
	{
		mrOut<<"    if Time <= "<<_X_values[0]<<" || Time>="<<_X_values[_numberOfValuePoints-1]<<endl;
		mrOut<<"        yout = 0;"<<endl;
		mrOut<<"        return"<<endl;
		mrOut<<"    end"<<endl<<endl;
		
		for(i=1; i<_numberOfValuePoints;i++)
		{
			mrOut<<"    if Time < "<<_X_values[i]<<endl;
			mrOut<<"        yout = "<<_derivedValues[i-1]<<";"<<endl;
			mrOut<<"        return"<<endl;
			mrOut<<"    end"<<endl<<endl;
		}

		return;
	}

	//---- "normal" (non-derived) values
	mrOut<<"    if Time <= "<<_X_values[0]<<endl;
	mrOut<<"        yout = "<<_Y_values[0]<<";"<<endl;
	mrOut<<"        return;"<<endl;
	mrOut<<"    end"<<endl<<endl;

	mrOut<<"    if Time >= "<<_X_values[_numberOfValuePoints-1]<<endl;
	mrOut<<"        yout = "<<_Y_values[_numberOfValuePoints-1]<<";"<<endl;
	mrOut<<"        return;"<<endl;
	mrOut<<"    end"<<endl<<endl;

	mrOut<<"    X_Values = [ "<<endl;
	for (i=0; i<_numberOfValuePoints; i++)
		mrOut<<_X_values[i]<<" ";
	mrOut<<"]"<<endl;

	mrOut<<"    Y_Values = [ "<<endl;
	for (i=0; i<_numberOfValuePoints; i++)
		mrOut<<_Y_values[i]<<" ";
	mrOut<<"]"<<endl;

	mrOut<<"    yout =  interp1(X_Values, Y_Values, Time);"<<endl;

}

void TableFormula::WriteFormulaCppCode(std::ostream & mrOut)
{
	// support for explicit (no non-constant parameter/variables dependend tables only so far)
	// static map allows fast search, nearest extrapolation
	// might be improved by using static vector with cached access hint
	mrOut << "double T_" << GetId() << "(double Time) {" << endl; // , const double *y
	mrOut << "   static const map<double, double> XY_Values = {";
	if (_useDerivedValues) { // use _derivedValues instead of _Y_
		mrOut << "{" << _X_values[0] << "," << 0.0 << "}";
		for (int i = 1; i<_numberOfValuePoints; i++)
			mrOut << ",{" << _X_values[i] << "," << _derivedValues[i-1] << "}";
	}
	else {
		mrOut << "{" << _X_values[0] << "," << _Y_values[0] << "}";
		for(int i=1; i<_numberOfValuePoints; i++)
			mrOut << ",{" << _X_values[i] << "," << _Y_values[i] << "}";
	}
	mrOut << "   };" << endl;
	mrOut << "   auto iter = XY_Values.lower_bound(Time);" << endl;
	mrOut << "   if(iter==XY_Values.begin()) return XY_Values.begin()->second;" << endl;
	mrOut << "   if(iter==XY_Values.end()) return XY_Values.rbegin()->second;" << endl;
	mrOut << "   double x1 = iter->first;" << endl;
	mrOut << "   double y1 = iter->second;" << endl;
	mrOut << "   --iter;" << endl;
	mrOut << "   double x0 = iter->first;" << endl;
	mrOut << "   double x0 = iter->second;" << endl;
	mrOut << "   return y0 + (y1-y0)/(x1-x0)*(Time-x0);" << endl;
	mrOut << "}" << endl << endl;
}

bool TableFormula::UseBracketsForODESystemGeneration ()
{
	return false;
}

bool TableFormula::IsRefIndependent(double & value)
{
	const bool forCurrentRunOnly = false;
	if (!Simplify(forCurrentRunOnly))
		return false;

	value = DE_Compute(NULL, _X_values[0], USE_SCALEFACTOR);

	return true;
}

vector < HierarchicalFormulaObject * > TableFormula::GetUsedHierarchicalFormulaObjects()
{
	return vector<HierarchicalFormulaObject * >();
}

void TableFormula::Finalize()
{
	//nothing to do so far
}

vector <double> TableFormula::RestartTimePoints()
{
	return _restartTimes;
}

bool TableFormula::IsTable(void)
{
	return true;
}

vector <ValuePoint> TableFormula::GetTablePoints()
{
	vector <ValuePoint> valuePoints;

	for(int i=0; i<_numberOfValuePoints; i++)
		valuePoints.push_back(ValuePoint(*(_valuePoints[i])));

	return valuePoints;
}

void TableFormula::SetTablePoints(const std::vector <ValuePoint> & valuePoints)
{
	_valuePoints.clear();

	for(size_t i=0; i<valuePoints.size(); i++)
	{
		const ValuePoint & vp = valuePoints[i];
		_valuePoints.push_back(new ValuePoint(vp));
	}

	CacheValues();
}

void TableFormula::AppendUsedVariables(set<int> & usedVariblesIndices, const set<int> & variblesIndicesUsedInSwitchAssignments)
{
	//nothing to do so far
}

void TableFormula::AppendUsedParameters(std::set<int> & usedParameterIDs)
{
	// TODO: nothing? check
}

void TableFormula::UpdateIndicesOfReferencedVariables()
{
	//nothing to do so far
}

void TableFormula::SetUseDerivedValues(bool useDerivedValues)
{
	_useDerivedValues = useDerivedValues;
}

bool TableFormula::UseDerivedValues()
{
	return _useDerivedValues;
}

double TableFormula::GetValue(double argument)
{
	//currently table formula y values are just numerical, 
	//so we can pass NULL instead of ODE-variable (and scale mode does not matter, so pass any)
	return DE_Compute(NULL, argument, USE_SCALEFACTOR);
}

}//.. end "namespace SimModelNative"
