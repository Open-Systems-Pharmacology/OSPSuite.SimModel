#! /bin/sh

#call: buildNix.sh distributionName version
# e.g. buildNix.sh MacOS 4.0.0.49

rm -f OSPSuite.SimModel4Nix.sln

if [ `uname -m` = 'x86_64' ]; 
then
  ARCH=x64
else
  ARCH=Arm64
fi

if [ "$1" = "MacOS" ];
then
    EXT="dylib"
    FUNCPARSER_DIR="$1.$ARCH"
    CVODES_DIR="$1$ARCH"
else
    EXT="so"
    FUNCPARSER_DIR="$1"
    CVODES_DIR="$1"
fi

git submodule update --init --recursive
nuget sources add -name bddhelper -source https://ci.appveyor.com/nuget/ospsuite-bddhelper 
nuget sources add -name utility -source https://ci.appveyor.com/nuget/ospsuite-utility 

nuget install packages.config -OutputDirectory packages -ExcludeVersion
dotnet restore --source https://ci.appveyor.com/nuget/ospsuite-bddhelper --source https://ci.appveyor.com/nuget/ospsuite-utility --source https://www.nuget.org/api/v2/

# copy the original solution file because it will be modified for dotnet build
cp -p -f OSPSuite.SimModel.sln OSPSuite.SimModel4Nix.sln

dotnet sln OSPSuite.SimModel4Nix.sln remove src/OSPSuite.SimModelNative/OSPSuite.SimModelNative.vcxproj
dotnet sln OSPSuite.SimModel4Nix.sln remove src/OSPSuite.SysTool/OSPSuite.SysTool.vcxproj
dotnet sln OSPSuite.SimModel4Nix.sln remove src/OSPSuite.XMLWrapper/OSPSuite.XMLWrapper.vcxproj
dotnet sln OSPSuite.SimModel4Nix.sln remove tests/TestAppNetCore/TestAppNetCore.csproj

for BuildType in Debug Release
do
  cmake -BBuild/${BuildType}/$ARCH/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=${BuildType} -DFuncParserDir=Build/${BuildType}/$ARCH/ -DEXT=$EXT
  cp packages/OSPSuite.FuncParser.$FUNCPARSER_DIR/OSPSuite.FuncParserNative/bin/native/$ARCH/${BuildType}/libOSPSuite.FuncParserNative.$EXT Build/${BuildType}/$ARCH/
  cp packages/OSPSuite.SimModelSolver_CVODES.$CVODES_DIR/OSPSuite.SimModelSolver_CVODES/bin/native/$ARCH/${BuildType}/libOSPSuite.SimModelSolver_CVODES.$EXT Build/${BuildType}/$ARCH/
  make -C Build/${BuildType}/$ARCH/
  dotnet build OSPSuite.SimModel4Nix.sln /property:Configuration=${BuildType}
done

export LD_LIBRARY_PATH=Build/Release/$ARCH:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=Build/Release/$ARCH:$DYLD_LIBRARY_PATH

dotnet test tests/OSPSuite.SimModel.Tests/OSPSuite.SimModel.Tests.csproj --no-build --no-restore --configuration:Release --verbosity normal --logger:"html;LogFileName=../../../testLog_$1.html"

dotnet pack src/OSPSuite.SimModel/ -p:PackageVersion=$2 -o ./
