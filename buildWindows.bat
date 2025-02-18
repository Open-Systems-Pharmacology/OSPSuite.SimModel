msbuild OSPSuite.SimModel.sln -t:build -restore /property:Configuration=Release;Platform=x64
msbuild OSPSuite.SimModel.sln -t:build -restore /property:Configuration=Debug;Platform=x64

dotnet test tests/OSPSuite.SimModel.Tests/OSPSuite.SimModel.Tests.csproj --no-build --no-restore --configuration:Release --verbosity normal --logger:"html;LogFileName=../../../testLog_Windows.html"

dotnet pack src/OSPSuite.SimModel/OSPSuite.SimModel.csproj -o ./ -p:PackageVersion=%1