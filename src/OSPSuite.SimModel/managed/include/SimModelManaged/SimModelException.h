#ifndef _SimModelException_H_
#define _SimModelException_H_

namespace SimModelNET
{
	ref class SimModelException : public OSPSuite::Utility::Exceptions::OSPSuiteException
	{
	public:
		SimModelException();
		SimModelException(System::String^ message);
		SimModelException(System::String^ message, System::Exception^ innerException);
	};
}

#endif //_SimModelException_H_

