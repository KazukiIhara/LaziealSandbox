#include "Scene/PointLightScene.h"

#include <array>
#include <cmath>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kLightOrbitRadius = 4.0f;
	constexpr float kLightHeight = 2.5f;
	constexpr float kLightAngularSpeed = 35.0f * Math::DEG_TO_RAD;

	constexpr std::array<Vector3, 10> kObjectPositions{
		Vector3{ -6.0f, 0.8f, -2.5f },
		Vector3{ -3.0f, 0.8f, -2.5f },
		Vector3{  0.0f, 0.8f, -2.5f },
		Vector3{  3.0f, 0.8f, -2.5f },
		Vector3{  6.0f, 0.8f, -2.5f },
		Vector3{ -6.0f, 0.8f,  2.5f },
		Vector3{ -3.0f, 0.8f,  2.5f },
		Vector3{  0.0f, 0.8f,  2.5f },
		Vector3{  3.0f, 0.8f,  2.5f },
		Vector3{  6.0f, 0.8f,  2.5f },
	};

}

void PointLightScene::Update() {
	UpdateCamera();

	if (Key::Space.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::R.Trigger()) {
		lightAngle_ = 0.0f;
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	if (!isPaused_) {
		lightAngle_ += kLightAngularSpeed * static_cast<float>(System::DeltaTime());
	}
	pointLight_.position = {
		std::cos(lightAngle_) * kLightOrbitRadius,
		kLightHeight,
		std::sin(lightAngle_) * kLightOrbitRadius,
	};
}

void PointLightScene::Draw() const {
	DrawStage(fillLight_);
	pointLight_.SetScene();

	const Material3D sphereMaterial{
		.color = Color::White,
		.metallic = 0.0f,
		.roughness = 0.65f,
	};
	const Material3D boxMaterial{
		.color = Color::LightGray,
		.metallic = 0.75f,
		.roughness = 0.25f,
	};
	for (std::size_t index = 0; index < kObjectPositions.size(); ++index) {
		const Matrix4x4 worldMatrix = Math::MakeTranslateMatrix(kObjectPositions[index]);
		if (index % 2u == 0u) {
			sphere_.Draw(worldMatrix, sphereMaterial);
		} else {
			box_.Draw(worldMatrix, boxMaterial);
		}
	}

	lightMarker_.Draw(
		Math::MakeTranslateMatrix(pointLight_.position),
		Material3D{
			.color = pointLight_.color,
			.unlit = true,
		});

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Point Light Demo")) {
		ImGui::TextUnformatted("PointLight::SetScene() is called every frame.");
		ImGui::TextUnformatted("Space: pause orbit | R: reset orbit | Escape: title");
		ImGui::Separator();
		ImGui::Text(
			"Position: (%+.2f, %+.2f, %+.2f)",
			pointLight_.position.x,
			pointLight_.position.y,
			pointLight_.position.z);
		ImGui::Text("Intensity: %.2f | Range: %.2f", pointLight_.intensity, pointLight_.range);
		ImGui::Text("Orbit: %s", isPaused_ ? "Paused" : "Playing");
	}
	ImGui::End();
}
