@echo off
pushd %~dp0

set CMD=packages\NUnit.ConsoleRunner.3.12.0\tools\nunit3-console.exe --x86 %*
echo %CMD%
%CMD%

popd
