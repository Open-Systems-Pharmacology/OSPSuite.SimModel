#! /bin/sh

#call: buildNix.sh distributionName version
# e.g. buildNix.sh MacOS 4.0.0.49

rm -f OSPSuite.SimModel4Nix.sln

if [ "$1" = "MacOS" ];
then
    EXT="dylib"
else
    EXT="so"
fi

git submodule update --init --recursive
nuget sources add -name bddhelper -source https://ci.appveyor.com/nuget/ospsuite-bddhelper 
nuget sources add -name utility -source https://ci.appveyor.com/nuget/ospsuite-utility 
nuget sources add -name funcparser -source https://ci.appveyor.com/nuget/ospsuite-funcparser 
nuget sources add -name cvodes -source https://ci.appveyor.com/nuget/ospsuite-simmodel-solver-cvodes

rm -rf packages/*$1*
nuget install packages.config -OutputDirectory packages -ExcludeVersion
dotnet restore --source https://ci.appveyor.com/nuget/ospsuite-simmodel-solver-cvodes --source https://ci.appveyor.com/nuget/ospsuite-funcparser --source https://ci.appveyor.com/nuget/ospsuite-bddhelper --source https://ci.appveyor.com/nuget/ospsuite-utility --source https://www.nuget.org/api/v2/

# copy the original solution file because it will be modified for dotnet build
cp -p -f OSPSuite.SimModel.sln OSPSuite.SimModel4Nix.sln

dotnet sln OSPSuite.SimModel4Nix.sln remove src/OSPSuite.SimModelNative/OSPSuite.SimModelNative.vcxproj
dotnet sln OSPSuite.SimModel4Nix.sln remove src/OSPSuite.SysTool/OSPSuite.SysTool.vcxproj
dotnet sln OSPSuite.SimModel4Nix.sln remove src/OSPSuite.XMLWrapper/OSPSuite.XMLWrapper.vcxproj
dotnet sln OSPSuite.SimModel4Nix.sln remove tests/TestAppNetCore/TestAppNetCore.csproj

cmake -BBuild/Release/x64/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Release -DFuncParserDir=Build/Release/x64/ -DEXT=$EXT -DCMAKE_OSX_ARCHITECTURES=x86_64
cp packages/OSPSuite.FuncParser.$1/OSPSuite.FuncParserNative/bin/native/x64/Release/libOSPSuite.FuncParserNative.$EXT Build/Release/x64/
cp packages/OSPSuite.SimModelSolver_CVODES.$1/OSPSuite.SimModelSolver_CVODES/bin/native/x64/Release/libOSPSuite.SimModelSolver_CVODES.$EXT Build/Release/x64/
make -C Build/Release/x64/
dotnet build OSPSuite.SimModel4Nix.sln /property:Configuration=Release;Platform=x64

cmake -BBuild/Debug/x64/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Debug -DFuncParserDir=Build/Debug/x64/ -DEXT=$EXT  -DCMAKE_OSX_ARCHITECTURES=x86_64
cp packages/OSPSuite.FuncParser.$1/OSPSuite.FuncParserNative/bin/native/x64/Debug/libOSPSuite.FuncParserNative.$EXT Build/Debug/x64/
cp packages/OSPSuite.SimModelSolver_CVODES.$1/OSPSuite.SimModelSolver_CVODES/bin/native/x64/Debug/libOSPSuite.SimModelSolver_CVODES.$EXT Build/Debug/x64/
make -C Build/Debug/x64/
dotnet build OSPSuite.SimModel4Nix.sln /property:Configuration=Debug;Platform=x64

#optionally: run tests
export LD_LIBRARY_PATH=Build/Release/x64:$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=Build/Release/x64:$DYLD_LIBRARY_PATH

dotnet test tests/OSPSuite.SimModel.Tests/OSPSuite.SimModel.Tests.csproj --no-build --no-restore --configuration:Release --verbosity normal 

dotnet pack src/OSPSuite.SimModel/ -p:PackageVersion=$2
