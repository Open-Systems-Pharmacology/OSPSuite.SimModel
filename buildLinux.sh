#! /bin/sh

#call: buildLinux.sh distributionName version
# e.g. buildLinux.sh CentOS7 4.0.0.49

rm -f OSPSuite.SimModel4Linux.sln

git submodule update --init --recursive
nuget sources add -name bddhelper -source https://ci.appveyor.com/nuget/ospsuite-bddhelper 
nuget sources add -name utility -source https://ci.appveyor.com/nuget/ospsuite-utility 
nuget sources add -name funcparser -source https://ci.appveyor.com/nuget/ospsuite-funcparser 
nuget sources add -name cvodes -source https://ci.appveyor.com/nuget/ospsuite-simmodel-solver-cvodes
nuget sources add -name osp -source https://ci.appveyor.com/nuget/open-systems-pharmacology-ci

rm -rf packages/*$1*
nuget install packages.config -OutputDirectory packages -ExcludeVersion
dotnet restore --source https://ci.appveyor.com/nuget/ospsuite-simmodel-solver-cvodes --source https://ci.appveyor.com/nuget/ospsuite-funcparser --source https://ci.appveyor.com/nuget/ospsuite-bddhelper --source https://ci.appveyor.com/nuget/ospsuite-utility --source https://www.nuget.org/api/v2/

# copy the original solution file because it will be modified for dotnet build
cp -p -f OSPSuite.SimModel.sln OSPSuite.SimModel4Linux.sln

dotnet sln OSPSuite.SimModel4Linux.sln remove src/OSPSuite.SimModelNative/OSPSuite.SimModelNative.vcxproj
dotnet sln OSPSuite.SimModel4Linux.sln remove src/OSPSuite.SysTool/OSPSuite.SysTool.vcxproj
dotnet sln OSPSuite.SimModel4Linux.sln remove src/OSPSuite.XMLWrapper/OSPSuite.XMLWrapper.vcxproj
dotnet sln OSPSuite.SimModel4Linux.sln remove tests/TestAppNetCore/TestAppNetCore.csproj

cmake -BBuild/Release/x64/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Release -DFuncParserDir=Build/Release/x64/
cp packages/OSPSuite.FuncParser.$1/OSPSuite.FuncParserNative/bin/native/x64/Release/libOSPSuite.FuncParserNative.so Build/Release/x64/
cp packages/OSPSuite.SimModelSolver_CVODES.$1/OSPSuite.SimModelSolver_CVODES/bin/native/x64/Release/libOSPSuite.SimModelSolver_CVODES.so Build/Release/x64/
make -C Build/Release/x64/
dotnet build OSPSuite.SimModel4Linux.sln /property:Configuration=Release;Platform=x64

cmake -BBuild/Debug/x64/ -Hsrc/OSPSuite.SimModelNative/ -DCMAKE_BUILD_TYPE=Debug -DFuncParserDir=Build/Debug/x64/
cp packages/OSPSuite.FuncParser.$1/OSPSuite.FuncParserNative/bin/native/x64/Debug/libOSPSuite.FuncParserNative.so Build/Debug/x64/
cp packages/OSPSuite.SimModelSolver_CVODES.$1/OSPSuite.SimModelSolver_CVODES/bin/native/x64/Debug/libOSPSuite.SimModelSolver_CVODES.so Build/Debug/x64/
make -C Build/Debug/x64/
dotnet build OSPSuite.SimModel4Linux.sln /property:Configuration=Debug;Platform=x64

#optionally: run tests
#export LD_LIBRARY_PATH=Build/Release/x64/:$LD_LIBRARY_PATH
#dotnet test --no-build --no-restore --configuration:Release --verbosity normal 

nuget pack src/OSPSuite.SimModel/OSPSuite.SimModel.$1.nuspec -version $2
