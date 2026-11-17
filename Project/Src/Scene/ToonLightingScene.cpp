#include "ToonLightingScene.h"

#include <imgui.h>

using namespace LGF;

ToonLightingScene::ToonLightingScene() :
	camera_(18.0f),
	directionalLight_({ -0.55f, -1.0f, 0.35f }, Color::White, 1.0f),
	ground_(26.0f, 20.0f),
	sphere_(1.7f),
	capsule_(1.1f, 3.2f),
	cylinder_(1.3f, 3.5f),
	cone_(1.5f, 3.5f),
	box_(3.0f) {

	Graphics3D::ResetPostProcessProfile();
	Graphics3D::SetExposure(0.0f);
	ResetLightingSettings();
}

ToonLightingScene::~ToonLightingScene() = default;

void ToonLightingScene::Update() {
	camera_.Update();
	DrawControlPanel();
}

void ToonLightingScene::Draw() {
	camera_.SetScene();
	directionalLight_.SetScene();
	toonLighting_.SetScene();

	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -2.0f, 1.0f }),
		Material3D{
			.color = { 0.82f, 0.78f, 0.70f, 1.0f },
			.roughness = 0.85f,
		});

	sphere_.Draw(
		Math::MakeTranslateMatrix({ -6.0f, 0.0f, 0.0f }),
		Material3D{
			.color = { 0.95f, 0.25f, 0.18f, 1.0f },
			.roughness = 0.65f,
			.shadingModel = ShadingModel3D::PBR,
		});
	capsule_.Draw(
		Math::MakeTranslateMatrix({ -2.5f, 0.0f, 0.0f }),
		Material3D{
			.color = { 0.95f, 0.68f, 0.12f, 1.0f },
			.roughness = 0.65f,
			.shadingModel = ShadingModel3D::Toon,
			.toonLighting = warmToonMaterial_,
		});
	cylinder_.Draw(
		Math::MakeTranslateMatrix({ 1.0f, 0.0f, 0.0f }),
		Material3D{
			.color = { 0.16f, 0.72f, 0.45f, 1.0f },
			.roughness = 0.65f,
		});
	cone_.Draw(
		Math::MakeTranslateMatrix({ 4.5f, 0.0f, 0.0f }),
		Material3D{
			.color = { 0.20f, 0.48f, 0.95f, 1.0f },
			.roughness = 0.65f,
			.shadingModel = ShadingModel3D::Toon,
			.toonLighting = coolToonMaterial_,
		});
	box_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, 0.0f, 5.0f }),
		Material3D{
			.color = { 0.65f, 0.28f, 0.88f, 1.0f },
			.roughness = 0.65f,
		});
}

void ToonLightingScene::DrawControlPanel() {
	if (!ImGui::Begin("Toon Lighting")) {
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("Arrow keys: orbit camera   Q / E: distance");
	ImGui::TextUnformatted("Sphere: PBR   Capsule / Cone: Material Toon");
	ImGui::TextUnformatted("Others: Scene Default");
	ImGui::Separator();
	ImGui::Checkbox("Enabled", &toonLighting_.enabled);
	ImGui::ColorEdit3("Shadow Color", &toonLighting_.shadowColor.x);
	ImGui::ColorEdit3("Middle Color", &toonLighting_.middleColor.x);
	ImGui::ColorEdit3("Light Color", &toonLighting_.lightColor.x);
	ImGui::SliderFloat(
		"Shadow Threshold",
		&toonLighting_.shadowThreshold,
		0.0f,
		toonLighting_.lightThreshold);
	ImGui::SliderFloat(
		"Light Threshold",
		&toonLighting_.lightThreshold,
		toonLighting_.shadowThreshold,
		1.0f);
	ImGui::SliderFloat("Smoothness", &toonLighting_.smoothness, 0.0f, 0.25f);

	if (ImGui::TreeNode("Warm Material Toon")) {
		ImGui::ColorEdit3("Warm Shadow", &warmToonMaterial_.shadowColor.x);
		ImGui::ColorEdit3("Warm Middle", &warmToonMaterial_.middleColor.x);
		ImGui::ColorEdit3("Warm Light", &warmToonMaterial_.lightColor.x);
		ImGui::SliderFloat(
			"Warm Shadow Threshold",
			&warmToonMaterial_.shadowThreshold,
			0.0f,
			warmToonMaterial_.lightThreshold);
		ImGui::SliderFloat(
			"Warm Light Threshold",
			&warmToonMaterial_.lightThreshold,
			warmToonMaterial_.shadowThreshold,
			1.0f);
		ImGui::SliderFloat("Warm Smoothness", &warmToonMaterial_.smoothness, 0.0f, 0.25f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Cool Material Toon")) {
		ImGui::ColorEdit3("Cool Shadow", &coolToonMaterial_.shadowColor.x);
		ImGui::ColorEdit3("Cool Middle", &coolToonMaterial_.middleColor.x);
		ImGui::ColorEdit3("Cool Light", &coolToonMaterial_.lightColor.x);
		ImGui::SliderFloat(
			"Cool Shadow Threshold",
			&coolToonMaterial_.shadowThreshold,
			0.0f,
			coolToonMaterial_.lightThreshold);
		ImGui::SliderFloat(
			"Cool Light Threshold",
			&coolToonMaterial_.lightThreshold,
			coolToonMaterial_.shadowThreshold,
			1.0f);
		ImGui::SliderFloat("Cool Smoothness", &coolToonMaterial_.smoothness, 0.0f, 0.25f);
		ImGui::TreePop();
	}

	ImGui::Separator();
	ImGui::SliderFloat("Directional Intensity", &directionalLight_.intensity, 0.0f, 3.0f);
	ImGui::ColorEdit3("Directional Color", &directionalLight_.color.x);

	ImGui::Separator();
	if (ImGui::Button("Reset")) {
		ResetLightingSettings();
	}

	ImGui::End();
}

void ToonLightingScene::ResetLightingSettings() {
	toonLighting_ = ToonLighting{};
	warmToonMaterial_ = ToonMaterial3D{
		.shadowColor = { 0.28f, 0.10f, 0.08f, 1.0f },
		.middleColor = { 0.85f, 0.46f, 0.18f, 1.0f },
		.lightColor = { 1.0f, 0.92f, 0.68f, 1.0f },
		.shadowThreshold = 0.30f,
		.lightThreshold = 0.68f,
		.smoothness = 0.015f,
	};
	coolToonMaterial_ = ToonMaterial3D{
		.shadowColor = { 0.08f, 0.14f, 0.32f, 1.0f },
		.middleColor = { 0.20f, 0.55f, 0.86f, 1.0f },
		.lightColor = { 0.78f, 0.94f, 1.0f, 1.0f },
		.shadowThreshold = 0.42f,
		.lightThreshold = 0.76f,
		.smoothness = 0.035f,
	};
	directionalLight_.direction = { -0.55f, -1.0f, 0.35f };
	directionalLight_.color = Color::White;
	directionalLight_.intensity = 1.0f;
}
