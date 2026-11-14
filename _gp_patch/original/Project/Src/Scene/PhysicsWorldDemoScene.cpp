#include "Scene/PhysicsWorldDemoScene.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

using namespace LGF;

namespace {

	constexpr std::size_t kMaximumSphereCount = 40u;
	constexpr std::size_t kMaximumParticleCount = 240u;
	constexpr ObjectHandle kPlayerObjectHandle{ 0u, 1u };
	constexpr uint32_t kFloorObjectIndex = 1u;
	constexpr uint32_t kUpperRampObjectIndex = 2u;
	constexpr uint32_t kLowerRampObjectIndex = 3u;
	constexpr uint32_t kPlatformObjectIndex = 9u;
	constexpr float kPlayerMoveSpeed = 4.5f;

	Physics::PhysicsWorldSettings MakeWorldSettings() {
		Physics::PhysicsWorldSettings settings{};
		settings.worldBounds = {
			.min = { -12.0f, -5.0f, -12.0f },
			.max = { 12.0f, 20.0f, 12.0f },
		};
		settings.substepCount = 8u;
		settings.solverIterationCount = 6u;
		settings.linearDamping = 0.15f;
		return settings;
	}

	Collision::OBB MakeBox(
		const Vector3& center,
		const Vector3& halfExtents,
		float rotationZ = 0.0f) {
		return {
			.center = center,
			.halfExtents = halfExtents,
			.rotation = Math::MakeRotateAxisAngle({ 0.0f, 0.0f, 1.0f }, rotationZ),
		};
	}

	Matrix4x4 MakeBoxMatrix(const Collision::OBB& box) {
		return Math::MakeAffineMatrix(
			box.halfExtents * 2.0f,
			box.rotation,
			box.center);
	}

	Collision::OBB MakePlatform(float time) {
		return MakeBox(
			{ -3.5f + std::sin(time) * 2.0f, 1.35f, -2.8f },
			{ 1.6f, 0.18f, 1.2f });
	}
}

PhysicsWorldDemoScene::PhysicsWorldDemoScene(const InitData& init) :
	SampleScene(init),
	physicsWorld_(MakeWorldSettings()) {
	Reset();
}

void PhysicsWorldDemoScene::Update() {
	UpdateCamera();

	if (Key::P.Trigger() || Key::Escape.Trigger()) {
		ChangeScene(SceneNames::GamePlayScene);
		return;
	}
	if (Key::R.Trigger()) {
		Reset();
	}
	if (Key::Space.Trigger()) {
		SpawnSpheres(10u);
	}
	if (Key::O.Trigger()) {
		SpawnParticles(48u);
	}
	if (Key::I.Trigger()) {
		LaunchSpheres();
	}
	if (Key::F.Trigger()) {
		physicsWorld_.AddImpulse(playerHandle_, { 0.0f, 6.0f, 0.0f });
	}
	if (Key::G.Press()) {
		physicsWorld_.GetDynamicObjectStates(dynamicObjectStates_);
		for (const Physics::DynamicObjectState& state : dynamicObjectStates_) {
			if (state.objectHandle != kPlayerObjectHandle) {
				physicsWorld_.AddForce(
					state.handle,
					{ 4.0f * state.mass, 0.0f, 0.0f });
			}
		}
	}

	const float deltaTime = static_cast<float>(System::DeltaTime());
	if (!isPaused_) {
		UpdatePlayer();
		UpdateKinematicPlatform(deltaTime);
		spawnTimer_ += deltaTime;
		if (autoSpawn_ &&
			spawnTimer_ >= 0.65f &&
			GetFallingSphereCount() < kMaximumSphereCount) {
			spawnTimer_ = 0.0f;
			SpawnSpheres(1u);
		}
		physicsWorld_.Update(deltaTime);
	}

	physicsWorld_.GetStaticObjectStates(staticObjectStates_);
	physicsWorld_.GetKinematicObjectStates(kinematicObjectStates_);
	physicsWorld_.GetDynamicObjectStates(dynamicObjectStates_);
	physicsWorld_.GetParticleStates(particleStates_);
	DrawDebugWindow();
}

void PhysicsWorldDemoScene::Draw() const {
	DrawStage();

	for (const Physics::StaticObjectState& state : staticObjectStates_) {
		if (const auto* box = std::get_if<Collision::OBB>(&state.shape)) {
			Vector4 color = Color::Silver;
			if (state.options.objectHandle.index == kFloorObjectIndex) {
				color = Color::DarkGray;
			} else if (
				state.options.objectHandle.index == kUpperRampObjectIndex ||
				state.options.objectHandle.index == kLowerRampObjectIndex) {
				color = Color::RoyalBlue;
			}
			Box3D(1.0f).Draw(MakeBoxMatrix(*box), color);
		} else if (const auto* sphere =
			std::get_if<Collision::Sphere>(&state.shape)) {
			Sphere3D(sphere->radius).Draw(
				Math::MakeTranslateMatrix(sphere->center),
				Color::Orange);
		}
	}

	for (const Physics::KinematicObjectState& state :
		kinematicObjectStates_) {
		if (const auto* box = std::get_if<Collision::OBB>(&state.shape)) {
			Box3D(1.0f).Draw(MakeBoxMatrix(*box), Color::Violet);
		}
	}

	for (const Physics::DynamicObjectState& state : dynamicObjectStates_) {
		const Collision::Sphere& sphere = std::get<Collision::Sphere>(state.shape);
		if (state.objectHandle == kPlayerObjectHandle) {
			Sphere3D(sphere.radius).Draw(
				Math::MakeTranslateMatrix(sphere.center),
				Color::SpringGreen);
			continue;
		}
		const Vector4 color = Math::Length(state.velocity) > 7.0f
			? Color::Tomato
			: Color::Gold;
		Sphere3D(sphere.radius).Draw(
			Math::MakeTranslateMatrix(sphere.center),
			color);
	}

	for (const Physics::PhysicsParticleState& state : particleStates_) {
		Sphere3D(state.radius).Draw(
			Math::MakeTranslateMatrix(state.position),
			Color::White);
	}
}

void PhysicsWorldDemoScene::Reset() {
	physicsWorld_.Clear();
	physicsWorld_.SetSettings(MakeWorldSettings());
	staticObjectStates_.clear();
	kinematicObjectStates_.clear();
	dynamicObjectStates_.clear();
	particleStates_.clear();
	nextObjectIndex_ = 100u;
	spawnTimer_ = 0.0f;
	platformTime_ = 0.0f;
	autoSpawn_ = true;
	isPaused_ = false;

	physicsWorld_.AddStaticObject(
		MakeBox({ 0.0f, -0.5f, 0.0f }, { 7.0f, 0.5f, 6.0f }),
		Physics::StaticObjectOptions{
			.material = { .restitution = 0.1f, .friction = 0.85f },
			.objectHandle = { kFloorObjectIndex, 1u },
		});
	physicsWorld_.AddStaticObject(
		MakeBox(
			{ 2.3f, 6.1f, 1.5f },
			{ 3.7f, 0.18f, 1.6f },
			18.0f * Math::DEG_TO_RAD),
		Physics::StaticObjectOptions{
			.material = { .restitution = 0.1f, .friction = 0.55f },
			.objectHandle = { kUpperRampObjectIndex, 1u },
		});
	physicsWorld_.AddStaticObject(
		MakeBox(
			{ 1.7f, 3.6f, 1.5f },
			{ 3.7f, 0.18f, 1.6f },
			-18.0f * Math::DEG_TO_RAD),
		Physics::StaticObjectOptions{
			.material = { .restitution = 0.1f, .friction = 0.55f },
			.objectHandle = { kLowerRampObjectIndex, 1u },
		});
	physicsWorld_.AddStaticObject(
		MakeBox({ -7.25f, 2.0f, 0.0f }, { 0.25f, 2.0f, 6.0f }),
		Physics::StaticObjectOptions{ .objectHandle = { 4u, 1u } });
	physicsWorld_.AddStaticObject(
		MakeBox({ 7.25f, 2.0f, 0.0f }, { 0.25f, 2.0f, 6.0f }),
		Physics::StaticObjectOptions{ .objectHandle = { 5u, 1u } });
	physicsWorld_.AddStaticObject(
		MakeBox({ 0.0f, 2.0f, -6.25f }, { 7.0f, 2.0f, 0.25f }),
		Physics::StaticObjectOptions{ .objectHandle = { 6u, 1u } });
	physicsWorld_.AddStaticObject(
		MakeBox({ 0.0f, 2.0f, 6.25f }, { 7.0f, 2.0f, 0.25f }),
		Physics::StaticObjectOptions{ .objectHandle = { 7u, 1u } });

	physicsWorld_.AddStaticObject(
		Collision::Sphere{
			.center = { -1.6f, 0.85f, 1.5f },
			.radius = 0.85f,
		},
		Physics::StaticObjectOptions{
			.material = { .restitution = 0.7f, .friction = 0.25f },
			.objectHandle = { 8u, 1u },
		});

	platformHandle_ = physicsWorld_.AddKinematicObject(
		MakePlatform(platformTime_),
		Physics::KinematicObjectOptions{
			.material = { .restitution = 0.1f, .friction = 0.9f },
			.objectHandle = { kPlatformObjectIndex, 1u },
		});

	playerHandle_ = physicsWorld_.AddDynamicObject({
		.shape = Collision::Sphere{
			.center = { 0.0f, 0.65f, -3.5f },
			.radius = 0.6f,
		},
		.mass = 1.5f,
		.material = { .restitution = 0.05f, .friction = 0.9f },
		.objectHandle = kPlayerObjectHandle,
	});

	SpawnSpheres(8u);
	SpawnParticles(64u);
	physicsWorld_.GetStaticObjectStates(staticObjectStates_);
	physicsWorld_.GetKinematicObjectStates(kinematicObjectStates_);
	physicsWorld_.GetDynamicObjectStates(dynamicObjectStates_);
	physicsWorld_.GetParticleStates(particleStates_);
}

void PhysicsWorldDemoScene::SpawnSpheres(std::size_t count) {
	const std::size_t available = kMaximumSphereCount -
		std::min(GetFallingSphereCount(), kMaximumSphereCount);
	const std::size_t spawnCount = std::min(count, available);

	for (std::size_t index = 0u; index < spawnCount; ++index) {
		const uint32_t objectIndex = nextObjectIndex_++;
		const int32_t horizontalOffset =
			static_cast<int32_t>(objectIndex % 3u) - 1;
		const int32_t depth = static_cast<int32_t>(objectIndex % 5u) - 2;
		const float radius =
			0.28f + static_cast<float>(objectIndex % 4u) * 0.045f;

		physicsWorld_.AddDynamicObject({
			.shape = Collision::Sphere{
				.center = {
					5.2f + static_cast<float>(horizontalOffset) * 0.18f,
					8.2f + static_cast<float>(objectIndex % 8u) * 0.55f,
					1.5f + static_cast<float>(depth) * 0.45f,
				},
				.radius = radius,
			},
			.velocity = { -0.2f, 0.0f, 0.0f },
			.mass = 0.75f + radius,
			.material = { .restitution = 0.35f, .friction = 0.3f },
			.objectHandle = { objectIndex, 1u },
		});
	}
}

void PhysicsWorldDemoScene::SpawnParticles(std::size_t count) {
	const std::size_t available = kMaximumParticleCount -
		std::min(physicsWorld_.GetParticleCount(), kMaximumParticleCount);
	const std::size_t spawnCount = std::min(count, available);
	for (std::size_t index = 0u; index < spawnCount; ++index) {
		const uint32_t objectIndex = nextObjectIndex_++;
		physicsWorld_.AddParticle({
			.position = {
				-5.2f + static_cast<float>(index % 8u) * 0.18f,
				5.5f + static_cast<float>(index / 8u) * 0.2f,
				-2.8f + static_cast<float>(index % 3u) * 0.13f,
			},
			.velocity = { 0.7f, 0.0f, 0.0f },
			.radius = 0.09f,
			.mass = 0.15f,
			.material = { .restitution = 0.3f, .friction = 0.2f },
			.objectHandle = { objectIndex, 1u },
		});
	}
}

void PhysicsWorldDemoScene::LaunchSpheres() {
	physicsWorld_.GetDynamicObjectStates(dynamicObjectStates_);
	for (const Physics::DynamicObjectState& state : dynamicObjectStates_) {
		if (state.objectHandle == kPlayerObjectHandle) {
			continue;
		}
		physicsWorld_.AddImpulse(
			state.handle,
			{ 0.0f, 7.5f * state.mass, 0.0f });
	}
}

void PhysicsWorldDemoScene::UpdatePlayer() {
	const std::optional<Physics::DynamicObjectState> playerState =
		physicsWorld_.GetState(playerHandle_);
	if (!playerState) {
		return;
	}

	Vector3 movement{};
	if (Key::A.Press()) {
		movement.x -= 1.0f;
	}
	if (Key::D.Press()) {
		movement.x += 1.0f;
	}
	if (Key::W.Press()) {
		movement.z += 1.0f;
	}
	if (Key::S.Press()) {
		movement.z -= 1.0f;
	}
	if (Math::LengthSq(movement) > Math::EPSILON) {
		movement = Math::Normalize(movement) * kPlayerMoveSpeed;
	}

	physicsWorld_.SetVelocity(
		playerHandle_,
		{ movement.x, playerState->velocity.y, movement.z });
}

void PhysicsWorldDemoScene::UpdateKinematicPlatform(float deltaTime) {
	platformTime_ += deltaTime;
	physicsWorld_.SetKinematicObjectShape(
		platformHandle_,
		MakePlatform(platformTime_));
}

std::size_t PhysicsWorldDemoScene::GetFallingSphereCount() const {
	const std::size_t dynamicObjectCount = physicsWorld_.GetDynamicObjectCount();
	return physicsWorld_.IsValid(playerHandle_) && dynamicObjectCount > 0u
		? dynamicObjectCount - 1u
		: dynamicObjectCount;
}

void PhysicsWorldDemoScene::DrawDebugWindow() {
	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(470.0f, 0.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("PhysicsWorld Demo")) {
		ImGui::TextUnformatted("WASD: move green player | F: impulse jump | G: wind force");
		ImGui::TextUnformatted("Space: spawn 10 balls | I: impulse balls | O: spawn particles");
		ImGui::TextUnformatted("R: reset | P / Escape: return | Arrow keys/Q/E: camera");
		ImGui::Checkbox("Auto spawn", &autoSpawn_);
		ImGui::SameLine();
		ImGui::Checkbox("Paused", &isPaused_);

		if (ImGui::Button("Spawn 10 balls")) {
			SpawnSpheres(10u);
		}
		ImGui::SameLine();
		if (ImGui::Button("Launch balls")) {
			LaunchSpheres();
		}
		ImGui::SameLine();
		if (ImGui::Button("Spawn 48 particles")) {
			SpawnParticles(48u);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {
			Reset();
		}

		const Physics::PhysicsWorldSettings& settings = physicsWorld_.GetSettings();
		const Physics::PhysicsParticleStepStatistics& statistics =
			physicsWorld_.GetLastParticleStepStatistics();
		ImGui::SeparatorText("World state");
		ImGui::Text(
			"Dynamic spheres: %zu / %zu  Particles: %zu / %zu",
			GetFallingSphereCount(),
			kMaximumSphereCount,
			physicsWorld_.GetParticleCount(),
			kMaximumParticleCount);
		ImGui::Text(
			"Static: %zu  Kinematic: %zu  Dynamic: %zu",
			physicsWorld_.GetStaticObjectCount(),
			physicsWorld_.GetKinematicObjectCount(),
			physicsWorld_.GetDynamicObjectCount());
		ImGui::Text(
			"Gravity: (%.1f, %.1f, %.1f)",
			settings.gravity.x,
			settings.gravity.y,
			settings.gravity.z);
		ImGui::Text(
			"Substeps: %u  Solver iterations: %u",
			settings.substepCount,
			settings.solverIterationCount);
		ImGui::Text(
			"Particle step: %.3f ms  Candidate pairs: %llu",
			statistics.totalMilliseconds,
			static_cast<unsigned long long>(statistics.candidatePairCount));
		ImGui::TextDisabled("Gold balls collide with static shapes, the player, and each other.");
		ImGui::TextDisabled("Purple platform is Kinematic; white spheres use the particle solver.");
		ImGui::TextDisabled("Orange obstacle demonstrates a Static Sphere alongside OBB surfaces.");
	}
	ImGui::End();
}
