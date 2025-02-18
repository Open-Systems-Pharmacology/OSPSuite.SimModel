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

cmake -BBuild/Release/$ARCH/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Release -DFuncParserDir=Build/Release/$ARCH/ -DEXT=$EXT
cp packages/OSPSuite.FuncParser.$FUNCPARSER_DIR/OSPSuite.FuncParserNative/bin/native/$ARCH/Release/libOSPSuite.FuncParserNative.$EXT Build/Release/$ARCH/
cp packages/OSPSuite.SimModelSolver_CVODES.$CVODES_DIR/OSPSuite.SimModelSolver_CVODES/bin/native/$ARCH/Release/libOSPSuite.SimModelSolver_CVODES.$EXT Build/Release/$ARCH/
make -C Build/Release/$ARCH/
dotnet build OSPSuite.SimModel4Nix.sln /property:Configuration=Release

cmake -BBuild/Debug/$ARCH/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Debug -DFuncParserDir=Build/Debug/$ARCH/ -DEXT=$EXT
cp packages/OSPSuite.FuncParser.$FUNCPARSER_DIR/OSPSuite.FuncParserNative/bin/native/$ARCH/Debug/libOSPSuite.FuncParserNative.$EXT Build/Debug/$ARCH/
cp packages/OSPSuite.SimModelSolver_CVODES.$CVODES_DIR/OSPSuite.SimModelSolver_CVODES/bin/native/$ARCH/Debug/libOSPSuite.SimModelSolver_CVODES.$EXT Build/Debug/$ARCH/
make -C Build/Debug/$ARCH/
dotnet build OSPSuite.SimModel4Nix.sln /property:Configuration=Debug

export LD_LIBRARY_PATH=Build/Release/$ARCH:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=Build/Release/$ARCH:$DYLD_LIBRARY_PATH

dotnet test tests/OSPSuite.SimModel.Tests/OSPSuite.SimModel.Tests.csproj --no-build --no-restore --configuration:Release --verbosity normal 

dotnet pack src/OSPSuite.SimModel/ -p:PackageVersion=$2 -o ./
