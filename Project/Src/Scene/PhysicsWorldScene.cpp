#include "Scene/PhysicsWorldScene.h"

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

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
		physicsWorld_.Update(static_cast<float>(System::DeltaTime()));
	}
	UpdateCamera();
}

void PhysicsWorldScene::Draw() const {
	DrawStage();

	const std::optional<Physics::StaticObjectState> floorState =
		physicsWorld_.GetState(floorHandle_);
	if (floorState) {
		DrawOBB(
			std::get<Collision::OBB>(floorState->shape),
			Color::DarkGray);
	}

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
		ImGui::TextUnformatted("Space: pause | R: reset | Escape: title");
		ImGui::TextWrapped(
			"Current scope: Static OBB and Dynamic Sphere. "
			"The sphere is pushed out without restitution or friction.");
	}
	ImGui::End();
}

void PhysicsWorldScene::ResetSimulation() {
	physicsWorld_.Clear();
	isPaused_ = false;

	const Collision::OBB floor{
		.center = { 0.0f, 0.5f, 0.0f },
		.halfExtents = { 4.0f, 0.5f, 3.0f },
		.rotation = Math::MakeQuaternion({ 0.0f, 0.3f, 0.0f }),
	};
	const Collision::Sphere sphere{
		.center = { 0.0f, 5.0f, 0.0f },
		.radius = 0.5f,
	};

	floorHandle_ = physicsWorld_.AddStaticObject(floor);
	sphereHandle_ = physicsWorld_.AddDynamicObject(sphere);
}
