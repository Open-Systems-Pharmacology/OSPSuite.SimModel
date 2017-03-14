#ifndef _ExceptionHelper_H_
#define _ExceptionHelper_H_

#include "ErrorData.h"
#include <string>

namespace SimModelNET
{
	ref class ExceptionHelper
	{
	public:
		static void ThrowExceptionFrom(ErrorData & ED);
		static void ThrowExceptionFrom(const std::string message);
		static void ThrowExceptionFromUnknown(std::string source);
		static void ThrowExceptionFromUnknown();
	};
}

#endif //_ExceptionHelper_H_

