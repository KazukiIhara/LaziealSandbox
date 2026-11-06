#include "Scene/PhysicsParticleScene.h"

#include <array>
#include <cmath>
#include <vector>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kParticleRadius = 0.18f;
	constexpr Collision::AABB kWorldBounds{
		.min = { -8.0f, -4.0f, -8.0f },
		.max = { 8.0f, 12.0f, 8.0f },
	};
	constexpr Collision::AABB kFloor{
		.min = { -5.0f, -0.5f, -5.0f },
		.max = { 5.0f, 0.0f, 5.0f },
	};
	constexpr Collision::AABB kBoxObstacle{
		.min = { -3.2f, 0.0f, -1.2f },
		.max = { -1.4f, 2.0f, 1.2f },
	};
	constexpr Collision::Sphere kSphereObstacle{
		.center = { 1.5f, 1.25f, 0.0f },
		.radius = 1.25f,
	};
	constexpr Collision::SphericalCap kSphericalCap{
		.apex = { 0.0f, 0.05f, 0.0f },
		.axis = { 0.0f, 1.0f, 0.0f },
		.baseRadius = 3.5f,
		.height = 1.4f,
	};

	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
		Line3D(start, end).Draw(Math::Identity4x4(), color);
	}

	void DrawBounds(const Collision::AABB& box, const Vector4& color) {
		const std::array<Vector3, 8> corners{
			Vector3{ box.min.x, box.min.y, box.min.z },
			Vector3{ box.max.x, box.min.y, box.min.z },
			Vector3{ box.max.x, box.min.y, box.max.z },
			Vector3{ box.min.x, box.min.y, box.max.z },
			Vector3{ box.min.x, box.max.y, box.min.z },
			Vector3{ box.max.x, box.max.y, box.min.z },
			Vector3{ box.max.x, box.max.y, box.max.z },
			Vector3{ box.min.x, box.max.y, box.max.z },
		};
		constexpr std::array<std::array<std::size_t, 2>, 12> edges{
			std::array<std::size_t, 2>{ 0u, 1u }, { 1u, 2u }, { 2u, 3u }, { 3u, 0u },
			{ 4u, 5u }, { 5u, 6u }, { 6u, 7u }, { 7u, 4u },
			{ 0u, 4u }, { 1u, 5u }, { 2u, 6u }, { 3u, 7u },
		};
		for (const auto& edge : edges) {
			DrawLine(corners[edge[0]], corners[edge[1]], color);
		}
	}

	void DrawBox(const Collision::AABB& box, const Vector4& color) {
		const Vector3 size = box.max - box.min;
		const Vector3 center = (box.min + box.max) * 0.5f;
		Box3D(size.x, size.y, size.z).Draw(Math::MakeTranslateMatrix(center), color);
	}

}

PhysicsParticleScene::PhysicsParticleScene(const InitData& init) :
	SampleScene(init),
	physicsWorld_({
		.gravity = { 0.0f, -9.8f, 0.0f },
		.worldBounds = kWorldBounds,
		.substepCount = 4u,
		.solverIterationCount = 6u,
		.linearDamping = 0.08f,
	}) {
	ResetSimulation();
}

void PhysicsParticleScene::Update() {
	if (Key::Space.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::R.Trigger()) {
		ResetSimulation();
	}
	if (Key::E.Trigger()) {
		SpawnParticles();
	}
	if (Key::T.Trigger()) {
		ScatterParticles();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	if (!isPaused_) {
		physicsWorld_.Update(static_cast<float>(System::DeltaTime()));
	}
	UpdateCamera();
}

void PhysicsParticleScene::Draw() const {
	DrawStage();
	DrawBox(kFloor, Color::DarkGray);
	DrawBox(kBoxObstacle, Color::RoyalBlue);
	Sphere3D(kSphereObstacle.radius).Draw(Math::MakeTranslateMatrix(kSphereObstacle.center), Color::Turquoise);
	SphericalCap3D(kSphericalCap.baseRadius, kSphericalCap.height).Draw(
		Math::MakeTranslateMatrix(kSphericalCap.apex),
		Material3D{
			.color = Color::Silver,
			.metallic = 0.85f,
			.roughness = 0.3f,
		});

	DrawBounds(kWorldBounds, Color::Yellow);

	std::vector<Physics::PhysicsParticleState> states;
	physicsWorld_.GetParticleStates(states);
	for (const Physics::PhysicsParticleState& state : states) {
		const Vector4 color = state.handle.index % 3u == 0u
			? Color::Gold
			: state.handle.index % 3u == 1u ? Color::Orange : Color::Yellow;
		Sphere3D(state.radius).Draw(Math::MakeTranslateMatrix(state.position), color);
	}
	const Physics::PhysicsParticleStepStatistics& statistics = physicsWorld_.GetLastParticleStepStatistics();

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("PhysicsParticle Demo")) {
		ImGui::Text("Particles: %zu", physicsWorld_.GetParticleCount());
		ImGui::Text("Static colliders: %zu", physicsWorld_.GetStaticObjectCount());
		ImGui::Text("Physics: %.3f ms | Candidate pairs: %llu", statistics.totalMilliseconds, static_cast<unsigned long long>(statistics.candidatePairCount));
		ImGui::Text("Integrate %.3f | Broad phase %.3f | Position %.3f ms", statistics.integrationMilliseconds, statistics.broadPhaseMilliseconds, statistics.positionSolveMilliseconds);
		ImGui::Text("Velocity %.3f | Cleanup %.3f ms", statistics.velocitySolveMilliseconds, statistics.cleanupMilliseconds);
		ImGui::Text("Simulation: %s", isPaused_ ? "Paused" : "Playing");
		ImGui::Separator();
		ImGui::TextUnformatted("E: spawn particles | T: scatter particles");
		ImGui::TextUnformatted("Space: pause | R: reset | Escape: title");
		ImGui::Separator();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow wire box: world bounds");
		ImGui::TextWrapped("Particles collide with the SphericalCap, AABB and Sphere obstacles, and each other.");
		ImGui::TextWrapped("Particles are automatically removed after their center leaves the world bounds.");
	}
	ImGui::End();
}

void PhysicsParticleScene::ResetSimulation() {
	physicsWorld_.Clear();
	nextParticleId_ = 0u;
	isPaused_ = false;

	const Physics::StaticObjectOptions floorOptions{
		.material = {
			.restitution = 0.0f,
			.friction = 1.0f,
		},
	};
	const Physics::StaticObjectOptions obstacleOptions{
		.material = {
			.restitution = 0.35f,
			.friction = 0.35f,
		},
	};
	physicsWorld_.AddStaticObject(kFloor, floorOptions);
	physicsWorld_.AddStaticObject(kBoxObstacle, obstacleOptions);
	physicsWorld_.AddStaticObject(kSphereObstacle, obstacleOptions);
	physicsWorld_.AddStaticObject(kSphericalCap, floorOptions);

	SpawnParticles();
	SpawnParticles();
}

void PhysicsParticleScene::SpawnParticles() {
	constexpr uint32_t columnCount = 4u;
	for (uint32_t z = 0u; z < columnCount; ++z) {
		for (uint32_t x = 0u; x < columnCount; ++x) {
			const uint64_t particleId = nextParticleId_++;
			physicsWorld_.AddParticle({
				.position = {
					(static_cast<float>(x) - 1.5f) * 0.55f,
					6.0f + static_cast<float>((particleId / 16u) % 4u) * 0.55f,
					(static_cast<float>(z) - 1.5f) * 0.55f,
				},
				.velocity = {
					static_cast<float>(static_cast<int32_t>(z) - 1) * 0.1f,
					0.0f,
					static_cast<float>(static_cast<int32_t>(x) - 1) * 0.1f,
				},
				.radius = kParticleRadius,
				.material = {
					.restitution = 0.0f,
					.friction = 1.0f,
				},
			});

		}
	}
}

void PhysicsParticleScene::ScatterParticles() {
	std::vector<Physics::PhysicsParticleState> states;
	physicsWorld_.GetParticleStates(states);
	for (const Physics::PhysicsParticleState& state : states) {
		const float angle = static_cast<float>(state.handle.index) * 2.399963f;
		physicsWorld_.AddParticleImpulse(state.handle, { std::cos(angle) * 8.0f, 5.0f + static_cast<float>(state.handle.index % 5u) * 0.3f, std::sin(angle) * 8.0f });
	}
}


