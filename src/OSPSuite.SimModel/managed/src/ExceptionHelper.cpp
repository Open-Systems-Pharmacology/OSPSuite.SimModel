#include "SimModelManaged/ExceptionHelper.h"
#include "SimModelManaged/SimModelException.h"

using namespace std;

namespace SimModelNET
{
	void ExceptionHelper::ThrowExceptionFrom(const string message)
	{
		throw gcnew SimModelException(gcnew System::String(message.c_str()));
	}

	void ExceptionHelper::ThrowExceptionFrom(ErrorData & ED)
	{
		string message = ED.GetDescription();

		//string fullMessage = "SimModel: Exception in "+ED.GetSource();//+":\n"+message;
		//System::Exception^ innerException = gcnew System::Exception(gcnew System::String(fullMessage.c_str()));

		//throw gcnew SimModelException(gcnew System::String(message.c_str()), innerException);
		throw gcnew SimModelException(gcnew System::String(message.c_str()));
	}

	void ExceptionHelper::ThrowExceptionFromUnknown(string source)
	{
		string message = "SimModel: unknown Exception";
		if (source != "")
			message += " in" + source;

		ExceptionHelper::ThrowExceptionFrom(message);
	}

	void ExceptionHelper::ThrowExceptionFromUnknown()
	{
		ExceptionHelper::ThrowExceptionFromUnknown("");
	}

	void ExceptionHelper::ThrowExceptionFrom(System::Exception^ ex)
	{
		auto message = ex->Message+System::Environment::NewLine+ex->StackTrace->ToString();
		throw gcnew System::Exception(message);// gcnew SimModelException(message);
	}
}
