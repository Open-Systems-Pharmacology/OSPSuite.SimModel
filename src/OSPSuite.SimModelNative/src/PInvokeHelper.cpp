#include "SimModel/PInvokeHelper.h"

#ifdef _WINDOWS
#include "comdef.h"
#endif

#ifdef linux
#define CoTaskMemAlloc malloc
#endif

namespace SimModelNative
{
   using namespace std;

   char* MarshalString(const char* sourceString)
   {
      // Allocate memory for the string
      size_t length = strlen(sourceString) + 1;
      char* destString = (char*)CoTaskMemAlloc(length);
      strcpy_s(destString, length, sourceString);
      return destString;
   }

   char* MarshalString(const string& sourceString)
   {
      return MarshalString(sourceString.c_str());
   }

   char* ErrorMessageFrom(ErrorData& ED)
   {
      return MarshalString(ED.GetDescription());
   }

   char* ErrorMessageFromUnknown(const string& errorSource)
   {
      string message = "Unknown error";
      if (!errorSource.empty())
         message += " in " + errorSource;

      return MarshalString(message);
   }

}//.. end "namespace FuncParserNative"
