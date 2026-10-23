#include "Scene/SphericalShellCapScene.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr std::string_view kParameterFileName = "SphericalShellCapDemo.json";

}

SphericalShellCapScene::SphericalShellCapScene(const InitData& init) :
	SampleScene(init),
	parameters_(LoadParameters()),
	particleWorld_(MakeWorldSettings(parameters_)),
	parameterStatus_("Loaded Assets/Parameters/SphericalShellCapDemo.json") {
	ResetSimulation();
}

void SphericalShellCapScene::Update() {
	if (Key::Space.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::R.Trigger()) {
		ReloadParameters();
	}
	if (Key::E.Trigger()) {
		SpawnParticles();
	}
	if (Key::A.Trigger()) {
		automaticMove_ = !automaticMove_;
		automaticMoveTime_ = 0.0f;
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	const bool mouseAvailable =
		Window::IsFocused() && !ImGui::GetIO().WantCaptureMouse;
	if (mouseAvailable && Mouse::Left.Press()) {
		const Vector2 mouseDelta = Cursor::Delta();
		shellPosition_.x += mouseDelta.x * 0.018f;
		shellPosition_.z += mouseDelta.y * 0.018f;
		shellPosition_.x = std::clamp(shellPosition_.x, -3.5f, 3.5f);
		shellPosition_.z = std::clamp(shellPosition_.z, -3.5f, 3.5f);
		automaticMove_ = false;
	}
	if (mouseAvailable) {
		shellPosition_.y += Mouse::WheelDelta() * 0.2f;
		shellPosition_.y = std::clamp(shellPosition_.y, -0.5f, 2.5f);
	}

	if (automaticMove_) {
		automaticMoveTime_ += static_cast<float>(System::DeltaTime());
		shellPosition_.x = std::sin(automaticMoveTime_ * 2.2f) * 2.7f;
		shellPosition_.z = std::sin(automaticMoveTime_ * 1.35f) * 1.3f;
	}

	particleWorld_.SetColliderShape(shellCollider_, MakeShell());
	if (!isPaused_) {
		particleWorld_.Update(static_cast<float>(System::DeltaTime()));
	}
	UpdateCamera();
}

void SphericalShellCapScene::Draw() const {
	DrawStage();

	SphericalShellCap3D(
		parameters_.outerRadius,
		parameters_.innerRadius,
		parameters_.shellHeight).Draw(
		Math::MakeTranslateMatrix(shellPosition_),
		Material3D{
			.color = Color::Silver,
			.metallic = 0.85f,
			.roughness = 0.28f,
		});

	std::vector<Physics::ParticleState> states;
	particleWorld_.GetStates(states);
	for (const Physics::ParticleState& state : states) {
		const Vector4 color = state.userData % 3u == 0u
			? Color::Gold
			: state.userData % 3u == 1u ? Color::Orange : Color::Yellow;
		Sphere3D(state.radius).Draw(Math::MakeTranslateMatrix(state.position), color);
	}

	const std::size_t removedParticleCount =
		spawnedParticleCount_ - particleWorld_.GetParticleCount();
	const std::size_t outsideCavityCount = CountParticlesOutsideCavity();
	const Physics::ParticleWorldStepStatistics& statistics =
		particleWorld_.GetLastStepStatistics();

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("SphericalShellCap Demo")) {
		ImGui::Text("Particles: %zu", particleWorld_.GetParticleCount());
		ImGui::Text("Outside cavity below rim: %zu", outsideCavityCount);
		ImGui::Text("Removed outside world bounds: %zu", removedParticleCount);
		ImGui::Text(
			"Physics: %.3f ms | Candidate pairs: %llu",
			statistics.totalMilliseconds,
			static_cast<unsigned long long>(statistics.candidatePairCount));
		ImGui::Text(
			"Integrate %.3f | Broad phase %.3f | Position %.3f ms",
			statistics.integrationMilliseconds,
			statistics.broadPhaseMilliseconds,
			statistics.positionSolveMilliseconds);
		ImGui::Text(
			"Velocity %.3f | Cleanup %.3f ms",
			statistics.velocitySolveMilliseconds,
			statistics.cleanupMilliseconds);
		ImGui::Text("Automatic movement: %s", automaticMove_ ? "On" : "Off");
		ImGui::Text("Simulation: %s", isPaused_ ? "Paused" : "Playing");
		ImGui::Text(
			"Mass %.2f | Restitution %.2f | Friction %.2f | Damping %.2f",
			parameters_.particleMass,
			parameters_.particleRestitution,
			parameters_.particleFriction,
			parameters_.linearDamping);
		ImGui::Text(
			"Slop %.3f | Correction %.2f | Max depenetration %.1f | Bounce threshold %.1f",
			parameters_.penetrationSlop,
			parameters_.positionCorrectionRate,
			parameters_.maximumDepenetrationVelocity,
			parameters_.bounceVelocityThreshold);
		ImGui::TextWrapped("Parameters: %s", parameterStatus_.c_str());
		ImGui::Separator();
		ImGui::TextUnformatted("Left drag: move shell on XZ plane");
		ImGui::TextUnformatted("Mouse wheel: move shell vertically");
		ImGui::TextUnformatted("A: automatic stress test | E: spawn particles");
		ImGui::TextUnformatted("Space: pause | R: reload JSON and reset | Escape: title");
		ImGui::Separator();
		ImGui::TextWrapped(
			"The collider is interpolated through each physics substep while it moves. "
			"Use the counters above to check whether particles pass through the shell.");
	}
	ImGui::End();
}

SphericalShellCapScene::DemoParameters SphericalShellCapScene::LoadParameters() {
	DemoParameters result{};
	result.gravity = Parameter::GetOr<Vector3>(
		kParameterFileName,
		"World/Gravity",
		result.gravity);
	result.worldBounds.min = Parameter::GetOr<Vector3>(
		kParameterFileName,
		"World/BoundsMin",
		result.worldBounds.min);
	result.worldBounds.max = Parameter::GetOr<Vector3>(
		kParameterFileName,
		"World/BoundsMax",
		result.worldBounds.max);
	if (!Collision::IsValid(result.worldBounds)) {
		result.worldBounds = DemoParameters{}.worldBounds;
	}

	result.substepCount = static_cast<uint32_t>(std::clamp(
		Parameter::GetOr<int32_t>(kParameterFileName, "World/SubstepCount", 8),
		1,
		32));
	result.solverIterationCount = static_cast<uint32_t>(std::clamp(
		Parameter::GetOr<int32_t>(kParameterFileName, "World/SolverIterationCount", 8),
		1,
		32));
	result.linearDamping = std::max(
		Parameter::GetOr<float>(kParameterFileName, "World/LinearDamping", result.linearDamping),
		0.0f);
	result.penetrationSlop = std::max(
		Parameter::GetOr<float>(
			kParameterFileName,
			"World/PenetrationSlop",
			result.penetrationSlop),
		0.0f);
	result.positionCorrectionRate = std::clamp(
		Parameter::GetOr<float>(
			kParameterFileName,
			"World/PositionCorrectionRate",
			result.positionCorrectionRate),
		0.0f,
		1.0f);
	result.maximumDepenetrationVelocity = std::max(
		Parameter::GetOr<float>(
			kParameterFileName,
			"World/MaximumDepenetrationVelocity",
			result.maximumDepenetrationVelocity),
		0.0f);
	result.bounceVelocityThreshold = std::max(
		Parameter::GetOr<float>(
			kParameterFileName,
			"World/BounceVelocityThreshold",
			result.bounceVelocityThreshold),
		0.0f);

	result.particleRadius = std::max(
		Parameter::GetOr<float>(kParameterFileName, "Particle/Radius", result.particleRadius),
		0.001f);
	result.particleMass = std::max(
		Parameter::GetOr<float>(kParameterFileName, "Particle/Mass", result.particleMass),
		0.001f);
	result.particleRestitution = std::clamp(
		Parameter::GetOr<float>(
			kParameterFileName,
			"Particle/Restitution",
			result.particleRestitution),
		0.0f,
		1.0f);
	result.particleFriction = std::clamp(
		Parameter::GetOr<float>(
			kParameterFileName,
			"Particle/Friction",
			result.particleFriction),
		0.0f,
		1.0f);

	result.outerRadius = std::max(
		Parameter::GetOr<float>(kParameterFileName, "Shell/OuterRadius", result.outerRadius),
		0.02f);
	result.innerRadius = std::clamp(
		Parameter::GetOr<float>(kParameterFileName, "Shell/InnerRadius", result.innerRadius),
		0.01f,
		result.outerRadius - 0.01f);
	const float thickness = result.outerRadius - result.innerRadius;
	result.shellHeight = std::clamp(
		Parameter::GetOr<float>(kParameterFileName, "Shell/Height", result.shellHeight),
		thickness + 0.01f,
		result.outerRadius);
	result.shellRestitution = std::clamp(
		Parameter::GetOr<float>(
			kParameterFileName,
			"Shell/Restitution",
			result.shellRestitution),
		0.0f,
		1.0f);
	result.shellFriction = std::clamp(
		Parameter::GetOr<float>(kParameterFileName, "Shell/Friction", result.shellFriction),
		0.0f,
		1.0f);
	return result;
}

Physics::ParticleWorldSettings SphericalShellCapScene::MakeWorldSettings(
	const DemoParameters& parameters) {
	return {
		.gravity = parameters.gravity,
		.worldBounds = parameters.worldBounds,
		.substepCount = parameters.substepCount,
		.solverIterationCount = parameters.solverIterationCount,
		.linearDamping = parameters.linearDamping,
		.penetrationSlop = parameters.penetrationSlop,
		.positionCorrectionRate = parameters.positionCorrectionRate,
		.maximumDepenetrationVelocity = parameters.maximumDepenetrationVelocity,
		.bounceVelocityThreshold = parameters.bounceVelocityThreshold,
	};
}

void SphericalShellCapScene::ReloadParameters() {
	const ParameterIoResult result = Parameter::Reload(kParameterFileName);
	if (!result) {
		parameterStatus_ = "Reload failed: " + result.message;
		return;
	}

	parameters_ = LoadParameters();
	particleWorld_.SetSettings(MakeWorldSettings(parameters_));
	parameterStatus_ = "Reloaded Assets/Parameters/SphericalShellCapDemo.json";
	ResetSimulation();
}

Collision::SphericalShellCap SphericalShellCapScene::MakeShell() const {
	return {
		.outerApex = shellPosition_,
		.axis = { 0.0f, 1.0f, 0.0f },
		.outerRadius = parameters_.outerRadius,
		.innerRadius = parameters_.innerRadius,
		.height = parameters_.shellHeight,
	};
}

void SphericalShellCapScene::ResetSimulation() {
	particleWorld_.Clear();
	shellPosition_ = { 0.0f, 0.05f, 0.0f };
	nextParticleId_ = 0u;
	spawnedParticleCount_ = 0u;
	automaticMoveTime_ = 0.0f;
	isPaused_ = false;
	automaticMove_ = false;

	shellCollider_ = particleWorld_.AddCollider(
		MakeShell(),
		{
			.collisionParameter = {
				.restitution = parameters_.shellRestitution,
				.friction = parameters_.shellFriction,
			},
		});
	SpawnParticles();
	SpawnParticles();
}

void SphericalShellCapScene::SpawnParticles() {
	constexpr uint32_t columnCount = 5u;
	for (uint32_t z = 0u; z < columnCount; ++z) {
		for (uint32_t x = 0u; x < columnCount; ++x) {
			const uint64_t particleId = nextParticleId_++;
			const Physics::ParticleHandle handle = particleWorld_.Add({
				.position = {
					(static_cast<float>(x) - 2.0f) * 0.48f,
					3.2f + static_cast<float>((particleId / 25u) % 5u) * 0.5f,
					(static_cast<float>(z) - 2.0f) * 0.48f,
				},
				.velocity = {},
				.radius = parameters_.particleRadius,
				.mass = parameters_.particleMass,
				.collisionParameter = {
					.restitution = parameters_.particleRestitution,
					.friction = parameters_.particleFriction,
				},
				.userData = particleId,
			});
			if (handle.IsValid()) {
				++spawnedParticleCount_;
			}
		}
	}
}

std::size_t SphericalShellCapScene::CountParticlesOutsideCavity() const {
	const Collision::SphericalShellCap shell = MakeShell();
	const Vector3 sphereCenter = shell.outerApex + shell.axis * shell.outerRadius;
	const float openingHeight = shell.outerApex.y + shell.height;
	std::vector<Physics::ParticleState> states;
	particleWorld_.GetStates(states);

	return static_cast<std::size_t>(std::ranges::count_if(
		states,
		[&](const Physics::ParticleState& state) {
			if (state.position.y >= openingHeight + state.radius) {
				return false;
			}
			const float safeCavityRadius = shell.innerRadius - state.radius;
			return Math::DistanceSq(state.position, sphereCenter) >
				safeCavityRadius * safeCavityRadius;
		}));
}
