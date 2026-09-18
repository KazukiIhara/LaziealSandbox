# LaziealSandbox

LaziealSandbox is the sample application and development playground for
LaziealRuntime and LaziealGraphicsFramework.

## Clone

```powershell
git clone --recursive <repository-url>
```

Dependencies are nested intentionally: Sandbox references Runtime, and Runtime
references LaziealGraphicsFramework. Update all of them with:

```powershell
git submodule update --init --recursive
```

## Generate and build

Run `Project/Premake.bat`, then build `Project/LaziealSandbox.slnx` for x64.
The solution references the Runtime and Graphics projects directly, so Visual
Studio builds them in dependency order.
