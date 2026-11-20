#include "ShowcaseScene.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <stdexcept>

#include <imgui.h>

using namespace LGF;

namespace {

	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	constexpr const char* kEnvironmentPath =
		"Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.hdr";
	constexpr const char* kFoxModelPath = "Assets/Models/Fox/Fox.glb";
	constexpr const char* kCheckerTexturePath = "Assets/Textures/Checker.bmp";
	constexpr const char* kEffectPath = "Assets/Effects/Effekseer/Laser01.efkefc";
	constexpr const char* kParticleWorldPath = "Assets/Parameters/ParticleWorld.json";
	constexpr const char* kParticleEmitterPath = "Assets/Parameters/ParticleEmitter.json";
	constexpr const char* kPostProcessProfilePath = "Default.json";

	constexpr const char* kSpawnCommand = "Showcase.SpawnPhysicsBall";
	constexpr const char* kEffectCommand = "Showcase.PlayEffect";
	constexpr float kFoxScale = 0.025f;

	Matrix4x4 MakeWorldMatrix(const Vector3& scale, const Vector3& translation) {
		return Math::MakeAffineMatrix(scale, Quaternion{}, translation);
	}

	Matrix4x4 MakeWorldMatrix(const Vector3& scale, float rotationY, const Vector3& translation) {
		return Math::MakeAffineMatrix(
			scale,
			Math::MakeRotateAxisAngle({ 0.0f, 1.0f, 0.0f }, rotationY),
			translation);
	}

}

ShowcaseScene::ShowcaseScene() :
	camera_(Parameter::GetOr<float>("ShowcaseScene.json", "Camera/Distance", 24.0f)),
	renderTargetCamera_({ 0.0f, 2.8f, -8.0f }, 0.0f, -8.0f * Math::DEG_TO_RAD),
	directionalLight_(
		{ -0.45f, -1.0f, 0.30f },
		Color::White,
		Parameter::GetOr<float>("ShowcaseScene.json", "Lighting/DirectionalIntensity", 1.5f)),
	pointLight_({ 0.0f, 4.0f, -1.0f }, { 0.15f, 0.55f, 1.0f, 1.0f }, 24.0f, 14.0f),
	skybox_(kEnvironmentPath),
	ground_(32.0f, 26.0f),
	grid_(26.0f, 1.0f),
	sphere_(1.0f),
	box_(2.0f),
	capsule_(0.8f, 2.4f),
	collisionLink_({ -3.0f, 1.0f, -1.0f }, { 1.0f, 1.0f, -1.0f }),
	foxModel_(kFoxModelPath),
	foxAnimator_(foxModel_),
	renderTarget_({
		.width = 480u,
		.height = 270u,
		.clearColor = { 0.025f, 0.04f, 0.07f, 1.0f },
	}),
	renderTargetPreview_(renderTarget_.GetTexture(), { 0.0f, 0.0f }, { 320.0f, 180.0f }),
	titleFont_(kFontPath, 28u),
	uiFont_(kFontPath, 18u),
	worldFont_(kFontPath, 48u),
	checkerTexture_(kCheckerTexturePath),
	particleWorld_(VFX::LoadParticleWorldSettings(kParticleWorldPath)),
	gpuParticleWorld_(VFX::LoadParticleWorldSettings(kParticleWorldPath)),
	pointLightIntensity_(Parameter::GetOr<float>("ShowcaseScene.json", "Lighting/PointIntensity", 24.0f)),
	particleEmissionRate_(Parameter::GetOr<float>("ShowcaseScene.json", "VFX/EmissionRate", 120.0f)),
	bloomIntensity_(Parameter::GetOr<float>("ShowcaseScene.json", "PostProcess/BloomIntensity", 0.18f)),
	vignetteIntensity_(Parameter::GetOr<float>("ShowcaseScene.json", "PostProcess/VignetteIntensity", 0.2f)),
	physicsParticleRadius_(Parameter::GetOr<float>("ShowcaseScene.json", "Physics/ParticleRadius", 0.12f)),
	physicsSpawnCount_(Parameter::GetOr<int32_t>("ShowcaseScene.json", "Physics/InitialBallCount", 8)),
	physicsParticleSpawnCount_(Parameter::GetOr<int32_t>("ShowcaseScene.json", "Physics/InitialParticleCount", 3000)) {

	// Animatorはモデル内のクリップ名を指定して再生します。
	if (!foxAnimator_.Play("Walk", { .loop = true, .fadeDuration = 0.2f })) {
		throw std::runtime_error("Fox.glb must contain the Walk animation.");
	}
	// Toonはポストエフェクトではなくマテリアルのライティング方式です。
	// 初期状態はPBRとし、ImGuiからシーン既定の方式をToonへ切り替えられます。
	toonLighting_.enabled = false;

	// 同じAnimatorを共有するインスタンスは、スキニング結果を一度だけ計算してまとめて描画できます。
	for (int index = 0; index < 5; ++index) {
		const float x = (static_cast<float>(index) - 2.0f) * 2.0f;
		const float tint = index % 2 == 0 ? 1.0f : 0.72f;
		foxInstances_.push_back({
			.worldMatrix = MakeWorldMatrix(
				{ kFoxScale, kFoxScale, kFoxScale },
				Math::PI,
				{ x, -1.45f, -5.0f }),
			.color = { 1.0f, tint, 0.35f, 1.0f },
		});
	}

	// ParticleEmitterの初期値もJSONから読み込みます。ImGuiでの変更はこの実行中だけ有効です。
	VFX::ParticleEmitterDesc emitterDesc = VFX::LoadParticleEmitterDesc(kParticleEmitterPath);
	emitterDesc.position = { 0.0f, 0.0f, 1.5f };
	emitterDesc.emissionRate = particleEmissionRate_;
	particleEmitter_ = particleWorld_.CreateEmitter(emitterDesc);

	// GPU版は同じ設定構造体を使用できるため、CPU版との切り替えも簡単です。
	emitterDesc.position = { 3.0f, 0.0f, 1.5f };
	emitterDesc.startColor = { 0.15f, 0.8f, 2.0f, 1.0f };
	emitterDesc.endColor = { 0.05f, 0.1f, 1.0f, 0.0f };
	gpuParticleEmitter_ = gpuParticleWorld_.CreateEmitter(emitterDesc);

	laserEffect_ = effectWorld_.Load(kEffectPath);

	InitializeInput();
	InitializeUI();
	InitializeCollision();
	InitializePhysics();
	InitializePostProcess();
}

ShowcaseScene::~ShowcaseScene() {
	// グローバルな登録物は、シーン破棄時に必ず元へ戻します。
	Input::UnregisterCommand(kSpawnCommand);
	Input::UnregisterCommand(kEffectCommand);
	Graphics3D::ResetPostProcessProfile();
	Graphics3D::RemovePostProcessProfile(postProcessProfile_);
}

void ShowcaseScene::Update() {
	const float deltaTime = static_cast<float>(System::DeltaTime());
	elapsedTime_ += deltaTime;

	// ImGui操作中もカメラ入力を受けるとスライダー操作と競合するため、キーボード占有中は止めます。
	if (!ImGui::GetIO().WantCaptureKeyboard) {
		camera_.Update();
	}

	foxAnimator_.Update(deltaTime * animationSpeed_);
	UpdateUI();
	UpdateCollision();
	UpdatePhysics(deltaTime);
	UpdateVFX(deltaTime);
	DrawControlPanel();
	ApplyPostProcessSettings();
}

void ShowcaseScene::Draw() {
	// RenderTargetへの描画を先に完了させ、そのテクスチャを後段の2D描画で利用します。
	DrawRenderTarget();
	DrawWorld();
	if (show2DOverlay_) {
		DrawOverlay2D();
	}
}

void ShowcaseScene::InitializeInput() {
	// InputCommandを使うと、ゲーム側はデバイス固有のキーを意識せず操作名だけ参照できます。
	Input::RegisterCommand(kSpawnCommand, { Key::Space, Gamepad{ 0u }.Button(GamepadButtonCode::South) });
	Input::RegisterCommand(kEffectCommand, { Key::V, Gamepad{ 0u }.Button(GamepadButtonCode::West) });
}

void ShowcaseScene::InitializeUI() {
	// UIWorldへカメラを登録すると、World CanvasのBillboard系表示も同じAPIで扱えます。
	// FixedなCanvasでも、3D Labelの回転とパネル表面への深度オフセットはUIWorldが解決します。
	uiWorld_.SetCamera(&camera_);

	// CanvasがScreen / Worldの空間を決めます。子要素側は低レベルな描画クラスを直接呼びません。
	screenCanvas_ = uiWorld_.AddCanvas({ .space = UICanvasSpace::Screen });
	const UIHandle informationPanel = uiWorld_.AddPanel({
		.element = { .parent = screenCanvas_ },
		.transform = {
			.position = { 18.0f, -150.0f },
			.size = { 360.0f, 132.0f },
			.anchor = { 0.0f, 1.0f },
		},
		.color = { 0.025f, 0.04f, 0.075f, 0.88f },
	});
	uiWorld_.AddImage({
		.element = { .parent = informationPanel },
		.transform = { .position = { 12.0f, 18.0f }, .size = { 96.0f, 96.0f } },
		.texture = checkerTexture_,
	});
	uiWorld_.AddLabel({
		.element = { .parent = informationPanel },
		.text = "2D / Image / Label",
		.transform = { .position = { 167.0f, 26.0f } },
		.font = titleFont_,
		.textAnchor = {},
	});
	uiWorld_.AddLabel({
		.element = { .parent = informationPanel },
		.text = "SPACE: Physics   V: VFX",
		.transform = { .position = { 167.0f, 72.0f } },
		.font = uiFont_,
		.color = Color::LightGray,
		.textAnchor = {},
	});
	uiWorld_.AddLabel({
		.element = { .parent = informationPanel },
		.text = "TAB: UI navigation",
		.transform = { .position = { 167.0f, 100.0f } },
		.font = uiFont_,
		.color = Color::LightGray,
		.textAnchor = {},
	});

	// anchorはCanvas上の基準位置、pivotはボタン自身の基準位置です。
	spawnButton_ = uiWorld_.AddButton({
		.element = { .parent = screenCanvas_ },
		.text = "Spawn physics ball",
		.transform = {
			.position = { -24.0f, -104.0f },
			.size = { 210.0f, 38.0f },
			.anchor = { 1.0f, 1.0f },
			.pivot = { 1.0f, 1.0f },
		},
		.font = uiFont_,
		.shortcutCommand = kSpawnCommand,
		.isNavigationEnabled = true,
	});
	effectButton_ = uiWorld_.AddButton({
		.element = { .parent = screenCanvas_ },
		.text = "Play Effekseer",
		.transform = {
			.position = { -24.0f, -58.0f },
			.size = { 210.0f, 38.0f },
			.anchor = { 1.0f, 1.0f },
			.pivot = { 1.0f, 1.0f },
		},
		.font = uiFont_,
		.shortcutCommand = kEffectCommand,
		.isNavigationEnabled = true,
	});
	const UIHandle worldCanvas = uiWorld_.AddCanvas({
		.space = UICanvasSpace::World,
		.transform3D = {
			.position = { 5.5f, 2.8f, 3.5f },
		},
	});
	worldButton_ = uiWorld_.AddButton({
		.element = { .parent = worldCanvas },
		.text = "3D UI",
		.transform3D = {
			.size = { 2.8f, 0.85f },
		},
		.font3D = worldFont_,
		.isNavigationEnabled = true,
	});
	uiWorld_.SetSelected(spawnButton_);
}

void ShowcaseScene::InitializeCollision() {
	colliderWorld_.RegisterTags({ "Fixed", "Moving" });
	fixedCollider_ = colliderWorld_.Add(
		Collision::Sphere{ .center = { -3.0f, 1.0f, -1.0f }, .radius = 1.2f },
		{ .tag = "Fixed", .targetTags = { "Moving" } });
	movingCollider_ = colliderWorld_.Add(
		Collision::Sphere{ .center = { 1.0f, 1.0f, -1.0f }, .radius = 1.0f },
		{ .tag = "Moving", .targetTags = { "Fixed" } });
}

void ShowcaseScene::InitializePhysics() {
	// 見た目のGround3Dとは別に、衝突計算用の静的AABBを登録します。
	physicsGround_ = physicsWorld_.AddStaticObject(
		Collision::AABB{
			.min = { 4.0f, -2.0f, -5.0f },
			.max = { 11.0f, -1.0f, 5.0f },
		},
		{ .material = { .restitution = 0.7f, .friction = 0.35f } });

	// Kinematicは物理演算から力を受けず、ゲーム側で動かす床や障害物に向いています。
	kinematicPlatform_ = physicsWorld_.AddKinematicObject(
		Collision::AABB{
			.min = { 6.0f, 0.5f, -3.0f },
			.max = { 9.0f, 0.8f, 0.0f },
		},
		{ .material = { .restitution = 0.45f, .friction = 0.6f } });

	// PhysicsParticleの位置はGPU上に常駐します。RenderGroupを指定するとReadbackなしで直接描画できます。
	physicsParticleRenderGroup_ = physicsWorld_.CreateParticleRenderGroup({
		.visual = Physics::PhysicsParticleSphereVisual{},
		.color = { 0.15f, 0.85f, 1.0f, 1.0f },
	});
	ResetPhysicsBalls();
	SpawnPhysicsParticles();
}

void ShowcaseScene::InitializePostProcess() {
	postProcessProfile_ = Graphics3D::CreatePostProcessProfile(kPostProcessProfilePath);
	if (!postProcessProfile_.IsValid() || !Graphics3D::SetPostProcessProfile(postProcessProfile_)) {
		throw std::runtime_error("Failed to create the default Post Process Profile.");
	}
	ApplyPostProcessSettings();
}

void ShowcaseScene::UpdateUI() {
	// 親Canvasの表示状態は、デフォルト設定のまま子要素へまとめて継承されます。
	uiWorld_.SetVisible(screenCanvas_, show2DOverlay_);
	UIWorldInput input{
		.cursorPosition = Cursor::Pos(),
		.isCursorPressed = Mouse::Left.Press(),
		.selectionChange = Key::Tab.Trigger() ? UISelectionChange::Next : UISelectionChange::None,
		.isNavigationPressed = Key::Enter.Press(),
		.isCursorEnabled = !ImGui::GetIO().WantCaptureMouse,
		.cursor3D = MakeUICursor3D(camera_, Cursor::Pos(), 100.0f),
	};
	uiWorld_.Update(input);

	if (uiWorld_.Trigger(spawnButton_)) {
		SpawnPhysicsBall();
	}
	if (uiWorld_.Trigger(effectButton_) || uiWorld_.Trigger(worldButton_)) {
		PlayEffect();
	}
}

void ShowcaseScene::UpdateCollision() {
	// 一方の球を往復させ、Enter / Stay / ExitをColliderWorldに判定させます。
	const Vector3 movingCenter{
		-0.6f + std::sin(elapsedTime_ * 1.2f) * 3.2f,
		1.0f,
		-1.0f,
	};
	colliderWorld_.SetShape(
		movingCollider_,
		Collision::Sphere{ .center = movingCenter, .radius = 1.0f });
	colliderWorld_.Step();
	collisionDetected_ = colliderWorld_.IsColliding(movingCollider_);
	collisionLink_.end = movingCenter;
}

void ShowcaseScene::UpdatePhysics(float deltaTime) {
	if (physicsEnabled_) {
		// Kinematic形状をUpdate前に更新すると、移動速度も衝突応答へ反映されます。
		kinematicPlatformHeight_ = 1.2f + std::sin(elapsedTime_ * 1.4f) * 0.8f;
		physicsWorld_.SetKinematicObjectShape(
			kinematicPlatform_,
			Collision::AABB{
				.min = { 6.0f, kinematicPlatformHeight_, -3.0f },
				.max = { 9.0f, kinematicPlatformHeight_ + 0.3f, 0.0f },
			});
		physicsWorld_.Update(deltaTime);
	}
}

void ShowcaseScene::UpdateVFX(float deltaTime) {
	particleWorld_.SetEmitterEnabled(particleEmitter_, particlesEnabled_);
	particleWorld_.Update(deltaTime);
	gpuParticleWorld_.SetEmitterEnabled(gpuParticleEmitter_, gpuParticlesEnabled_);
	gpuParticleWorld_.Update(deltaTime);
	effectWorld_.Update(deltaTime);
}

void ShowcaseScene::DrawWorld() {
	camera_.SetScene();
	directionalLight_.SetScene();
	pointLight_.intensity = pointLightIntensity_;
	pointLight_.position = { std::sin(elapsedTime_) * 5.0f, 4.0f, -1.0f };
	pointLight_.SetScene();
	toonLighting_.SetScene();
	skybox_.SetScene();

	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -1.5f, 1.0f }),
		Material3D{
			.color = { 0.14f, 0.17f, 0.20f, 1.0f },
			.metallic = 0.1f,
			.roughness = 0.72f,
		});
	if (showGrid_) {
		grid_.Draw(Math::MakeTranslateMatrix({ 0.0f, -1.48f, 1.0f }));
	}

	if (showPrimitives_) {
		// 全形状を羅列せず、PBRマテリアルを比較しやすい代表的な3形状に絞っています。
		sphere_.Draw(
			MakeWorldMatrix({ 1.5f, 1.5f, 1.5f }, { -6.0f, 0.0f, 4.0f }),
			Material3D{
				.color = { 0.95f, 0.18f, 0.08f, 1.0f },
				.metallic = 0.75f,
				.roughness = 0.16f,
				.shadingModel = ShadingModel3D::PBR,
			});
		box_.Draw(
			MakeWorldMatrix({ 1.0f, 1.0f, 1.0f }, elapsedTime_ * 0.55f, { -2.0f, 0.0f, 4.0f }),
			Material3D{
				.color = { 0.12f, 0.65f, 1.0f, 1.0f },
				.metallic = 0.15f,
				.roughness = 0.35f,
			});
		capsule_.Draw(
			MakeWorldMatrix({ 1.0f, 1.0f, 1.0f }, { 2.0f, 0.0f, 4.0f }),
			Material3D{
				.color = { 0.25f, 0.95f, 0.48f, 1.0f },
				.roughness = 0.55f,
			});
	}

	if (showModel_) {
		foxModel_.DrawInstanced(
			foxInstances_,
			foxAnimator_,
			ModelDrawOptions{
				.debugDraw = {
					.showMeshletColors = showMeshletColors_,
					.showMeshletBoundingSpheres = showMeshletBounds_,
				},
			});
	}

	DrawPhysics();
	DrawCollision();
	particleWorld_.Draw();
	gpuParticleWorld_.Draw();
	worldFont_.Draw(
		"Font3D",
		{ -6.0f, 3.5f, 4.0f },
		Font3DDrawOptions{
			.color = Color::Cyan,
			.anchor = { 0.5f, 0.5f },
			.worldUnitsPerPixel = 0.012f,
		});
	// Font / Panel / Image / Buttonの低レベル描画はUIWorld内部でまとめて行います。
	uiWorld_.Draw();
	if (showUIHitboxes_) {
		uiWorld_.DrawHitboxes3D();
	}
	if (showCameraDebug_) {
		renderTargetCamera_.DrawDebug({ .showFrustum = true, .showShadowCascades = true });
	}
}

void ShowcaseScene::DrawRenderTarget() {
	Graphics3D::BeginRenderPass(renderTarget_, renderTargetCamera_);
	directionalLight_.SetScene();
	ground_.Draw(
		Math::MakeTranslateMatrix({ 0.0f, -1.0f, 0.0f }),
		Vector4{ 0.08f, 0.12f, 0.18f, 1.0f });
	sphere_.Draw(
		MakeWorldMatrix({ 1.4f, 1.4f, 1.4f }, elapsedTime_, { 0.0f, 0.5f, 0.0f }),
		Material3D{
			.color = { 0.1f, 0.65f, 2.0f, 1.0f },
			.metallic = 0.6f,
			.roughness = 0.15f,
		});
	Graphics3D::EndRenderPass();
}

void ShowcaseScene::DrawPhysics() const {
	// GPU Particle BufferをMesh Shaderから直接参照し、粒子数に依存したCPU描画ループを避けます。
	physicsWorld_.DrawParticleRenderGroups();

	box_.Draw(
		MakeWorldMatrix(
			{ 1.5f, 0.15f, 1.5f },
			{ 7.5f, kinematicPlatformHeight_ + 0.15f, -1.5f }),
		Material3D{
			.color = { 1.0f, 0.48f, 0.08f, 1.0f },
			.metallic = 0.2f,
			.roughness = 0.5f,
		});

	for (const Physics::DynamicObjectHandle handle : physicsBalls_) {
		const auto state = physicsWorld_.GetState(handle);
		if (!state) {
			continue;
		}
		const auto* physicsSphere = std::get_if<Collision::Sphere>(&state->shape);
		if (!physicsSphere) {
			continue;
		}
		sphere_.Draw(
			MakeWorldMatrix(
				{ physicsSphere->radius, physicsSphere->radius, physicsSphere->radius },
				physicsSphere->center),
			Material3D{
				.color = { 0.78f, 0.22f, 1.0f, 1.0f },
				.metallic = 0.25f,
				.roughness = 0.28f,
			});
	}
}

void ShowcaseScene::DrawCollision() const {
	const auto fixedState = colliderWorld_.GetCollider(fixedCollider_);
	const auto movingState = colliderWorld_.GetCollider(movingCollider_);
	if (!fixedState || !movingState) {
		return;
	}
	const auto* fixedSphere = std::get_if<Collision::Sphere>(&fixedState->shape);
	const auto* movingSphere = std::get_if<Collision::Sphere>(&movingState->shape);
	if (!fixedSphere || !movingSphere) {
		return;
	}

	const Vector4 collisionColor = collisionDetected_ ? Color::Red : Color::Lime;
	sphere_.Draw(
		MakeWorldMatrix(
			{ fixedSphere->radius, fixedSphere->radius, fixedSphere->radius },
			fixedSphere->center),
		Material3D{ .color = { 0.18f, 0.45f, 0.2f, 1.0f }, .roughness = 0.75f });
	sphere_.Draw(
		MakeWorldMatrix(
			{ movingSphere->radius, movingSphere->radius, movingSphere->radius },
			movingSphere->center),
		Material3D{ .color = collisionColor, .roughness = 0.55f });
	collisionLink_.Draw(Math::Identity4x4(), collisionColor);
}

void ShowcaseScene::DrawOverlay2D() {
	const float windowWidth = static_cast<float>(Graphics2D::GetWindowWidth());
	const float windowHeight = static_cast<float>(Graphics2D::GetWindowHeight());

	// UIWorldとは別に、直接描画APIも特殊演出や一時的なデバッグ表示用として利用できます。
	Circle2D{ { 144.0f, windowHeight - 84.0f }, 30.0f }.Draw(
		Vector4{ 0.1f, 0.75f, 1.0f, 0.35f });

	renderTargetPreview_.leftTop = { windowWidth - renderTargetPreview_.size.x - 24.0f, 24.0f };
	renderTargetPreview_.Draw();
	uiFont_.Draw(
		"RenderTargetTexture 480 x 270",
		{ renderTargetPreview_.leftTop.x, renderTargetPreview_.leftTop.y + renderTargetPreview_.size.y + 6.0f },
		{ .color = Color::Cyan });

	if (showUIHitboxes_) {
		uiWorld_.DrawHitboxes();
	}
}

void ShowcaseScene::DrawControlPanel() {
	ImGui::SetNextWindowSize({ 390.0f, 650.0f }, ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Lazieal Feature Showcase")) {
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("Arrow keys: orbit camera   Q / E: distance");
	ImGui::TextUnformatted("The sample values below can be changed at runtime.");
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Feature checklist", ImGuiTreeNodeFlags_DefaultOpen)) {
		DrawFeatureList();
	}

	if (ImGui::CollapsingHeader("3D rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Show glTF model / skinning", &showModel_);
		ImGui::SliderFloat("Animation speed", &animationSpeed_, 0.0f, 3.0f);
		ImGui::Checkbox("Show meshlet colors", &showMeshletColors_);
		ImGui::Checkbox("Show meshlet bounding spheres", &showMeshletBounds_);
		ImGui::Checkbox("Show primitive samples", &showPrimitives_);
		ImGui::Checkbox("Show grid", &showGrid_);
		ImGui::Checkbox("Show camera frustum / shadow cascades", &showCameraDebug_);
		ImGui::SliderFloat("Point light intensity", &pointLightIntensity_, 0.0f, 80.0f);
		ImGui::ColorEdit3("Directional color", &directionalLight_.color.x);
		ImGui::SliderFloat("Directional intensity", &directionalLight_.intensity, 0.0f, 4.0f);

		ImGui::SeparatorText("Toon lighting");
		ImGui::Checkbox("Use Toon for scene default", &toonLighting_.enabled);
		ImGui::TextDisabled("Red sphere stays PBR for comparison.");
		ImGui::ColorEdit3("Toon shadow color", &toonLighting_.shadowColor.x);
		ImGui::ColorEdit3("Toon middle color", &toonLighting_.middleColor.x);
		ImGui::ColorEdit3("Toon light color", &toonLighting_.lightColor.x);
		ImGui::SliderFloat(
			"Toon shadow threshold",
			&toonLighting_.shadowThreshold,
			0.0f,
			toonLighting_.lightThreshold);
		ImGui::SliderFloat(
			"Toon light threshold",
			&toonLighting_.lightThreshold,
			toonLighting_.shadowThreshold,
			1.0f);
		ImGui::SliderFloat("Toon smoothness", &toonLighting_.smoothness, 0.0f, 0.25f);
	}

	if (ImGui::CollapsingHeader("Physics / collision", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Run physics", &physicsEnabled_);
		ImGui::SeparatorText("DynamicObject (CPU)");
		ImGui::SliderInt("Reset ball count", &physicsSpawnCount_, 1, 32);
		if (ImGui::Button("Spawn ball")) {
			SpawnPhysicsBall();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset balls")) {
			ResetPhysicsBalls();
		}
		ImGui::Text("Dynamic objects: %zu", physicsWorld_.GetDynamicObjectCount());

		ImGui::SeparatorText("PhysicsParticle (GPU)");
		ImGui::SliderInt("Particle spawn count", &physicsParticleSpawnCount_, 100, 20000);
		ImGui::SliderFloat("Physics particle radius", &physicsParticleRadius_, 0.05f, 0.25f);
		if (ImGui::Button("Generate GPU physics particles")) {
			SpawnPhysicsParticles();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear GPU physics particles")) {
			physicsWorld_.ClearParticles();
		}
		const Physics::PhysicsParticleStepStatistics& statistics =
			physicsWorld_.GetLastParticleStepStatistics();
		if (statistics.isAvailable) {
			ImGui::Text(
				"GPU alive: %llu   candidates: %llu   latency: %u frames",
				static_cast<unsigned long long>(statistics.particleCount),
				static_cast<unsigned long long>(statistics.candidatePairCount),
				statistics.latencyFrames);
		} else {
			ImGui::TextDisabled("GPU statistics: waiting for asynchronous readback...");
		}
		ImGui::TextDisabled("PhysicsParticle does not collide with DynamicObject.");

		ImGui::SeparatorText("ColliderWorld (CPU)");
		ImGui::TextColored(
			collisionDetected_ ? ImVec4{ 1.0f, 0.25f, 0.18f, 1.0f } : ImVec4{ 0.35f, 1.0f, 0.45f, 1.0f },
			collisionDetected_ ? "Collider event: Stay" : "Collider event: None");
	}

	if (ImGui::CollapsingHeader("VFX")) {
		ImGui::Checkbox("CPU particles", &particlesEnabled_);
		ImGui::Checkbox("GPU particles", &gpuParticlesEnabled_);
		if (ImGui::SliderFloat("Emission rate", &particleEmissionRate_, 0.0f, 500.0f)) {
			auto desc = particleWorld_.GetEmitterDesc(particleEmitter_);
			if (desc) {
				desc->emissionRate = particleEmissionRate_;
				particleWorld_.SetEmitterDesc(particleEmitter_, *desc);
			}
		}
		if (ImGui::Button("Burst 100 particles")) {
			particleWorld_.Emit(particleEmitter_, 100u);
		}
		ImGui::SameLine();
		if (ImGui::Button("Play Effekseer")) {
			PlayEffect();
		}
		ImGui::Text("Alive particles: %zu", particleWorld_.GetParticleCount());
		const VFX::ParticleStepStatistics& gpuStatistics = gpuParticleWorld_.GetLatestStatistics();
		if (gpuStatistics.isAvailable) {
			ImGui::Text(
				"GPU particles: %u   generated: %llu   latency: %u frames",
				gpuStatistics.particleCount,
				static_cast<unsigned long long>(gpuStatistics.generatedCount),
				gpuStatistics.latencyFrames);
		} else {
			ImGui::TextDisabled("GPU particle statistics: waiting for readback...");
		}
	}

	if (ImGui::CollapsingHeader("Post process")) {
		ImGui::Checkbox("Bloom", &bloomEnabled_);
		ImGui::SliderFloat("Bloom intensity", &bloomIntensity_, 0.0f, 2.0f);

		ImGui::Separator();
		ImGui::Checkbox("Vignette", &vignetteEnabled_);
		ImGui::SliderFloat("Vignette intensity", &vignetteIntensity_, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Checkbox("Grayscale", &grayscaleEnabled_);
		ImGui::SliderFloat("Grayscale intensity", &grayscaleIntensity_, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Checkbox("Chromatic aberration", &chromaticAberrationEnabled_);
		ImGui::SliderFloat(
			"Chromatic aberration intensity",
			&chromaticAberrationIntensity_,
			0.0f,
			16.0f);

		ImGui::Separator();
		ImGui::Checkbox("Pixelation", &pixelationEnabled_);
		ImGui::SliderFloat("Pixel size", &pixelSize_, 1.0f, 64.0f, "%.0f");

		ImGui::Separator();
		ImGui::Checkbox("Sharpen", &sharpenEnabled_);
		ImGui::SliderFloat("Sharpen intensity", &sharpenIntensity_, 0.0f, 2.0f);
		ImGui::SliderFloat("Sharpen threshold", &sharpenThreshold_, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::SliderFloat("Exposure EV", &exposureEV_, -3.0f, 3.0f);
	}

	if (ImGui::CollapsingHeader("2D / UI")) {
		ImGui::Checkbox("Show 2D overlay", &show2DOverlay_);
		ImGui::Checkbox("Show UI hitboxes", &showUIHitboxes_);
		const Gamepad gamepad{ 0u };
		ImGui::Text("Gamepad 0: %s", gamepad.IsConnected() ? gamepad.Name().c_str() : "Not connected");
		ImGui::Text("Cursor: %.0f, %.0f", Cursor::Pos().x, Cursor::Pos().y);
	}

	ImGui::Separator();
	ImGui::TextWrapped(
		"Initial values are loaded from Assets/Parameters/ShowcaseScene.json. "
		"Particle settings are loaded from the two Particle*.json files.");
	ImGui::End();
}

void ShowcaseScene::DrawFeatureList() const {
	ImGui::BulletText("3D primitives / PBR and Toon materials / lights / skybox");
	ImGui::BulletText("glTF model / shared skinning / instancing / meshlet debug");
	ImGui::BulletText("RenderTargetTexture / Texture / Sprite");
	ImGui::BulletText("2D and 3D fonts / primitives / UIWorld");
	ImGui::BulletText("Keyboard / mouse / gamepad InputCommand");
	ImGui::BulletText("ColliderWorld / PhysicsWorld");
	ImGui::BulletText("DynamicObject / KinematicObject / GPU PhysicsParticle");
	ImGui::BulletText("CPU and GPU VFX particles / Effekseer");
	ImGui::BulletText("Camera frustum / cascaded shadow debug draw");
	ImGui::BulletText("JSON parameters / Post Process Profile");
	ImGui::TextDisabled("Audio: no redistributable sample asset is included yet.");
	ImGui::TextDisabled("Scene transitions: kept in the dedicated multi-scene sample.");
}

void ShowcaseScene::SpawnPhysicsBall() {
	const float offset = static_cast<float>(physicsBalls_.size() % 5u) * 0.55f;
	const Collision::Sphere shape{
		.center = { 6.0f + offset, 7.0f + offset, -1.0f + offset },
		.radius = 0.45f,
	};
	physicsBalls_.push_back(physicsWorld_.AddDynamicObject({
		.shape = shape,
		.velocity = { 1.0f - offset, 0.0f, 0.4f },
		.mass = 1.0f,
		.material = { .restitution = 0.72f, .friction = 0.25f },
	}));
}

void ShowcaseScene::ResetPhysicsBalls() {
	physicsWorld_.ClearDynamicObjects();
	physicsBalls_.clear();
	for (int index = 0; index < physicsSpawnCount_; ++index) {
		SpawnPhysicsBall();
	}
}

void ShowcaseScene::SpawnPhysicsParticles() {
	// 生成コマンドは次のPhysicsWorld::UpdateでまとめてGPUへ転送されます。
	physicsWorld_.ClearParticles();
	const int columnCount = 20;
	const int rowCount = 12;
	const float spacing = physicsParticleRadius_ * 2.1f;

	for (int index = 0; index < physicsParticleSpawnCount_; ++index) {
		const int column = index % columnCount;
		const int row = (index / columnCount) % rowCount;
		const int layer = index / (columnCount * rowCount);
		physicsWorld_.AddParticle({
			.position = {
				5.1f + static_cast<float>(column) * spacing,
				4.0f + static_cast<float>(layer) * spacing,
				-3.5f + static_cast<float>(row) * spacing,
			},
			.velocity = {},
			.radius = physicsParticleRadius_,
			.mass = 1.0f,
			.material = { .restitution = 0.25f, .friction = 0.45f },
			.renderGroup = physicsParticleRenderGroup_,
		});
	}
}

void ShowcaseScene::PlayEffect() {
	if (!laserEffect_.IsValid()) {
		return;
	}
	if (effectWorld_.IsPlaying(effectHandle_)) {
		effectWorld_.Stop(effectHandle_);
	}
	effectHandle_ = effectWorld_.Play(
		laserEffect_,
		Transform3D{
			.scale = { 0.7f, 0.7f, 0.7f },
			.translation = { 0.0f, 0.0f, 1.5f },
		});
}

void ShowcaseScene::ApplyPostProcessSettings() {
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, "MainBloom", bloomEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, "MainBloom", "Intensity", bloomIntensity_);
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, "Vignette", vignetteEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, "Vignette", "Intensity", vignetteIntensity_);
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, "Grayscale", grayscaleEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, "Grayscale", "Intensity", grayscaleIntensity_);
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, "ChromaticAberration", chromaticAberrationEnabled_);
	Graphics3D::SetPostProcessParameter(
		postProcessProfile_,
		"ChromaticAberration",
		"Intensity",
		chromaticAberrationIntensity_);
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, "Pixelation", pixelationEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, "Pixelation", "PixelSize", pixelSize_);
	Graphics3D::SetPostProcessEffectEnabled(postProcessProfile_, "Sharpen", sharpenEnabled_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, "Sharpen", "Intensity", sharpenIntensity_);
	Graphics3D::SetPostProcessParameter(postProcessProfile_, "Sharpen", "Threshold", sharpenThreshold_);
	Graphics3D::SetToneMappingParameter(postProcessProfile_, "ExposureEV", exposureEV_);
}
