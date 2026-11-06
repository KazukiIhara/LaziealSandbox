#include "Scene/PhysicsShowcaseScene.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr std::string_view kParameterFileName = "PhysicsShowcaseDemo.json";
	constexpr float kGroundProbeMargin = 0.08f;
	constexpr float kMinimumGroundNormalY = 0.5f;

	void DrawOBB(const Collision::OBB& box, const Vector4& color) {
		Box3D(box.halfExtents.x * 2.0f, box.halfExtents.y * 2.0f, box.halfExtents.z * 2.0f).Draw(Math::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, box.rotation, box.center), color);
	}

}

PhysicsShowcaseScene::PhysicsShowcaseScene(const InitData& init) :
	SampleScene(init),
	parameters_(LoadParameters()),
	physicsWorld_(MakeWorldSettings(parameters_)),
	parameterStatus_("Loaded Assets/Parameters/PhysicsShowcaseDemo.json") {
	ResetSimulation();
}

void PhysicsShowcaseScene::Update() {
	if (toggleShellShakeRequested_) {
		isShellShaking_ = !isShellShaking_;
		toggleShellShakeRequested_ = false;
		if (!isShellShaking_) {
			shellShakeTime_ = 0.0f;
		}
	}
	if (Key::P.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::R.Trigger()) {
		ReloadParameters();
	}
	if (Key::E.Trigger()) {
		SpawnParticles();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	if (!isPaused_) {
		const float deltaTime = static_cast<float>(System::DeltaTime());
		UpdateShell(deltaTime);
		UpdatePlayer();
		physicsWorld_.Update(deltaTime);
		isPlayerGrounded_ = IsPlayerGrounded();
	}
	UpdateCamera();
}

void PhysicsShowcaseScene::Draw() const {
	DrawStage();

	SphericalShellCap3D(
		parameters_.shellOuterRadius,
		parameters_.shellInnerRadius,
		parameters_.shellHeight).Draw(
		Math::MakeTranslateMatrix(shellPosition_),
		Material3D{
			.color = Color::Silver,
			.metallic = 0.85f,
			.roughness = 0.28f,
		});

	DrawOBB(MakeWall(), Color::RoyalBlue);
	DrawOBB(MakeRamp(), Color::ForestGreen);

	std::vector<Physics::PhysicsParticleState> particleStates;
	physicsWorld_.GetParticleStates(particleStates);
	for (const Physics::PhysicsParticleState& state : particleStates) {
		const Vector4 color = state.handle.index % 3u == 0u
			? Color::Gold
			: state.handle.index % 3u == 1u ? Color::Orange : Color::Yellow;
		Sphere3D(state.radius).Draw(Math::MakeTranslateMatrix(state.position), color);
	}

	const std::optional<Physics::DynamicObjectState> playerState = physicsWorld_.GetState(playerObject_);
	if (playerState) {
		const Collision::Sphere& sphere = std::get<Collision::Sphere>(playerState->shape);
		Sphere3D(sphere.radius).Draw(Math::MakeTranslateMatrix(sphere.center), Color::Turquoise);
	}

	const Physics::PhysicsParticleStepStatistics& statistics = physicsWorld_.GetLastParticleStepStatistics();
	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Physics Showcase")) {
		ImGui::Text("Physics particles: %zu", physicsWorld_.GetParticleCount());
		ImGui::Text("Static objects: %zu", physicsWorld_.GetStaticObjectCount());
		ImGui::Text("Kinematic objects: %zu", physicsWorld_.GetKinematicObjectCount());
		ImGui::Text("Dynamic objects: %zu", physicsWorld_.GetDynamicObjectCount());
		ImGui::Text("Simulation: %s", isPaused_ ? "Paused" : "Playing");
		ImGui::Text("Wok motion: %s", isShellShaking_ ? "Automatic" : "Stopped");
		ImGui::Text("Player grounded: %s", isPlayerGrounded_ ? "Yes" : "No");
		ImGui::Text("Particle physics: %.3f ms | Candidate pairs: %llu", statistics.totalMilliseconds, static_cast<unsigned long long>(statistics.candidatePairCount));
		if (playerState) {
			const Collision::Sphere& sphere = std::get<Collision::Sphere>(playerState->shape);
			ImGui::Text("Player: %.2f, %.2f, %.2f", sphere.center.x, sphere.center.y, sphere.center.z);
		} else {
			ImGui::TextUnformatted("Player left the world bounds. Press R to reset.");
		}
		ImGui::Separator();
		if (ImGui::Button(isShellShaking_ ? "Stop shaking wok" : "Start shaking wok", ImVec2(-1.0f, 0.0f))) {
			toggleShellShakeRequested_ = true;
		}
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.9f, 1.0f), "Turquoise Sphere: WASD player");
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.0f, 1.0f), "Gold particles: PhysicsParticle");
		ImGui::TextUnformatted("WASD: move player | Space: jump | E: add particles");
		ImGui::TextUnformatted("P: pause | R: reload and reset | Escape: title");
		ImGui::TextWrapped("The player and particles are simulated by one PhysicsWorld and share the Static floor and Kinematic wok. They do not collide with each other yet.");
		ImGui::TextWrapped("Parameters: %s", parameterStatus_.c_str());
	}
	ImGui::End();
}

PhysicsShowcaseScene::DemoParameters PhysicsShowcaseScene::LoadParameters() {
	DemoParameters result{};
	result.gravity = Parameter::GetOr<Vector3>(kParameterFileName, "World/Gravity", result.gravity);
	result.worldBounds.min = Parameter::GetOr<Vector3>(kParameterFileName, "World/BoundsMin", result.worldBounds.min);
	result.worldBounds.max = Parameter::GetOr<Vector3>(kParameterFileName, "World/BoundsMax", result.worldBounds.max);
	if (!Collision::IsValid(result.worldBounds)) {
		result.worldBounds = DemoParameters{}.worldBounds;
	}

	result.floor.min = Parameter::GetOr<Vector3>(kParameterFileName, "Floor/Min", result.floor.min);
	result.floor.max = Parameter::GetOr<Vector3>(kParameterFileName, "Floor/Max", result.floor.max);
	if (!Collision::IsValid(result.floor)) {
		result.floor = DemoParameters{}.floor;
	}

	result.substepCount = static_cast<uint32_t>(std::clamp(Parameter::GetOr<int32_t>(kParameterFileName, "World/SubstepCount", 8), 1, 32));
	result.solverIterationCount = static_cast<uint32_t>(std::clamp(Parameter::GetOr<int32_t>(kParameterFileName, "World/SolverIterationCount", 8), 1, 32));
	result.linearDamping = std::max(Parameter::GetOr<float>(kParameterFileName, "World/LinearDamping", result.linearDamping), 0.0f);

	result.shellPosition = Parameter::GetOr<Vector3>(kParameterFileName, "Shell/Position", result.shellPosition);
	result.shellShakeAmplitude = Parameter::GetOr<Vector3>(kParameterFileName, "Shell/ShakeAmplitude", result.shellShakeAmplitude);
	result.shellShakeAmplitude.x = std::abs(result.shellShakeAmplitude.x);
	result.shellShakeAmplitude.y = std::abs(result.shellShakeAmplitude.y);
	result.shellShakeAmplitude.z = std::abs(result.shellShakeAmplitude.z);
	result.shellShakeAngularSpeed = Parameter::GetOr<Vector3>(kParameterFileName, "Shell/ShakeAngularSpeed", result.shellShakeAngularSpeed);
	result.shellShakeAngularSpeed.x = std::abs(result.shellShakeAngularSpeed.x);
	result.shellShakeAngularSpeed.y = std::abs(result.shellShakeAngularSpeed.y);
	result.shellShakeAngularSpeed.z = std::abs(result.shellShakeAngularSpeed.z);
	result.shellOuterRadius = std::max(Parameter::GetOr<float>(kParameterFileName, "Shell/OuterRadius", result.shellOuterRadius), 0.02f);
	result.shellInnerRadius = std::clamp(Parameter::GetOr<float>(kParameterFileName, "Shell/InnerRadius", result.shellInnerRadius), 0.01f, result.shellOuterRadius - 0.01f);
	const float shellThickness = result.shellOuterRadius - result.shellInnerRadius;
	result.shellHeight = std::clamp(Parameter::GetOr<float>(kParameterFileName, "Shell/Height", result.shellHeight), shellThickness + 0.01f, result.shellOuterRadius);
	result.shellRestitution = std::clamp(Parameter::GetOr<float>(kParameterFileName, "Shell/Restitution", result.shellRestitution), 0.0f, 1.0f);
	result.shellFriction = std::clamp(Parameter::GetOr<float>(kParameterFileName, "Shell/Friction", result.shellFriction), 0.0f, 1.0f);
	result.particleSpawnCenter = Parameter::GetOr<Vector3>(kParameterFileName, "Particle/SpawnCenter", result.particleSpawnCenter);
	result.particleColumnCount = static_cast<uint32_t>(std::clamp(Parameter::GetOr<int32_t>(kParameterFileName, "Particle/ColumnCount", 5), 1, 12));
	result.particleLayerCount = static_cast<uint32_t>(std::clamp(Parameter::GetOr<int32_t>(kParameterFileName, "Particle/LayerCount", 2), 1, 8));
	result.particleRadius = std::max(Parameter::GetOr<float>(kParameterFileName, "Particle/Radius", result.particleRadius), 0.01f);
	result.particleMass = std::max(Parameter::GetOr<float>(kParameterFileName, "Particle/Mass", result.particleMass), 0.01f);
	result.particleRestitution = std::clamp(Parameter::GetOr<float>(kParameterFileName, "Particle/Restitution", result.particleRestitution), 0.0f, 1.0f);
	result.particleFriction = std::clamp(Parameter::GetOr<float>(kParameterFileName, "Particle/Friction", result.particleFriction), 0.0f, 1.0f);
	result.particleSpacing = std::max(Parameter::GetOr<float>(kParameterFileName, "Particle/Spacing", result.particleSpacing), result.particleRadius * 2.0f);

	result.playerPosition = Parameter::GetOr<Vector3>(kParameterFileName, "Player/Position", result.playerPosition);
	result.playerRadius = std::max(Parameter::GetOr<float>(kParameterFileName, "Player/Radius", result.playerRadius), 0.05f);
	result.playerMoveSpeed = std::max(Parameter::GetOr<float>(kParameterFileName, "Player/MoveSpeed", result.playerMoveSpeed), 0.0f);
	result.playerJumpSpeed = std::max(Parameter::GetOr<float>(kParameterFileName, "Player/JumpSpeed", result.playerJumpSpeed), 0.0f);

	result.wallCenter = Parameter::GetOr<Vector3>(kParameterFileName, "Wall/Center", result.wallCenter);
	result.wallHalfExtents = Parameter::GetOr<Vector3>(kParameterFileName, "Wall/HalfExtents", result.wallHalfExtents);
	result.wallRotationDegrees = Parameter::GetOr<Vector3>(kParameterFileName, "Wall/RotationDegrees", result.wallRotationDegrees);
	result.wallHalfExtents.x = std::max(result.wallHalfExtents.x, 0.01f);
	result.wallHalfExtents.y = std::max(result.wallHalfExtents.y, 0.01f);
	result.wallHalfExtents.z = std::max(result.wallHalfExtents.z, 0.01f);

	result.rampCenter = Parameter::GetOr<Vector3>(kParameterFileName, "Ramp/Center", result.rampCenter);
	result.rampHalfExtents = Parameter::GetOr<Vector3>(kParameterFileName, "Ramp/HalfExtents", result.rampHalfExtents);
	result.rampRotationDegrees = Parameter::GetOr<Vector3>(kParameterFileName, "Ramp/RotationDegrees", result.rampRotationDegrees);
	result.rampHalfExtents.x = std::max(result.rampHalfExtents.x, 0.01f);
	result.rampHalfExtents.y = std::max(result.rampHalfExtents.y, 0.01f);
	result.rampHalfExtents.z = std::max(result.rampHalfExtents.z, 0.01f);
	return result;
}

Physics::PhysicsWorldSettings PhysicsShowcaseScene::MakeWorldSettings(const DemoParameters& parameters) {
	return {
		.gravity = parameters.gravity,
		.worldBounds = parameters.worldBounds,
		.substepCount = parameters.substepCount,
		.solverIterationCount = parameters.solverIterationCount,
		.linearDamping = parameters.linearDamping,
	};
}

void PhysicsShowcaseScene::ReloadParameters() {
	const ParameterIoResult result = Parameter::Reload(kParameterFileName);
	if (!result) {
		parameterStatus_ = "Reload failed: " + result.message;
		return;
	}

	parameters_ = LoadParameters();
	physicsWorld_.SetSettings(MakeWorldSettings(parameters_));
	parameterStatus_ = "Reloaded Assets/Parameters/PhysicsShowcaseDemo.json";
	ResetSimulation();
}

Collision::SphericalShellCap PhysicsShowcaseScene::MakeShell() const {
	return {
		.outerApex = shellPosition_,
		.axis = { 0.0f, 1.0f, 0.0f },
		.outerRadius = parameters_.shellOuterRadius,
		.innerRadius = parameters_.shellInnerRadius,
		.height = parameters_.shellHeight,
	};
}

Collision::OBB PhysicsShowcaseScene::MakeWall() const {
	return {
		.center = parameters_.wallCenter,
		.halfExtents = parameters_.wallHalfExtents,
		.rotation = Math::MakeQuaternion(parameters_.wallRotationDegrees * Math::DEG_TO_RAD),
	};
}

Collision::OBB PhysicsShowcaseScene::MakeRamp() const {
	return {
		.center = parameters_.rampCenter,
		.halfExtents = parameters_.rampHalfExtents,
		.rotation = Math::MakeQuaternion(parameters_.rampRotationDegrees * Math::DEG_TO_RAD),
	};
}

void PhysicsShowcaseScene::ResetSimulation() {
	physicsWorld_.Clear();
	shellPosition_ = parameters_.shellPosition;
	shellShakeTime_ = 0.0f;
	isPaused_ = false;
	isShellShaking_ = false;
	isPlayerGrounded_ = false;
	toggleShellShakeRequested_ = false;

	floorObject_ = physicsWorld_.AddStaticObject(
		parameters_.floor,
		{
			.material = {
				.restitution = 0.0f,
				.friction = 1.0f,
			},
		});

	shellObject_ = physicsWorld_.AddKinematicObject(
		MakeShell(),
		{
			.material = {
				.restitution = parameters_.shellRestitution,
				.friction = parameters_.shellFriction,
			},
		});

	wallObject_ = physicsWorld_.AddStaticObject(MakeWall());
	rampObject_ = physicsWorld_.AddStaticObject(MakeRamp());
	playerObject_ = physicsWorld_.AddDynamicObject(Collision::Sphere{
		.center = parameters_.playerPosition,
		.radius = parameters_.playerRadius,
	});

	SpawnParticles();
}

void PhysicsShowcaseScene::SpawnParticles() {
	const float halfColumns = static_cast<float>(parameters_.particleColumnCount - 1u) * 0.5f;
	for (uint32_t layer = 0u; layer < parameters_.particleLayerCount; ++layer) {
		for (uint32_t z = 0u; z < parameters_.particleColumnCount; ++z) {
			for (uint32_t x = 0u; x < parameters_.particleColumnCount; ++x) {
				physicsWorld_.AddParticle({
					.position = parameters_.particleSpawnCenter +
						(shellPosition_ - parameters_.shellPosition) + Vector3{
						(static_cast<float>(x) - halfColumns) * parameters_.particleSpacing,
						static_cast<float>(layer) * parameters_.particleSpacing,
						(static_cast<float>(z) - halfColumns) * parameters_.particleSpacing,
					},
					.radius = parameters_.particleRadius,
					.mass = parameters_.particleMass,
					.material = {
						.restitution = parameters_.particleRestitution,
						.friction = parameters_.particleFriction,
					},
				});

			}
		}
	}
}

void PhysicsShowcaseScene::UpdateShell(float deltaTime) {
	if (isShellShaking_) {
		shellShakeTime_ += deltaTime;
		shellPosition_ = parameters_.shellPosition + Vector3{
			std::sin(shellShakeTime_ * parameters_.shellShakeAngularSpeed.x) *
				parameters_.shellShakeAmplitude.x,
			std::sin(shellShakeTime_ * parameters_.shellShakeAngularSpeed.y) *
				parameters_.shellShakeAmplitude.y,
			std::sin(shellShakeTime_ * parameters_.shellShakeAngularSpeed.z) *
				parameters_.shellShakeAmplitude.z,
		};
	} else {
		shellPosition_ = parameters_.shellPosition;
	}
	physicsWorld_.SetKinematicObjectShape(shellObject_, MakeShell());
}

void PhysicsShowcaseScene::UpdatePlayer() {
	const std::optional<Physics::DynamicObjectState> playerState = physicsWorld_.GetState(playerObject_);
	if (!playerState) {
		return;
	}

	Vector3 movement{
		static_cast<float>(Key::D.Press()) -
			static_cast<float>(Key::A.Press()),
		0.0f,
		static_cast<float>(Key::W.Press()) -
			static_cast<float>(Key::S.Press()),
	};
	if (Math::LengthSq(movement) > 1.0f) {
		movement = Math::Normalize(movement);
	}
	const bool isGrounded = IsPlayerGrounded();
	const float verticalVelocity = Key::Space.Trigger() && isGrounded
		? parameters_.playerJumpSpeed
		: playerState->velocity.y;

	physicsWorld_.SetVelocity(playerObject_, { movement.x * parameters_.playerMoveSpeed, verticalVelocity, movement.z * parameters_.playerMoveSpeed });
}

bool PhysicsShowcaseScene::IsPlayerGrounded() const {
	const std::optional<Physics::DynamicObjectState> playerState = physicsWorld_.GetState(playerObject_);
	if (!playerState) {
		return false;
	}

	const Collision::Sphere& sphere = std::get<Collision::Sphere>(playerState->shape);
	const Collision::Ray groundProbe{
		.origin = sphere.center,
		.direction = { 0.0f, -1.0f, 0.0f },
	};
	const float maximumDistance = sphere.radius + kGroundProbeMargin;
	const auto isGroundHit = [&](const auto& surface) {
		const std::optional<Collision::RaycastHit> hit = Collision::Raycast(groundProbe, surface, maximumDistance);
		return hit && hit->normal.y >= kMinimumGroundNormalY;
	};

	return isGroundHit(parameters_.floor) || isGroundHit(MakeRamp());
}
