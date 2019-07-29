using System;
using System.Runtime.InteropServices;

namespace OSPSuite.SimModel
{
   internal class SimModelImportDefinitions
   {

#if _WINDOWS
      public const String NATIVE_DLL = "OSPSuite.SimModelNative.dll";
#else
      public const String NATIVE_DLL = "libOSPSuite.SimModelNative.so";
#endif

      public const CallingConvention CALLING_CONVENTION = CallingConvention.Cdecl;
   }
}
