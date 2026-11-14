#include "SkinningAnimationScene.h"

#include <format>
#include <stdexcept>

using namespace LGF;

namespace {

	constexpr const char* kFoxModelPath = "Assets/Models/Fox/Fox.glb";
	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	constexpr const char* kEnvironmentPath =
		"Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.hdr";
	constexpr float kFoxScale = 0.025f;
	constexpr uint32_t kWalkColumnCount = 6;
	constexpr uint32_t kWalkRowCount = 4;
	constexpr float kWalkSpacing = 4.0f;

	Matrix4x4 MakeFoxWorldMatrix(const Vector3& translation) {
		return Math::MakeAffineMatrix(
			{ kFoxScale, kFoxScale, kFoxScale },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			translation);
	}

	std::string JoinAnimationNames(const std::vector<std::string>& names) {
		std::string result;
		for (size_t index = 0; index < names.size(); ++index) {
			if (index != 0) {
				result += ", ";
			}
			result += names[index];
		}
		return result;
	}

}

SkinningAnimationScene::SkinningAnimationScene() :
	foxModel_(kFoxModelPath),
	walkAnimator_(foxModel_),
	surveyAnimator_(foxModel_),
	runAnimator_(foxModel_),
	camera_(34.0f),
	directionalLight_({ -0.4f, -1.0f, 0.3f }, Color::White, 2.0f),
	skybox_(kEnvironmentPath),
	ground_(32.0f, 28.0f),
	titleFont_(kFontPath, 32u),
	captionFont_(kFontPath, 18u),
	renderTarget_({
		.width = 640u,
		.height = 360u,
		.clearColor = { 0.04f, 0.06f, 0.08f, 1.0f },
	}),
	renderTargetPreview_(
		renderTarget_.GetTexture(),
		{ 0.0f, 20.0f },
		{ 384.0f, 216.0f }),
	renderPassCamera_(
		{ 0.0f, 5.0f, -16.0f },
		0.0f,
		-8.0f * Math::DEG_TO_RAD),
	animationNames_(JoinAnimationNames(walkAnimator_.GetAnimationNames())) {

	if (!walkAnimator_.Play("Walk", { .loop = true, .fadeDuration = 0.2f }) ||
		!surveyAnimator_.Play("Survey", { .loop = true, .fadeDuration = 0.2f }) ||
		!runAnimator_.Play("Run", {
			.loop = true,
			.playbackSpeed = 1.25f,
			.fadeDuration = 0.2f,
		})) {
		throw std::runtime_error(
			"Fox.glb must contain the Survey, Walk, and Run animations.");
	}

	walkInstances_.reserve(kWalkColumnCount * kWalkRowCount);
	for (uint32_t row = 0; row < kWalkRowCount; ++row) {
		for (uint32_t column = 0; column < kWalkColumnCount; ++column) {
			const float x =
				(static_cast<float>(column) -
				(static_cast<float>(kWalkColumnCount) - 1.0f) * 0.5f) *
				kWalkSpacing;
			const float z =
				(static_cast<float>(row) -
				(static_cast<float>(kWalkRowCount) - 1.0f) * 0.5f) *
				kWalkSpacing + 3.0f;
			const float tint = (row + column) % 2 == 0 ? 1.0f : 0.82f;
			walkInstances_.push_back({
				.worldMatrix = MakeFoxWorldMatrix({ x, 0.0f, z }),
				.color = { 1.0f, tint, tint, 1.0f },
			});
		}
	}
}

SkinningAnimationScene::~SkinningAnimationScene() = default;

void SkinningAnimationScene::Update() {
	camera_.Update();

	const float deltaTime = static_cast<float>(System::DeltaTime());
	walkAnimator_.Update(deltaTime);
	surveyAnimator_.Update(deltaTime);
	runAnimator_.Update(deltaTime);
}

void SkinningAnimationScene::Draw() {
	directionalLight_.SetScene();
	skybox_.SetScene();

	Graphics3D::BeginRenderPass(renderTarget_, renderPassCamera_);
	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -0.02f, 2.0f }),
		Vector4{ 0.12f, 0.16f, 0.20f, 1.0f });
	foxModel_.DrawInstanced(walkInstances_, walkAnimator_);
	foxModel_.Draw(
		MakeFoxWorldMatrix({ -5.0f, 0.0f, -7.0f }),
		surveyAnimator_,
		{ .color = Color::Gold });
	foxModel_.Draw(
		MakeFoxWorldMatrix({ 5.0f, 0.0f, -7.0f }),
		runAnimator_,
		{ .color = Color::SkyBlue });
	Graphics3D::EndRenderPass();

	camera_.SetScene();

	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -0.02f, 2.0f }),
		Vector4{ 0.18f, 0.22f, 0.19f, 1.0f });

	foxModel_.DrawInstanced(walkInstances_, walkAnimator_);
	foxModel_.Draw(
		MakeFoxWorldMatrix({ -5.0f, 0.0f, -7.0f }),
		surveyAnimator_,
		{ .color = Color::Gold });
	foxModel_.Draw(
		MakeFoxWorldMatrix({ 5.0f, 0.0f, -7.0f }),
		runAnimator_,
		{ .color = Color::SkyBlue });

	titleFont_.Draw(
		"Skinning Animation Demo",
		{ 24.0f, 20.0f },
		{ .color = Color::White });
	captionFont_.Draw(
		std::format(
			"{} foxes share one Walk animator / one CS skinning result",
			walkInstances_.size()),
		{ 24.0f, 62.0f },
		{ .color = Color::Lime });
	captionFont_.Draw(
		"Gold: Survey (individual pose)    Blue: Run (individual pose)",
		{ 24.0f, 88.0f },
		{ .color = Color::LightGray });
	captionFont_.Draw(
		"Embedded clips: " + animationNames_,
		{ 24.0f, 114.0f },
		{ .color = Color::LightGray });
	captionFont_.Draw(
		"Arrow keys: orbit    Q / E: camera distance",
		{ 24.0f, 140.0f },
		{ .color = Color::LightGray });

	renderTargetPreview_.leftTop = {
		static_cast<float>(Graphics3D::GetWindowWidth()) - renderTargetPreview_.size.x - 24.0f,
		20.0f };
	renderTargetPreview_.Draw();
	captionFont_.Draw(
		"Custom RenderPass (640 x 360)",
		{ renderTargetPreview_.leftTop.x, renderTargetPreview_.leftTop.y + renderTargetPreview_.size.y + 8.0f },
		{ .color = Color::Cyan });
}
