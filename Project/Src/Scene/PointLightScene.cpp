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
	for (std::size_t index = 0; index < pointLights_.size(); ++index) {
		const float angle = lightAngle_ + Math::PI * 2.0f *
			static_cast<float>(index) / static_cast<float>(pointLights_.size());
		pointLights_[index].position = {
			std::cos(angle) * kLightOrbitRadius,
			kLightHeight,
			std::sin(angle) * kLightOrbitRadius,
		};
	}
}

void PointLightScene::Draw() const {
	DrawStage(fillLight_);
	for (const PointLight& pointLight : pointLights_) {
		pointLight.SetScene();
	}

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

	for (const PointLight& pointLight : pointLights_) {
		lightMarker_.Draw(
			Math::MakeTranslateMatrix(pointLight.position),
			Material3D{
				.color = pointLight.color,
				.unlit = true,
			});
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Point Light Demo")) {
		ImGui::Text("%zu PointLight::SetScene() calls per frame.", pointLights_.size());
		ImGui::TextUnformatted("Space: pause orbit | R: reset orbit | Escape: title");
		ImGui::Separator();
		ImGui::Text("Maximum point lights: %u", PointLight::MaxCount);
		ImGui::Text(
			"Intensity: %.2f | Range: %.2f",
			pointLights_.front().intensity,
			pointLights_.front().range);
		ImGui::Text("Orbit: %s", isPaused_ ? "Paused" : "Playing");
	}
	ImGui::End();
}
