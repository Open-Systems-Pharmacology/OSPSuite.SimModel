#ifndef _SpecsHelper_H_
#define _SpecsHelper_H_

#include "SimModelManaged/ManagedSimulation.h"

namespace UnitTests
{
    using namespace OSPSuite::BDDHelper;
    using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace System::IO;
	using namespace System;
	using namespace SimModelNET;

	ref class SpecsHelper
	{
	public:
		static String^ BaseDirectory();
		static String^ TestFileFrom(String^ fileName);
		static String^ SchemaPath();
		static String^ SimModelCompConfigFilePath();
		static void ChangeWorkingDirToBaseDir();
		static void ArraysShouldBeEqual(array<double> ^ ar1, array<double> ^ ar2, double relTol);
		static void SaveSimulationToXML(Simulation^ sim, String^ fileName);
	};
}

#endif //_SpecsHelper_H_
