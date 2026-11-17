#include "PostProcessScene.h"

#include <stdexcept>

#include <imgui.h>

using namespace LGF;

namespace {

	constexpr const char* kEnvironmentPath =
		"Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.hdr";
	constexpr const char* kProfilePath = "Default.json";
	constexpr const char* kBloomId = "MainBloom";
	constexpr const char* kColorAdjustmentId = "SceneColor";
	constexpr const char* kGrayscaleId = "Grayscale";
	constexpr const char* kPosterizationId = "Posterization";
	constexpr const char* kSepiaId = "Sepia";
	constexpr const char* kColorInversionId = "ColorInversion";
	constexpr const char* kLensDistortionId = "LensDistortion";
	constexpr const char* kRadialBlurId = "RadialBlur";
	constexpr const char* kPixelationId = "Pixelation";
	constexpr const char* kOrderedDitheringId = "OrderedDithering";
	constexpr const char* kChromaticAberrationId = "ChromaticAberration";
	constexpr const char* kVignetteId = "Vignette";
	constexpr const char* kSharpenId = "Sharpen";
	constexpr const char* kFXAAId = "FXAA";

	Matrix4x4 MakeWorldMatrix(const Vector3& scale, const Vector3& translation) {
		return Math::MakeAffineMatrix(
			scale,
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			translation);
	}

}

PostProcessScene::PostProcessScene() :
	camera_(18.0f),
	directionalLight_({ -0.4f, -1.0f, 0.25f }, Color::White, 1.5f),
	warmLight_({ -5.0f, 3.0f, 1.0f }, { 1.0f, 0.25f, 0.05f, 1.0f }, 35.0f, 10.0f),
	coolLight_({ 5.0f, 3.0f, 1.0f }, { 0.05f, 0.35f, 1.0f, 1.0f }, 35.0f, 10.0f),
	accentLight_({ 0.0f, 2.5f, -4.0f }, { 0.4f, 1.0f, 0.2f, 1.0f }, 28.0f, 8.0f),
	skybox_(kEnvironmentPath),
	ground_(28.0f, 24.0f),
	grid_(24.0f, 1.0f),
	sphere_(1.5f),
	box_(2.5f) {

	postProcessProfile_ = Graphics3D::CreatePostProcessProfile(kProfilePath);
	if (!postProcessProfile_.IsValid() ||
		!Graphics3D::SetPostProcessProfile(postProcessProfile_)) {
		throw std::runtime_error("Failed to create the default Post Process Profile.");
	}
	ApplyPostProcessSettings();
}

PostProcessScene::~PostProcessScene() {
	Graphics3D::ResetPostProcessProfile();
	Graphics3D::RemovePostProcessProfile(postProcessProfile_);
}

void PostProcessScene::Update() {
	camera_.Update();
	DrawControlPanel();
	ApplyPostProcessSettings();
}

void PostProcessScene::Draw() {
	camera_.SetScene();
	directionalLight_.SetScene();
	warmLight_.SetScene();
	coolLight_.SetScene();
	accentLight_.SetScene();
	skybox_.SetScene();

	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -1.5f, 0.0f }),
		Material3D{
			.color = { 0.18f, 0.20f, 0.24f, 1.0f },
			.metallic = 0.15f,
			.roughness = 0.35f,
		});
	grid_.Draw(Math::MakeTranslateMatrix({ 0.0f, -1.48f, 0.0f }));

	sphere_.Draw(
		MakeWorldMatrix({ 1.0f, 1.0f, 1.0f }, { -4.0f, 0.0f, 1.0f }),
		Material3D{
			.color = { 1.0f, 0.12f, 0.04f, 1.0f },
			.metallic = 0.35f,
			.roughness = 0.18f,
		});
	sphere_.Draw(
		MakeWorldMatrix({ 1.0f, 1.0f, 1.0f }, { 4.0f, 0.0f, 1.0f }),
		Material3D{
			.color = { 0.04f, 0.25f, 1.0f, 1.0f },
			.metallic = 0.35f,
			.roughness = 0.18f,
		});
	sphere_.Draw(
		MakeWorldMatrix({ 0.75f, 0.75f, 0.75f }, { 0.0f, -0.35f, -3.0f }),
		Material3D{
			.color = { 0.25f, 1.0f, 0.08f, 1.0f },
			.metallic = 0.65f,
			.roughness = 0.12f,
		});

	box_.Draw(
		MakeWorldMatrix({ 1.0f, 2.0f, 1.0f }, { -7.0f, 1.0f, 5.0f }),
		Material3D{
			.color = { 0.55f, 0.12f, 0.06f, 1.0f },
			.roughness = 0.55f,
		});
	box_.Draw(
		MakeWorldMatrix({ 1.0f, 3.0f, 1.0f }, { 0.0f, 2.25f, 6.0f }),
		Material3D{
			.color = { 0.55f, 0.55f, 0.60f, 1.0f },
			.metallic = 0.8f,
			.roughness = 0.2f,
		});
	box_.Draw(
		MakeWorldMatrix({ 1.0f, 1.5f, 1.0f }, { 7.0f, 0.4f, 5.0f }),
		Material3D{
			.color = { 0.05f, 0.18f, 0.62f, 1.0f },
			.roughness = 0.45f,
		});
}

void PostProcessScene::DrawControlPanel() {
	if (!ImGui::Begin("Post Process")) {
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("Arrow keys: orbit camera   Q / E: distance");
	ImGui::Separator();

	ImGui::Checkbox("Bloom", &bloomEnabled_);
	ImGui::SliderFloat("Bloom Threshold", &bloomThreshold_, 0.0f, 10.0f);
	ImGui::SliderFloat("Bloom Soft Knee", &bloomSoftKnee_, 0.0f, 1.0f);
	ImGui::SliderFloat("Bloom Intensity", &bloomIntensity_, 0.0f, 5.0f);
	ImGui::SliderFloat("Bloom Radius", &bloomRadius_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("Color Adjustment", &colorAdjustmentEnabled_);
	ImGui::SliderFloat("Brightness", &brightness_, -1.0f, 1.0f);
	ImGui::SliderFloat("Contrast", &contrast_, 0.0f, 2.0f);
	ImGui::SliderFloat("Saturation", &saturation_, 0.0f, 2.0f);
	ImGui::SliderFloat("Color Intensity", &colorAdjustmentIntensity_, 0.0f, 1.0f);
	ImGui::ColorEdit4("Tint", &tint_.x);

	ImGui::Separator();
	ImGui::Checkbox("Grayscale", &grayscaleEnabled_);
	ImGui::SliderFloat("Grayscale Intensity", &grayscaleIntensity_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("Posterization", &posterizationEnabled_);
	ImGui::SliderFloat("Posterization Color Steps", &posterizationColorSteps_, 2.0f, 64.0f, "%.0f");
	ImGui::SliderFloat("Posterization Intensity", &posterizationIntensity_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("Sepia", &sepiaEnabled_);
	ImGui::SliderFloat("Sepia Intensity", &sepiaIntensity_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("Color Inversion", &colorInversionEnabled_);
	ImGui::SliderFloat("Color Inversion Intensity", &colorInversionIntensity_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("Lens Distortion", &lensDistortionEnabled_);
	ImGui::SliderFloat("Lens Distortion Amount", &lensDistortion_, -1.0f, 1.0f);
	ImGui::SliderFloat("Lens Distortion Scale", &lensDistortionScale_, 0.5f, 2.0f);
	ImGui::SliderFloat2("Lens Distortion Center", &lensDistortionCenter_.x, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("Radial Blur", &radialBlurEnabled_);
	ImGui::SliderFloat2("Radial Blur Center", &radialBlurCenter_.x, 0.0f, 1.0f);
	ImGui::SliderFloat("Radial Blur Strength", &radialBlurStrength_, -0.25f, 0.25f);
	ImGui::SliderFloat("Radial Blur Sample Count", &radialBlurSampleCount_, 2.0f, 32.0f, "%.0f");

	ImGui::Separator();
	ImGui::Checkbox("Pixelation", &pixelationEnabled_);
	ImGui::SliderFloat("Pixel Size", &pixelSize_, 1.0f, 128.0f, "%.0f");

	ImGui::Separator();
	ImGui::Checkbox("Ordered Dithering", &orderedDitheringEnabled_);
	ImGui::SliderFloat("Dithering Color Steps", &orderedDitheringColorSteps_, 2.0f, 64.0f, "%.0f");
	ImGui::SliderFloat("Dithering Intensity", &orderedDitheringIntensity_, 0.0f, 1.0f);
	ImGui::SliderFloat("Dithering Scale", &orderedDitheringScale_, 1.0f, 8.0f, "%.0f");

	ImGui::Separator();
	ImGui::Checkbox("Chromatic Aberration", &chromaticAberrationEnabled_);
	ImGui::SliderFloat("Chromatic Aberration Intensity", &chromaticAberrationIntensity_, 0.0f, 32.0f);

	ImGui::Separator();
	ImGui::Checkbox("Vignette", &vignetteEnabled_);
	ImGui::SliderFloat("Vignette Intensity", &vignetteIntensity_, 0.0f, 1.0f);
	ImGui::SliderFloat("Vignette Smoothness", &vignetteSmoothness_, 0.001f, 1.0f);
	ImGui::SliderFloat("Vignette Roundness", &vignetteRoundness_, 0.0f, 1.0f);
	ImGui::ColorEdit4("Vignette Color", &vignetteColor_.x);

	ImGui::Separator();
	ImGui::Checkbox("Sharpen", &sharpenEnabled_);
	ImGui::SliderFloat("Sharpen Intensity", &sharpenIntensity_, 0.0f, 2.0f);
	ImGui::SliderFloat("Sharpen Threshold", &sharpenThreshold_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::Checkbox("FXAA", &fxaaEnabled_);
	ImGui::SliderFloat("FXAA Edge Threshold", &fxaaEdgeThreshold_, 0.0312f, 0.333f);
	ImGui::SliderFloat("FXAA Minimum Edge Threshold", &fxaaMinimumEdgeThreshold_, 0.0f, 0.0833f);
	ImGui::SliderFloat("FXAA Blend Strength", &fxaaBlendStrength_, 0.0f, 1.0f);

	ImGui::Separator();
	ImGui::SliderFloat("Exposure EV", &exposureEV_, -5.0f, 5.0f);
	if (ImGui::Button("Reset")) {
		ResetPostProcessSettings();
	}

	ImGui::End();
}

void PostProcessScene::ApplyPostProcessSettings() {
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kBloomId, bloomEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kBloomId, "Threshold", bloomThreshold_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kBloomId, "SoftKnee", bloomSoftKnee_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kBloomId, "Intensity", bloomIntensity_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kBloomId, "Radius", bloomRadius_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kColorAdjustmentId, colorAdjustmentEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kColorAdjustmentId, "Brightness", brightness_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kColorAdjustmentId, "Contrast", contrast_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kColorAdjustmentId, "Saturation", saturation_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kColorAdjustmentId, "Intensity", colorAdjustmentIntensity_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kColorAdjustmentId, "Tint", tint_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kGrayscaleId, grayscaleEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kGrayscaleId, "Intensity", grayscaleIntensity_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kPosterizationId, posterizationEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kPosterizationId, "ColorSteps", posterizationColorSteps_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kPosterizationId, "Intensity", posterizationIntensity_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kSepiaId, sepiaEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kSepiaId, "Intensity", sepiaIntensity_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kColorInversionId, colorInversionEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kColorInversionId, "Intensity", colorInversionIntensity_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kLensDistortionId, lensDistortionEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kLensDistortionId, "Distortion", lensDistortion_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kLensDistortionId, "Scale", lensDistortionScale_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kLensDistortionId, "Center", lensDistortionCenter_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kRadialBlurId, radialBlurEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kRadialBlurId, "Center", radialBlurCenter_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kRadialBlurId, "Strength", radialBlurStrength_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kRadialBlurId, "SampleCount", radialBlurSampleCount_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kPixelationId, pixelationEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kPixelationId, "PixelSize", pixelSize_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kOrderedDitheringId, orderedDitheringEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kOrderedDitheringId, "ColorSteps", orderedDitheringColorSteps_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kOrderedDitheringId, "Intensity", orderedDitheringIntensity_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kOrderedDitheringId, "Scale", orderedDitheringScale_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kChromaticAberrationId, chromaticAberrationEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kChromaticAberrationId, "Intensity", chromaticAberrationIntensity_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kVignetteId, vignetteEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kVignetteId, "Intensity", vignetteIntensity_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kVignetteId, "Smoothness", vignetteSmoothness_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kVignetteId, "Roundness", vignetteRoundness_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kVignetteId, "Color", vignetteColor_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kSharpenId, sharpenEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kSharpenId, "Intensity", sharpenIntensity_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kSharpenId, "Threshold", sharpenThreshold_);

	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, kFXAAId, fxaaEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kFXAAId, "EdgeThreshold", fxaaEdgeThreshold_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kFXAAId, "MinimumEdgeThreshold", fxaaMinimumEdgeThreshold_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, kFXAAId, "BlendStrength", fxaaBlendStrength_);
	Graphics3D::SetToneMappingParameter(postProcessProfile_, "ExposureEV", exposureEV_);
}

void PostProcessScene::ResetPostProcessSettings() {
	bloomEnabled_ = true;
	bloomThreshold_ = 1.0f;
	bloomSoftKnee_ = 0.5f;
	bloomIntensity_ = 0.15f;
	bloomRadius_ = 0.7f;
	colorAdjustmentEnabled_ = true;
	brightness_ = 0.0f;
	contrast_ = 1.0f;
	saturation_ = 1.0f;
	colorAdjustmentIntensity_ = 1.0f;
	tint_ = Color::White;
	grayscaleEnabled_ = false;
	grayscaleIntensity_ = 1.0f;
	posterizationEnabled_ = false;
	posterizationColorSteps_ = 8.0f;
	posterizationIntensity_ = 1.0f;
	sepiaEnabled_ = false;
	sepiaIntensity_ = 1.0f;
	colorInversionEnabled_ = false;
	colorInversionIntensity_ = 1.0f;
	lensDistortionEnabled_ = false;
	lensDistortion_ = 0.15f;
	lensDistortionScale_ = 1.0f;
	lensDistortionCenter_ = { 0.5f, 0.5f };
	radialBlurEnabled_ = false;
	radialBlurCenter_ = { 0.5f, 0.5f };
	radialBlurStrength_ = 0.08f;
	radialBlurSampleCount_ = 12.0f;
	pixelationEnabled_ = false;
	pixelSize_ = 8.0f;
	orderedDitheringEnabled_ = false;
	orderedDitheringColorSteps_ = 8.0f;
	orderedDitheringIntensity_ = 1.0f;
	orderedDitheringScale_ = 1.0f;
	chromaticAberrationEnabled_ = false;
	chromaticAberrationIntensity_ = 2.0f;
	vignetteEnabled_ = true;
	vignetteIntensity_ = 0.25f;
	vignetteSmoothness_ = 0.5f;
	vignetteRoundness_ = 1.0f;
	vignetteColor_ = Color::Black;
	fxaaEnabled_ = true;
	fxaaEdgeThreshold_ = 0.125f;
	fxaaMinimumEdgeThreshold_ = 0.0312f;
	fxaaBlendStrength_ = 0.75f;
	sharpenEnabled_ = false;
	sharpenIntensity_ = 0.5f;
	sharpenThreshold_ = 0.02f;
	exposureEV_ = 0.0f;
	Graphics3D::ClearPostProcessOverrides(postProcessProfile_);
}
