@echo off
setlocal
cd /d "%~dp0.."

set "configuration=%~1"
if "%configuration%"=="" set "configuration=Develop"

set "source=Dependencies\LaziealRuntime\generated\outputs\%configuration%\x64\ParameterEditor"
set "destination=Tools\ParameterEditor"

if not exist "%source%\LaziealParameterEditor.exe" (
    echo ParameterEditor has not been built for %configuration%.
    echo Build LaziealParameterEditor in Dependencies\LaziealRuntime\Project first.
    exit /b 1
)

if not exist "%destination%" mkdir "%destination%"
xcopy /E /I /Y /D "%source%\*" "%destination%\"
if errorlevel 1 exit /b 1

if exist "Project\Assets\Parameters" (
    xcopy /E /I /Y /D "Project\Assets\Parameters" "%destination%\Assets\Parameters"
    if errorlevel 1 exit /b 1
)

echo ParameterEditor was deployed to %destination%.
exit /b 0
