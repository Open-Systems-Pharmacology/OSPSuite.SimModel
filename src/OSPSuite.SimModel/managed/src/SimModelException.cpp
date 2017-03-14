#include "SimModelManaged/SimModelException.h"

using namespace OSPSuite::Utility::Exceptions;

namespace SimModelNET
{
	SimModelException::SimModelException() : OSPSuiteException()
	{
	}

	SimModelException::SimModelException(System::String^ message) : OSPSuiteException(message)
	{
	}

	SimModelException::SimModelException(System::String^ message, 
		System::Exception^ innerException) : OSPSuiteException(message, innerException)
	{
	}

}