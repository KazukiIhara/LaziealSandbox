@echo off
setlocal
cd /d "%~dp0" || exit /b 1

set "visualStudio=%~1"
if not defined visualStudio call :DetectVisualStudio
set "premake="
if /i "%visualStudio%"=="vs2026" set "premake=Premake.bat"
if /i "%visualStudio%"=="vs2022" set "premake=PremakeVS2022.bat"
if not defined premake (
    echo Usage: Setup.bat [vs2026^|vs2022]
    echo Install Visual Studio with Desktop development with C++, a Windows SDK, and C++ CMake tools.
    exit /b 1
)

where git >nul 2>&1
if errorlevel 1 (
    echo Git was not found. Install Git and add it to PATH, then rerun Setup.bat.
    exit /b 1
)

echo [1/2] Fetching submodules...
git submodule sync --recursive
if errorlevel 1 goto SubmoduleError
git submodule update --init --recursive
if errorlevel 1 goto SubmoduleError

echo [2/2] Preparing dependencies and generating the %visualStudio% solution...
call "Project\%premake%"
if errorlevel 1 (
    echo Setup failed. Check the error above, then rerun Setup.bat %visualStudio%.
    exit /b 1
)

if /i "%visualStudio%"=="vs2022" (
    echo Open "Project\LaziealSandbox.sln" and select Develop ^| x64.
) else (
    echo Open "Project\LaziealSandbox.slnx" and select Develop ^| x64.
)
exit /b 0

:SubmoduleError
echo Could not prepare submodules. Check GitHub access and the error above, then rerun Setup.bat.
exit /b 1

:DetectVisualStudio
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" exit /b 0
for /f "tokens=1 delims=." %%v in ('call "%vswhere%" -latest -products * -version "[17.0,19.0)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 Microsoft.VisualStudio.Component.VC.CMake.Project -property installationVersion') do (
    if "%%v"=="18" set "visualStudio=vs2026"
    if "%%v"=="17" set "visualStudio=vs2022"
)
exit /b 0
