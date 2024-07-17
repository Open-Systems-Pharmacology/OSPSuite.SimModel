using System;
using System.Runtime.InteropServices;

namespace OSPSuite.SimModel
{
   internal class SimModelImportDefinitions
   {

#if NETSTANDARD2_0
      public const String NATIVE_DLL = "OSPSuite.SimModelNative";
#else
      public const String NATIVE_DLL = "OSPSuite.SimModelNative.dll";
#endif

      public const CallingConvention CALLING_CONVENTION = CallingConvention.Cdecl;
   }
}
