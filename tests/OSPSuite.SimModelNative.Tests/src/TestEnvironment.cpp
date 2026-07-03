#include <gtest/gtest.h>

// On Windows the XML wrapper uses MSXML6 via COM (CoCreateInstance). Any test
// that round-trips a formula through the XML layer (e.g. ExplicitFormula parsing)
// therefore requires COM to be initialized for the current thread. The
// production executables do this themselves (see TestAppCpp.cpp), but the
// GoogleTest runner has no such entry point, so we initialize COM once for the
// whole test process via a global test environment. On non-Windows platforms
// the XML wrapper uses libxml2 and this is a no-op.
#ifdef _WINDOWS
#include <objbase.h>

namespace
{
   class ComEnvironment : public ::testing::Environment
   {
   public:
      void SetUp() override { _comInitialized = SUCCEEDED(CoInitialize(NULL)); }
      void TearDown() override
      {
         // Only balance a successful CoInitialize. A failed call (e.g.
         // RPC_E_CHANGED_MODE) must not trigger CoUninitialize.
         if (_comInitialized)
         {
            CoUninitialize();
            _comInitialized = false;
         }
      }

   private:
      bool _comInitialized = false;
   };

   const ::testing::Environment* const _comEnvironment =
      ::testing::AddGlobalTestEnvironment(new ComEnvironment());
}
#endif
