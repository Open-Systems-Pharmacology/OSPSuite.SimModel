using System;
using System.Runtime.InteropServices;

namespace OSPSuite.SimModel
{
   internal class SimModelImportDefinitions
   {
#if NETFRAMEWORK
      public const String NATIVE_DLL = "OSPSuite.SimModelNative.dll";
#else
      public const String NATIVE_DLL = "libOSPSuite.SimModelNative";
#endif

      public const CallingConvention CALLING_CONVENTION = CallingConvention.Cdecl;
   }
}
