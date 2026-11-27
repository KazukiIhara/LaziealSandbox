#include "LevelEditorDemoScene.h"

using namespace LGF;

namespace {

	constexpr std::string_view kLevel3DFile = "Level1.json";
	constexpr std::string_view kLevel2DFile = "Level2D.level.json";
	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";

}

LevelEditorDemoScene::LevelEditorDemoScene() :
	camera_(8.5f),
	directionalLight_({ -0.45f, -1.0f, 0.35f }, Color::White, 2.0f),
	ground_(18.0f, 18.0f),
	font_(kFontPath, 18u),
	levelFile_(kLevel3DFile) {
	LoadLevel(levelFile_);
}

void LevelEditorDemoScene::Update() {
	camera_.Update();
	if (Key::Num1.Trigger()) {
		LoadLevel(kLevel3DFile);
	} else if (Key::Num2.Trigger()) {
		LoadLevel(kLevel2DFile);
	} else if (Key::R.Trigger()) {
		LoadLevel(levelFile_);
	}
}

void LevelEditorDemoScene::LoadLevel(std::string_view fileName) {
	levelResult_ = SetLevelData(fileName);
	if (levelResult_) {
		levelFile_ = fileName;
	}
}

void LevelEditorDemoScene::Draw() {
	camera_.SetScene();
	directionalLight_.SetScene();
	if (levelFile_ == kLevel3DFile) {
		ground_.Draw(
			Math::MakeTranslateMatrix({ 0.0f, -0.55f, 0.0f }),
			Material3D{
				.color = { 0.16f, 0.21f, 0.28f, 1.0f },
				.roughness = 0.9f,
			});
	} else if (const GameObjectWorld* world = GetLevelGameObjectWorld()) {
		for (const char* id : { "CenterPivot", "BottomPivot" }) {
			const GameObject* object = world->Get(world->Find(id));
			const Transform2D* transform = object ? object->GetComponent<Transform2D>() : nullptr;
			if (transform) {
				Circle2D{ transform->translation, 5.0f }.Draw(Color::Gold);
			}
		}
	}

	font_.Draw(std::string("Level Editor Demo / ").append(levelFile_), { 20.0f, 20.0f });
	font_.Draw("1: 3D   2: 2D   Arrow keys: orbit   Q / E: distance   R: reload", { 20.0f, 50.0f });
	if (!levelResult_) {
		font_.Draw(levelResult_.message, { 20.0f, 84.0f }, { .color = Color::Red });
	} else if (const GameObjectWorld* world = GetLevelGameObjectWorld()) {
		font_.Draw("GameObjects: " + std::to_string(world->GetObjectCount()), { 20.0f, 84.0f });
	}
}
