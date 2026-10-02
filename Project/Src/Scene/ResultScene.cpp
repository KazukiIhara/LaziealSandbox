#include "Scene/ResultScene.h"

#include <algorithm>
#include <cmath>

#include "Scene/SceneNames.h"

using namespace LGF;

void ResultScene::Update() {
	rotation_ += static_cast<float>(System::DeltaTime());
	UpdateCamera();

	if (Key::Enter.Trigger()) {
		ChangeScene(SceneNames::Title);
	} else if (Key::R.Trigger()) {
		ChangeScene(SceneNames::Game);
	}
}

void ResultScene::Draw() const {
	DrawStage();
	pedestal_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, 0.35f, 0.0f }),
		Color::RoyalBlue);

	const int markerCount = std::clamp(GetData().collisionCount, 0, 8);
	if (markerCount == 0) {
		marker_.Draw(
			Math::MakeTranslateMatrix({ 0.0f, 1.4f, 0.0f }),
			Color::Gray);
		return;
	}

	for (int index = 0; index < markerCount; ++index) {
		const float angle = rotation_ + Math::PI * 2.0f *
			static_cast<float>(index) / static_cast<float>(markerCount);
		const Vector3 position{
			std::cos(angle) * 2.0f,
			1.5f,
			std::sin(angle) * 2.0f,
		};
		marker_.Draw(Math::MakeTranslateMatrix(position), Color::Gold);
	}
}
