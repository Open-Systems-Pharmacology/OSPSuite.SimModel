#! /bin/sh

# Build the native library and run the .NET tests on Linux or macOS.
#
# Usage: buildNix.sh distributionName
#   e.g. buildNix.sh Linux
#
# distributionName is only used to name the test log file (testLog_$1.html).
# Packing the unified multi-RID NuGet package is done in the dedicated CI
# pack-and-publish job, not here, because it requires natives from all three
# platforms to be present under runtimes/.
#
# The OSP-GitHub-Packages NuGet source must be configured before running this
# script (CI does this; for local runs add it via `nuget sources add` with a
# personal access token that has read:packages scope).

set -e

UNAME_S="$(uname)"
UNAME_M="$(uname -m)"

# OSP supports macOS Arm64 and Linux x86_64. Intel macOS and Linux Arm64 are
# explicitly out of scope - fail fast so a misconfigured local run does not
# silently stage a wrong-arch binary under the wrong runtime folder.
if [ "$UNAME_S" = 'Darwin' ]; then
  if [ "$UNAME_M" != 'arm64' ]; then
    echo "Unsupported macOS architecture: $UNAME_M (only arm64 is supported; Intel macOS was dropped in #198)" >&2
    exit 1
  fi
  ARCH=Arm64
  RID=osx-arm64
  EXT=dylib
  NATIVE_FILE=libOSPSuite.SimModelNative.dylib
elif [ "$UNAME_M" = 'x86_64' ]; then
  ARCH=x64
  RID=linux-x64
  EXT=so
  NATIVE_FILE=libOSPSuite.SimModelNative.so
else
  echo "Unsupported architecture: $UNAME_S/$UNAME_M" >&2
  exit 1
fi

git submodule update --init --recursive

# Restore native upstream packages (FuncParser + CVODES solver) so the C++
# build can find headers and the FuncParser .so/.dylib it links against.
nuget install packages.config -OutputDirectory packages -ExcludeVersion

# Build native (Release only - the runtimes/<rid>/native/ NuGet convention has
# no Debug/Release axis; consumers needing native debugging build from the
# source shipped in the package under OSPSuite.SimModelNative/src/ and
# include/).
cmake -BBuild/Release/$ARCH/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Release -DRID=$RID -DEXT=$EXT
make -C Build/Release/$ARCH/

# Stage the native binary at runtimes/<rid>/native/ - the canonical location
# read by both OSPSuite.SimModel.csproj (for in-repo tests) and the unified
# nuspec (for packing). FuncParser and CVODES natives don't need staging here;
# the .NET SDK auto-copies them to the test bin output via PackageReference
# RID-based asset resolution, and SimModelNative finds them at runtime via
# RPATH=$ORIGIN (Linux) / @rpath (macOS).
mkdir -p runtimes/$RID/native
cp Build/Release/$ARCH/$NATIVE_FILE runtimes/$RID/native/

# Build managed projects via a .NET-only solution (the C++ vcxprojs are not
# buildable with `dotnet`).
rm -f OSPSuite.SimModel.NetOnly.sln
cp -p -f OSPSuite.SimModel.sln OSPSuite.SimModel.NetOnly.sln
dotnet sln OSPSuite.SimModel.NetOnly.sln remove src/OSPSuite.SimModelNative/OSPSuite.SimModelNative.vcxproj
dotnet sln OSPSuite.SimModel.NetOnly.sln remove src/OSPSuite.SysTool/OSPSuite.SysTool.vcxproj
dotnet sln OSPSuite.SimModel.NetOnly.sln remove src/OSPSuite.XMLWrapper/OSPSuite.XMLWrapper.vcxproj

dotnet restore OSPSuite.SimModel.NetOnly.sln
dotnet build OSPSuite.SimModel.NetOnly.sln --configuration Release --no-restore
dotnet test tests/OSPSuite.SimModel.Tests/OSPSuite.SimModel.Tests.csproj --no-build --no-restore --configuration Release --logger:"html;LogFileName=../../../testLog_$1.html"
