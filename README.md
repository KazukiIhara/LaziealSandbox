# LaziealSandbox

DirectX 12とC++20で開発している自作ゲームフレームワークのサンプル兼開発環境です。

ゲーム側から扱いやすいAPIと、設定や描画パイプラインを外部ファイルで管理できるData Drivenな構成を目指しています。

現在は開発中です。APIやプロジェクト構成は予告なく変更される場合があります。

## クローンして試す

Gitと、C++のデスクトップ開発・Windows SDK・C++ CMakeツールを含むVisual Studioを用意します。GitHubのテンプレートから新しいリポジトリを作った場合は、そのリポジトリをクローンして同じ手順を使えます。

1. `git clone --recursive <repository-url>` で対象のリポジトリをクローンします。
2. クローン先のルートでターミナルを開き、`.\Setup.bat` を実行します。submoduleとEffekseerを準備して、利用可能なVisual Studio向けのソリューションを生成します。バージョンを指定する場合は `.\Setup.bat vs2022` または `.\Setup.bat vs2026` を実行します。
3. バッチに表示された `Project/LaziealSandbox.slnx` または `Project/LaziealSandbox.sln` を開き、`Develop | x64` で `LaziealSandbox` を起動します。
4. 同じソリューションで `LaziealLevelEditor` を起動し、`Level Preview` から `Level1.json` を開きます。`Open in UI Edit` から、ボタンの文言などを変えて保存できます。
5. `LaziealSandbox` のデモで `R` を押すと、保存した配置を読み込み直します。

## 構成

このリポジトリは、次の3層で構成されています。

- `LaziealSandbox`: 機能の実装例と動作確認を行うアプリケーション
- `LaziealRuntime`: ウィンドウ、入力、音声、時間、シーン、物理などを管理するランタイム
- `LaziealGraphicsFramework`: DirectX 12を使用した描画とGPU処理を担当するグラフィックス基盤

`LaziealRuntime`と`LaziealGraphicsFramework`はGit submoduleとして参照されます。

## 主な機能

### グラフィックス

- 2D・3Dプリミティブ描画と、Mesh Shaderによる3Dインスタンシング
- Deferred RenderingとMetallic-Roughness PBR
- HDR、ACES Filmic Tone Mapping、露出補正
- SkyboxとImage Based Lighting
- 平行光源、ポイントライト、Cascaded Shadow Maps
- glTFなどの3Dモデル読み込み
- スキニングアニメーション、アニメーション切り替え、インスタンシング
- Sprite、2D・3Dフォント、Dear ImGui
- 任意のRender Targetへ描画するカスタムRenderPass
- CPU・GPUパーティクル
- Effekseerエフェクトの同期読み込み、再生、停止、Transform変更
- JSONからのGraphics Pipeline、Compute Pipeline生成

### ランタイム

- Win32ウィンドウとフレームループ
- キーボード、マウス、ゲームパッド、Joy-Con入力
- WAV・MP3音声再生
- Delta Timeと簡易アニメーション
- シーン管理
- 3D衝突判定とCollider管理
- 静的・キネマティック・動的オブジェクトを扱う物理ワールド
- GPUで計算・描画する物理粒子
- 型付きJSONパラメータと専用パラメータエディタ

## 動作環境

- Windows 10以降
- Visual Studio 2022またはVisual Studio 2026
- C++20対応MSVCツールセット
- Windows SDKとDirectX Shader Compiler
- Visual StudioのC++ CMakeツール（初回のEffekseerライブラリ生成に使用）
- DirectX 12対応GPU
  - Mesh Shader Tier 1以上
  - Shader Model 6.6以上
  - Resource Binding Tier 3

## ビルド出力

ビルド結果は次のディレクトリへ出力されます。

```text
generated/outputs/<Configuration>/x64
```

`LaziealParameterEditor.exe` はこの出力先の `ParameterEditor` フォルダに生成されます。

`LaziealLevelEditor.exe` は `LevelEditor` フォルダに生成されます。起動時はトップ画面から `UI Edit`、`World Edit`、`Level Preview` を選べます。各モードで新規作成または既存のJSONを開けます。

`Project/Assets/LevelDatas` の直下に統合レベル、`UI` に画面UI、`World` に2D・3Dの配置ファイルを置きます。サンプルの `Level1.json` は `UI/Level1UI.json` と `World/Level1World.json` を参照します。

`Level Preview` では参照先を差し替えて保存でき、ワールドと画面UIを重ねて確認できます。`Game size` でプレビュー内のゲーム画面サイズを変えられます。`Open in UI Edit` と `Open in World Edit` で参照先を個別に編集できます。UIは階層とギズモで編集でき、`Test buttons` でボタンの反応を確認できます。

ワールド配置ではモデルとスプライトを追加し、`3D` / `2D` プレビューで `Move`、`Rotate`、`Scale` を切り替えて編集できます。3Dプレビューは右ドラッグで回転、ホイールでズームできます。既存の配置サンプルを保存してSandboxで `R` を押すと、変更を読み込み直します。

3種類のデータはいずれもJSONです。ファイルに書き込まれる `Type` は `Level`、`UILayout2D`、`WorldLayout` です。新規保存時の名前にはそれぞれ `.level.json`、`.ui.json`、`.world.json` が提案されます。

ゲームでは `LGF::SetLevelData("Level1.json")` で、参照先のUIと3D配置がRuntimeのフレーム処理で描画されます。3D配置の各オブジェクトは `Transform3D` と `ModelRenderer` を組み合わせて定義できます。

## 基本的な使い方

アプリケーション側では`LGF/LGF.h`をインクルードし、`Main()`を定義します。エントリーポイントと初期化・終了処理はランタイムが管理します。

```cpp
#include <LGF/LGF.h>

void Main() {
    while (LGF::System::Update()) {
        // 更新処理
        // 描画処理
    }
}
```

Sandboxの実装は`Project/main.cpp`と`Project/Src`にあります。

## 現在のサンプル

Sandboxの起動時には `LevelEditorDemoScene` が開きます。`Level1.json` の3D配置とUIを読み込み、テクスチャ付きの箱を2つ表示します。

- `矢印キー` でカメラを回転
- `Q` / `E` でカメラ距離を変更
- `1` で3D配置、`2` で2Dスプライトのピボット確認に切り替え
- `R` でレベルを再読み込み

2Dサンプルは `Project/Assets/LevelDatas/World/Level2DWorld.json` にあります。黄色い点が各スプライトのピボット位置です。

描画、物理、エフェクトをまとめて試す `ShowcaseScene` とスキニング専用のサンプルは `Project/Src/Scene` にあります。

## ランタイム設定

起動設定は`Project/RuntimeSetting.ini`で管理します。

```ini
[Runtime]
assetRoot = ../Dependencies/LaziealRuntime/Dependencies/LaziealGraphicsFramework/Project/Assets

[Graphics]
vsync = true
multiFrameContextEnabled = true
gpuBasedValidationEnabled = false

[Audio]
enabled = true
masterVolume = 1

[Window]
title = Lazieal Sandbox
width = 1280
height = 720
fullscreen = false
```

同じファイルからゲームパッド、Joy-Con、スティックのデッドゾーン、モーションセンサーも設定できます。

設定ファイルが存在しない場合はデフォルト値で生成されます。項目が追加された場合も、読み込み後に現在の形式へ更新されます。

## Data Drivenな設定

描画パイプラインは次のディレクトリにあるJSONから生成されます。

```text
Dependencies/LaziealRuntime/Dependencies/LaziealGraphicsFramework/Project/Assets/JsonData/Pipeline
```

ゲーム固有のパラメータは`LGF::Parameter`または`LGF::ParameterDocument`でJSONから読み込めます。

```cpp
const float speed = LGF::Parameter::GetOr<float>(
    "Player.json",
    "Movement/Speed",
    1.0f);
```

対応する型は`bool`、`int32_t`、`float`、`Vector2`、`Vector3`、`Vector4`、`std::string`です。

パラメータの作成と編集には、LaziealRuntimeに付属する `LaziealParameterEditor` を使えます。Sandboxは同じソリューションに独立した実行プロジェクトとして組み込み、ソリューションを生成したクローン内のパラメータフォルダを起動時に指定します。エディタはSandboxが読み込むJSONを直接編集します。

エディタを単体で起動するときは、`LaziealParameterEditor.exe --parameter-directory "<フォルダのパス>"` で編集先を指定できます。引数を省略した場合は前回選んだフォルダが開き、初回はフォルダを選択できます。

## ディレクトリ

```text
LaziealSandbox
├─ Project
│  ├─ Assets              Sandbox用アセット
│  ├─ Src                 Sandboxのソースコード
│  ├─ RuntimeSetting.ini  起動設定
│  └─ Premake             プロジェクト生成設定
├─ Dependencies
│  └─ LaziealRuntime
│     └─ Dependencies
│        └─ LaziealGraphicsFramework
├─ Tests                  テストコード
├─ Tools                  開発用ツール
└─ generated              ビルド生成物
```

## 主な依存ライブラリ

- DirectXTex
- DirectXMesh
- Dear ImGui
- Assimp
- nlohmann/json
- SDL3
- Effekseer 1.80.8

各サンプルアセットの出典とライセンスは、アセットと同じディレクトリにある`SOURCE.md`やライセンスファイルを参照してください。
