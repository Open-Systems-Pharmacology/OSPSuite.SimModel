#ifndef _PInvokeHelper_H_
#define _PInvokeHelper_H_

#include <string>
#include "ErrorData.h"

#ifdef _WINDOWS
#include "comdef.h"
#endif

#if defined(linux) || defined (__APPLE__)
#define CoTaskMemAlloc malloc
#endif

namespace SimModelNative
{
   char* MarshalString(const char* sourceString);
   char* MarshalString(const std::string & sourceString);
   char* ErrorMessageFrom(ErrorData& ED);
   char* ErrorMessageFromUnknown(const std::string & errorSource="");
}//.. end "namespace SimModelNative"


#endif //_PInvokeHelper_H_

