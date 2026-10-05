#include "Scene/TitleScene.h"

#include "Scene/SceneNames.h"

using namespace LGF;

void TitleScene::Update() {
	rotation_ += static_cast<float>(System::DeltaTime());
	UpdateCamera();

	if (Key::G.Trigger()) {
		ChangeScene(SceneNames::GamepadTest);
	} else if (Key::Enter.Trigger()) {
		ChangeScene(SceneNames::Game);
	}
}

void TitleScene::Draw() const {
	DrawStage();

	const Quaternion rotation = Math::MakeRotateAxisAngle(
		{ 0.0f, 1.0f, 0.0f },
		rotation_);
	box_.Draw(
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f }, rotation, { -2.5f, 1.0f, 0.0f }),
		Color::RoyalBlue);
	capsule_.Draw(
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f }, rotation, { 0.0f, 1.25f, 0.0f }),
		Color::Gold);
	sphere_.Draw(
		Math::MakeTranslateMatrix({ 2.5f, 1.0f, 0.0f }),
		Color::Turquoise);
}
