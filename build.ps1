$ErrorActionPreference = "Stop"

$msbuild_path="C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe"
$platform="ARM"
$configuration="Release"

Set-Location raspikey
& $msbuild_path raspikey.sln /property:Configuration=$configuration /property:Platform=$platform
Set-Location ..

Set-Location raspikey-ui
yarn build
Set-Location ..

Copy-Item -Force -Path .\raspikey\bin\$platform\$configuration\raspikey .\setup\setup
Copy-Item -Recurse -Force -Path .\raspikey-ui\build .\setup\setup\html

Compress-Archive -CompressionLevel Optimal -Path .\setup\* raspikey.zip