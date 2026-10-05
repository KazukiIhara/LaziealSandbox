# LaziealSandbox

LaziealRuntimeとLaziealGraphicsFrameworkの機能を確認するための、
サンプルアプリケーション兼開発用サンドボックスです。

## クローン

依存リポジトリを含めて取得します。

```powershell
git clone --recursive <repository-url>
```

再帰オプションを付けずにクローンした場合は、次のコマンドで依存リポジトリを取得できます。

```powershell
git submodule update --init --recursive
```

SandboxはLaziealRuntimeを参照し、LaziealRuntimeは
LaziealGraphicsFrameworkを参照する入れ子構成になっています。

## プロジェクト生成とビルド

`Project/Premake.bat`を実行した後、生成された
`Project/LaziealSandbox.slnx`をVisual Studioで開き、x64構成をビルドします。
RuntimeとGraphicsのプロジェクトもソリューションから参照され、依存順にビルドされます。

## ランタイム設定

起動設定は`Project/RuntimeSetting.ini`から読み込まれます。

- `[Runtime] assetRoot`: 初期化時に使用するアセットルート
- `[Window] title`: UTF-8形式のウィンドウタイトル
- `[Window] width`, `height`: 起動時のクライアント領域サイズ
- `[Window] fullscreen`: フルスクリーンで起動するか

実行中のタイトル、ウィンドウサイズ、フルスクリーン状態は`LGF::Window`から変更できます。
アセットルートは初期化後に変更できません。

## ゲームパッドとJoy-Con

ゲームパッド入力にはSDL3を使用しています。既定では左右のJoy-Conを合成せず、
アプリケーションから個別に取得できます。

```ini
[Input.Gamepad]
enabled = true
combineJoyCons = false
verticalJoyCons = true
backgroundInput = false
```

`verticalJoyCons = true`は、左右のJoy-Conを両手に縦持ちする場合の設定です。
Joy-Conを1本ずつ横持ちする場合は`false`に変更してください。

通常のゲームパッドはインデックスを指定して取得します。

```cpp
const LGF::Gamepad gamepad{ 0 };
if (gamepad.IsConnected() && gamepad.HasGyroscope()) {
    const LGF::Vector3 angularVelocity = gamepad.Gyroscope();
    const LGF::Vector3 frameRotation = gamepad.RotationDelta();
}
```

ジャイロの角速度はrad/s、加速度はm/s²、`RotationDelta()`は
センサー値から積分した1フレーム分の回転量をラジアンで返します。

左右別のJoy-Conは専用APIから取得できます。未接続時は`std::nullopt`を返します。
同じ側を複数接続した場合は`playerIndex`で選択できます。

```cpp
if (const auto left = LGF::JoyCon::Left()) {
    const LGF::Vector3 leftGyro = left->Gyroscope();
}

if (const auto right = LGF::JoyCon::Right()) {
    const LGF::Vector3 rightGyro = right->Gyroscope();
}
```

`combineJoyCons = true`で左右を合成した場合は、
`GamepadSensorType::Left`と`GamepadSensorType::Right`を使って各IMUを選択できます。

## Joy-Con検証シーン

起動直後にImGuiベースの`GamepadTestScene`が表示されます。
接続中のゲームパッド、ボタン、スティック、ジャイロ、加速度、
1フレーム分の回転量をリアルタイムで確認できます。

- 青い3Dボックス: 左Joy-Conの姿勢
- 赤い3Dボックス: 右Joy-Conの姿勢
- `R`: 姿勢をリセット
- `Escape`: 既存のサンプルシーンへ移動
- Titleシーンで`G`: Joy-Con検証シーンへ戻る
