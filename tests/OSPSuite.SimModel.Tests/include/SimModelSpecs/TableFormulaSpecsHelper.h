#ifndef _TableFormulaSpecsHelper_H_
#define _TableFormulaSpecsHelper_H_
		

#include "SimModel/Formula.h"
#include "SimModel/TableFormula.h"
#include "SimModel/TableFormulaWithOffset.h"

#include <vector>
#include <string>

namespace UnitTests
{
	using namespace std;
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace System::IO;
	using namespace System;
	using namespace SimModelNET;

	//provide access to protected functions via public inheritance
	class TableFormulaExtender : public SimModelNative::TableFormula
	{
	public:
		SimModelNative::TObjectVector <SimModelNative::ValuePoint> & ValuePoints(){return _valuePoints;}
		void CallCacheValues(){CacheValues();}
	};

}

#endif //_TableFormulaSpecsHelper_H_
