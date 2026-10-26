#include "Scene/PhysicsWorldScene.h"

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kMoveSpeed = 4.0f;

	constexpr Collision::AABB kWorldBounds{
		.min = { -8.0f, -4.0f, -8.0f },
		.max = { 8.0f, 12.0f, 8.0f },
	};

	void DrawOBB(const Collision::OBB& box, const Vector4& color) {
		Box3D(
			box.halfExtents.x * 2.0f,
			box.halfExtents.y * 2.0f,
			box.halfExtents.z * 2.0f).Draw(
			Math::MakeAffineMatrix(
				{ 1.0f, 1.0f, 1.0f },
				box.rotation,
				box.center),
			color);
	}

	void DrawStaticObject(
		const Physics::PhysicsWorld& world,
		Physics::StaticObjectHandle handle,
		const Vector4& color) {
		const std::optional<Physics::StaticObjectState> state =
			world.GetState(handle);
		if (!state) {
			return;
		}
		DrawOBB(std::get<Collision::OBB>(state->shape), color);
	}

}

PhysicsWorldScene::PhysicsWorldScene(const InitData& init) :
	SampleScene(init),
	physicsWorld_({
		.gravity = { 0.0f, -9.8f, 0.0f },
		.worldBounds = kWorldBounds,
		.substepCount = 4u,
		.solverIterationCount = 4u,
	}) {
	ResetSimulation();
}

void PhysicsWorldScene::Update() {
	if (Key::Space.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::R.Trigger()) {
		ResetSimulation();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	if (!isPaused_) {
		const std::optional<Physics::DynamicObjectState> sphereState =
			physicsWorld_.GetState(sphereHandle_);
		if (sphereState) {
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

			// 水平方向だけを操作し、重力で変化するY速度はそのまま残す。
			physicsWorld_.SetVelocity(sphereHandle_, {
				movement.x * kMoveSpeed,
				sphereState->velocity.y,
				movement.z * kMoveSpeed,
			});
		}
		physicsWorld_.Update(static_cast<float>(System::DeltaTime()));
	}
	UpdateCamera();
}

void PhysicsWorldScene::Draw() const {
	DrawStage();

	DrawStaticObject(physicsWorld_, floorHandle_, Color::DarkGray);
	DrawStaticObject(physicsWorld_, wallHandle_, Color::RoyalBlue);
	DrawStaticObject(physicsWorld_, pillarHandle_, Color::Turquoise);

	const std::optional<Physics::DynamicObjectState> sphereState =
		physicsWorld_.GetState(sphereHandle_);
	if (sphereState) {
		const Collision::Sphere& sphere =
			std::get<Collision::Sphere>(sphereState->shape);
		Sphere3D(sphere.radius).Draw(
			Math::MakeTranslateMatrix(sphere.center),
			Color::Gold);
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("PhysicsWorld Demo")) {
		ImGui::Text(
			"Static objects: %zu",
			physicsWorld_.GetStaticObjectCount());
		ImGui::Text(
			"Dynamic objects: %zu",
			physicsWorld_.GetDynamicObjectCount());
		ImGui::Text("Simulation: %s", isPaused_ ? "Paused" : "Playing");
		if (sphereState) {
			const Collision::Sphere& sphere =
				std::get<Collision::Sphere>(sphereState->shape);
			ImGui::Text(
				"Sphere position: %.3f, %.3f, %.3f",
				sphere.center.x,
				sphere.center.y,
				sphere.center.z);
			ImGui::Text(
				"Sphere velocity: %.3f, %.3f, %.3f",
				sphereState->velocity.x,
				sphereState->velocity.y,
				sphereState->velocity.z);
		} else {
			ImGui::TextUnformatted("Sphere was removed outside the world bounds.");
		}

		ImGui::Separator();
		ImGui::TextUnformatted("WASD: move Sphere");
		ImGui::TextUnformatted("Space: pause | R: reset | Escape: title");
		ImGui::TextWrapped(
			"Floor, wall and pillar are Static OBBs. "
			"Move the Dynamic Sphere into them to check penetration resolution.");
	}
	ImGui::End();
}

void PhysicsWorldScene::ResetSimulation() {
	physicsWorld_.Clear();
	isPaused_ = false;

	const Collision::OBB floor{
		.center = { 0.0f, 0.5f, 0.0f },
		.halfExtents = { 6.0f, 0.5f, 6.0f },
	};
	const Collision::OBB wall{
		.center = { 0.0f, 2.5f, 3.0f },
		.halfExtents = { 3.0f, 1.5f, 0.25f },
		.rotation = Math::MakeQuaternion({ 0.0f, -0.15f, 0.0f }),
	};
	const Collision::OBB pillar{
		.center = { 2.0f, 2.0f, -0.5f },
		.halfExtents = { 0.65f, 1.0f, 0.65f },
		.rotation = Math::MakeQuaternion({ 0.0f, 0.4f, 0.0f }),
	};
	const Collision::Sphere sphere{
		.center = { 0.0f, 3.0f, -4.0f },
		.radius = 0.5f,
	};

	floorHandle_ = physicsWorld_.AddStaticObject(floor);
	wallHandle_ = physicsWorld_.AddStaticObject(wall);
	pillarHandle_ = physicsWorld_.AddStaticObject(pillar);
	sphereHandle_ = physicsWorld_.AddDynamicObject(sphere);
}
