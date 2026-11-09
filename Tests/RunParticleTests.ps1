$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$testOutput = Join-Path $projectRoot 'generated/ParticleTests'
$runtimeRoot = Join-Path $projectRoot 'Dependencies/LaziealRuntime'
$graphicsRoot = Join-Path $runtimeRoot 'Dependencies/LaziealGraphicsFramework'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vsInstall = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsInstall) { throw 'Visual Studio C++ tools are required.' }
New-Item -ItemType Directory -Path $testOutput -Force | Out-Null

$arguments = @(
    '/nologo', '/std:c++20', '/EHsc', '/utf-8', '/W4', '/WX', '/MT', '/DNOMINMAX',
    ('/I"' + $runtimeRoot + '/Project/Include"'),
    ('/I"' + $graphicsRoot + '/Project/Include"'),
    ('"' + $PSScriptRoot + '/ParticleWorldTests.cpp"'),
    ('/Fo"' + $testOutput + '/ParticleWorldTests.obj"'),
    ('/Fe"' + $testOutput + '/ParticleWorldTests.exe"'),
    '/link', '/IGNORE:4099',
    ('"' + $runtimeRoot + '/generated/outputs/Develop/x64/LaziealRuntime.lib"'),
    ('/LIBPATH:"' + $graphicsRoot + '/generated/outputs/Develop/x64"'),
    'LaziealGraphicsFramework.lib', 'DirectXTex.lib', 'DirectXMesh.lib', 'ImGui.lib',
    ('/LIBPATH:"' + $graphicsRoot + '/Project/Externals/assimp/lib/Release"'),
    'assimp-vc143-mt.lib',
    'd3d12.lib', 'dxgi.lib', 'dxguid.lib', 'dxcompiler.lib', 'windowscodecs.lib',
    'user32.lib', 'gdi32.lib', 'ole32.lib', 'shell32.lib', 'winmm.lib'
)
$responsePath = Join-Path $testOutput 'compile.rsp'
Set-Content -LiteralPath $responsePath -Value ($arguments -join ' ') -Encoding utf8
$buildPath = Join-Path $testOutput 'build.cmd'
$buildLines = @(
    '@echo off',
    ('call "' + $vsInstall + '\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul'),
    'if errorlevel 1 exit /b 1',
    ('cl.exe @"' + $responsePath + '"'),
    'exit /b %errorlevel%'
)
Set-Content -LiteralPath $buildPath -Value $buildLines -Encoding ascii
Push-Location $projectRoot
try {
    & $buildPath
    if ($LASTEXITCODE -ne 0) { throw 'Particle tests failed to compile. Build Sandbox Develop|x64 first.' }
    $testExecutable = Join-Path $testOutput 'ParticleWorldTests.exe'
    # Textureのリンク依存に合わせ、Sandboxが使用するDXC DLLをテストにも配置する。
    Copy-Item -LiteralPath (Join-Path $projectRoot 'generated/outputs/Develop/x64/dxcompiler.dll') -Destination $testOutput -Force
    & $testExecutable
    if ($LASTEXITCODE -ne 0) { throw 'Particle tests failed.' }
}
finally {
    Pop-Location
}
