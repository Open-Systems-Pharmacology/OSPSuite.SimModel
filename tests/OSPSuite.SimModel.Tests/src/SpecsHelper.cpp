
#include "SimModelSpecs/SpecsHelper.h"
#include <direct.h>
#include "SimModelManaged/Conversions.h"

#ifdef WIN32
#pragma warning( disable : 4691)
#endif

namespace UnitTests
{
	using namespace OSPSuite::BDDHelper;
	using namespace OSPSuite::BDDHelper::Extensions;
    using namespace NUnit::Framework;
	using namespace System::IO;
	using namespace System;
	using namespace SimModelNET;

	String^ SpecsHelper::BaseDirectory()
	{
		return AppDomain::CurrentDomain::get()->BaseDirectory::get();
	}

	String^ SpecsHelper::TestFileFrom(String^ fileName)
	{
		String^ relPath = (gcnew String("..\\..\\tests\\TestData\\")) + fileName;
		return Path::Combine(SpecsHelper::BaseDirectory(), relPath);
	}
	
	String^ SpecsHelper::SimModelCompConfigFilePath()
	{
		String^ relPath = (gcnew String("..\\..\\src\\OSPSuite_SimModelComp\\OSPSuite_SimModelComp.xml"));
		return Path::Combine(SpecsHelper::BaseDirectory(), relPath);
	}

	String^ SpecsHelper::SchemaPath()
	{
		return Path::Combine(SpecsHelper::BaseDirectory(), "..\\..\\src\\Schema\\OSPSuite.SimModel.xsd");
	}

	void SpecsHelper::ChangeWorkingDirToBaseDir()
	{
		_chdir(NETToCPPConversions::MarshalString(BaseDirectory()).c_str());
	}

	void SpecsHelper::ArraysShouldBeEqual(array<double> ^ ar1, array<double> ^ ar2, double relTol)
	{
		BDDExtensions::ShouldBeEqualTo(ar1->GetLowerBound(0), ar2->GetLowerBound(0));
		BDDExtensions::ShouldBeEqualTo(ar1->GetUpperBound(0), ar2->GetUpperBound(0));

		for (int i=ar1->GetLowerBound(0); i<=ar1->GetUpperBound(0); i++)
		{
			BDDExtensions::ShouldBeEqualTo(ar1[i], ar2[i], relTol);
		}

	}

	void SpecsHelper::SaveSimulationToXML(Simulation^ sim, String^ fileName)
	{
		String^ xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + 
			CPPToNETConversions::MarshalString(sim->GetNativeSimulation()->GetSimulationXMLString());

		StreamWriter^ writer = gcnew StreamWriter(fileName);
		writer->WriteLine(xml);
		writer->Close();
	}
}