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

## Runtime settings

Startup settings are loaded from `Project/RuntimeSetting.ini`:

- `[Runtime] assetRoot`: fixed asset root used during initialization
- `[Window] title`: UTF-8 window title
- `[Window] width`, `height`: initial client size
- `[Window] fullscreen`: initial fullscreen state

During execution, use `LGF::Window` to change the title, client size, or
fullscreen state. The asset root remains fixed after initialization.

Gamepad input is powered by SDL3. By default, left and right Joy-Con controllers
remain separate devices so applications can consume their inputs independently:

```ini
[Input.Gamepad]
enabled = true
combineJoyCons = false
verticalJoyCons = true
backgroundInput = false
```

Vertical mode matches the normal two-handed orientation. Set
`verticalJoyCons = false` when using each Joy-Con horizontally as a miniature
gamepad.

Read a connected gamepad through the LGF API. Gyroscope values are radians per
second, acceleration values are metres per second squared, and rotation delta
is the sensor-sample-integrated rotation for the current frame in radians.

```cpp
const LGF::Gamepad gamepad{ 0 };
if (gamepad.IsConnected() && gamepad.HasGyroscope()) {
    const LGF::Vector3 angularVelocity = gamepad.Gyroscope();
    const LGF::Vector3 frameRotation = gamepad.RotationDelta();
}
```

For a combined Joy-Con pair, `GamepadSensorType::Left` and
`GamepadSensorType::Right` select each controller's IMU. The default sensor
prefers the right Joy-Con when no combined/default sensor is exposed.

With the default separate-device setting, use the side-specific helpers. The
optional is empty while that side is disconnected. `playerIndex` can select a
second controller of the same side.

```cpp
if (const auto left = LGF::JoyCon::Left()) {
    const LGF::Vector3 leftGyro = left->Gyroscope();
}
if (const auto right = LGF::JoyCon::Right()) {
    const LGF::Vector3 rightGyro = right->Gyroscope();
}
```

The sandbox starts in `GamepadTestScene`, an ImGui monitor that displays all
connected gamepads, buttons, axes, gyroscopes, accelerometers, and per-frame
rotation. Separate blue and red 3D boxes follow the left and right Joy-Con.
Press `R` to reset their orientation or `Escape` to open the original sample
scenes. Press `G` on the title scene to return to the monitor.
