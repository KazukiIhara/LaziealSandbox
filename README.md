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
ファイルが存在しない場合はデフォルト値で自動生成されます。
エンジン更新で設定項目が追加された場合も、読み込めた既存値と不足項目の
デフォルト値を使って、既定のセクション・項目順に自動更新されます。
自動更新時はコメント、未知の項目、独自の並び順は保持されません。

- `[Runtime] assetRoot`: 初期化時に使用するアセットルート
- `[Window] title`: UTF-8形式のウィンドウタイトル
- `[Window] width`, `height`: 起動時のクライアント領域サイズ
- `[Window] fullscreen`: フルスクリーンで起動するか

実行中のタイトル、ウィンドウサイズ、フルスクリーン状態は`LGF::Window`から変更できます。
アセットルートは初期化後に変更できません。

## JSONパラメータ

`LGF::Parameter`を使うと、`Assets/Parameters`内のJSONから型付きの値を
静的APIで取得できます。ファイルは初回アクセス時に読み込まれ、キャッシュされます。

```cpp
const LGF::Vector3 position =
    LGF::Parameter::Get<LGF::Vector3>("Sphere.json", "Pos");
const float speed =
    LGF::Parameter::GetOr<float>("Player.json", "Speed", 1.0f);
```

必須パラメータには`Get`、既定値を許容するパラメータには`GetOr`、
取得失敗を呼び出し側で処理する場合は`TryGet`を使用します。
ファイルを読み直す場合は`Reload`、全キャッシュを破棄する場合は`ClearCache`を
使用します。

`LGF::ParameterDocument`は、階層化した型付きパラメータをJSONファイルへ
読み書きします。対応型は`bool`、`int32_t`、`float`、`Vector2`、`Vector3`、
`Vector4`、`std::string`です。

```cpp
LGF::ParameterDocument parameters;
if (parameters.Load("Assets/Parameters/FriedRice.json")) {
    const float friction =
        parameters.GetOr<float>("Physics/DynamicFriction", 0.45f);
}
```

`LaziealParameterEditor`はLaziealRuntime側のソリューションに生成される
開発ツール用の実行ファイルです。
初回起動時に選択したディレクトリ内のJSONをImGuiから作成・編集・保存できます。
選択したディレクトリは次回起動時にも引き継がれ、ツールバーの`Change...`から
変更できます。
保存は明示操作でのみ行われ、読み込みに失敗しても現在のドキュメントは保持されます。

LaziealRuntime側でビルドした後、`Tools/DeployParameterEditor.bat`を実行すると、
`Tools/ParameterEditor`へ設定、アセット、DLLを含む実行一式が配置されます。
引数を省略した場合は`Develop`構成が使用されます。別構成を配置する場合は、
`Tools/DeployParameterEditor.bat Debug`のように構成名を指定します。

## ゲームパッドとJoy-Con

ゲームパッド入力にはSDL3を使用しています。既定では左右のJoy-Conを合成せず、
アプリケーションから個別に取得できます。

```ini
[Input.Gamepad]
enabled = true
combineJoyCons = false
verticalJoyCons = true
backgroundInput = false

[Input.Gamepad.JoyCon.Left]
gyroscopeEnabled = true
accelerometerEnabled = true

[Input.Gamepad.JoyCon.Right]
gyroscopeEnabled = true
accelerometerEnabled = true
```

`verticalJoyCons = true`は、左右のJoy-Conを両手に縦持ちする場合の設定です。
Joy-Conを1本ずつ横持ちする場合は`false`に変更してください。
`gyroscopeEnabled`と`accelerometerEnabled`は左右のJoy-Conごとに設定でき、
`false`にすると対応するセンサーを有効化しません。設定を省略した場合は`true`です。

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

## ポイントライト検証シーン

Titleシーンで`P`を押すと、`PointLightScene`へ移動します。
赤、緑、青のポイントライトが白い球体と金属質のボックスの間を周回し、
複数ライトの加算、位置による照明方向、距離減衰を確認できます。

- `Space`: ライトの周回を一時停止・再開
- `R`: ライトの周回位置をリセット
- `Escape`: Titleシーンへ戻る
