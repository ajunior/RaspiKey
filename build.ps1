$ErrorActionPreference = "Stop"

$msbuild_path="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe"
$platform="ARM"
$configuration="Release"

try 
{
    Remove-Item -Force -Recurse -Path .\build

    Set-Location $PSScriptRoot\raspikey
    & $msbuild_path raspikey.sln /property:Configuration=$configuration /property:Platform=$platform
    if($LastExitCode -ne 0) {
        throw "raspikey.sln build failed" }

    Set-Location $PSScriptRoot\raspikey-ui-vue
    & yarn install
    if($LastExitCode -ne 0) {
        throw "yarn install failed" }
    & yarn build
    if($LastExitCode -ne 0) {
        throw "yarn build failed" }

    Set-Location $PSScriptRoot
    New-Item -ItemType Directory -Force -Path .\build
    if( Test-Path .\setup\setup\raspikey ) {
        Remove-Item -Force -Path .\setup\setup\raspikey }
    Copy-Item -Force -Path .\raspikey\bin\$platform\$configuration\raspikey .\setup\setup
    if( Test-Path .\setup\setup\html ) {
        Remove-Item -Force -Recurse -Path .\setup\setup\html }
    Copy-Item -Recurse -Force -Path .\raspikey-ui-vue\dist .\setup\setup\html

    Compress-Archive -Force -CompressionLevel Optimal -Path .\setup\* .\build\raspikey.zip
}
finally
{
    Set-Location $PSScriptRoot
}