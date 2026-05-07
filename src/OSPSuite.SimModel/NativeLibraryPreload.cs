using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace OSPSuite.SimModel
{
   internal static class NativeLibraryPreload
   {
      // Runs once when OSPSuite.SimModel.dll is first touched, before any P/Invoke
      // into OSPSuite.SimModelNative fires. Preloads SimModelNative's transitively-
      // linked native dependencies via NativeLibrary.Load — which uses the .NET
      // runtime's native resolver (runtimes/<rid>/native/) — so by the time the
      // OS loader walks SimModelNative's import table, the OS loader finds the
      // already-loaded modules in the process and skips its filesystem search.
      //
      // This is necessary because the .NET runtime's native lib resolution
      // understands the runtimes/<rid>/native/ NuGet convention via *.deps.json,
      // but the OS loader (Win32 LoadLibrary / dlopen) does not — it only searches
      // the loading module's directory and a few standard locations. Without this
      // preload, loading SimModelNative would fail with `DllNotFoundException :
      // Unable to load DLL 'OSPSuite.SimModelNative' or one of its dependencies`
      // because FuncParserNative is statically linked into SimModelNative.
      //
      // CVODES is also preloaded for symmetry — although managed code P/Invokes
      // it directly (so the .NET resolver would handle it on its own), preloading
      // makes the dependency surface explicit and consistent.
      //
      // CA2255 warns that ModuleInitializer is "intended for application code or
      // advanced source generator scenarios" — native dependency preloading is
      // exactly such an advanced scenario, so the warning is suppressed here.
#pragma warning disable CA2255
      [ModuleInitializer]
      internal static void Initialize()
      {
         var assembly = typeof(NativeLibraryPreload).Assembly;
         NativeLibrary.Load("OSPSuite.FuncParserNative", assembly, null);
         NativeLibrary.Load("OSPSuite.SimModelSolver_CVODES", assembly, null);
      }
#pragma warning restore CA2255
   }
}
