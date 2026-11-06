#include "Scene/PhysicsWorldScene.h"

#include <algorithm>
#include <cmath>
#include <string_view>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr std::string_view kParameterFileName = "PhysicsWorldDemo.json";

	ImVec4 ToImVec4(const Vector4& color) {
		return { color.x, color.y, color.z, color.w };
	}

	void DrawStaticOBB(const Physics::PhysicsWorld& world, Physics::StaticObjectHandle handle, const Vector4& color) {
		const std::optional<Physics::StaticObjectState> state = world.GetState(handle);
		if (!state) {
			return;
		}
		const Collision::OBB& box = std::get<Collision::OBB>(state->shape);
		Box3D(box.halfExtents.x * 2.0f, box.halfExtents.y * 2.0f, box.halfExtents.z * 2.0f).Draw(Math::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, box.rotation, box.center), color);
	}

}

PhysicsWorldScene::PhysicsWorldScene(const InitData& init) :
	SampleScene(init),
	parameters_(LoadParameters()),
	physicsWorld_(MakeWorldSettings(parameters_)),
	parameterStatus_("Loaded Assets/Parameters/PhysicsWorldDemo.json") {
	ResetSimulation();
}

void PhysicsWorldScene::Update() {
	if (resetRequested_) {
		ResetSimulation();
		resetRequested_ = false;
	}
	if (toggleAutoResetRequested_) {
		isAutoResetEnabled_ = !isAutoResetEnabled_;
		toggleAutoResetRequested_ = false;
	}
	if (Key::Space.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::A.Trigger()) {
		isAutoResetEnabled_ = !isAutoResetEnabled_;
	}
	if (Key::R.Trigger()) {
		ReloadParameters();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	if (!isPaused_) {
		const float deltaTime = static_cast<float>(System::DeltaTime());
		physicsWorld_.Update(deltaTime);
		demoTime_ += deltaTime;
		if (isAutoResetEnabled_ && demoTime_ >= parameters_.demoDuration) {
			ResetSimulation();
		}
	}
	UpdateCamera();
}

void PhysicsWorldScene::Draw() const {
	DrawStage();
	DrawStaticOBB(physicsWorld_, floorObject_, Color::Silver);
	DrawStaticOBB(physicsWorld_, leftWallObject_, Color::RoyalBlue);
	DrawStaticOBB(physicsWorld_, rightWallObject_, Color::RoyalBlue);

	Line3D({ -6.5f, 0.03f, -1.5f }, { 6.5f, 0.03f, -1.5f }).Draw(Math::Identity4x4(), Color::Gray);
	Line3D({ -6.5f, 0.03f, 1.5f }, { 6.5f, 0.03f, 1.5f }).Draw(Math::Identity4x4(), Color::Gray);

	for (const DemoObject& object : demoObjects_) {
		const std::optional<Physics::DynamicObjectState> state = physicsWorld_.GetState(object.handle);
		if (!state) {
			continue;
		}
		const Collision::Sphere& sphere = std::get<Collision::Sphere>(state->shape);
		Sphere3D(sphere.radius).Draw(Math::MakeTranslateMatrix(sphere.center), object.color);
		Line3D(sphere.center, sphere.center + state->velocity * 0.35f).Draw(Math::Identity4x4(), object.color);
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(510.0f, 0.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Dynamic Sphere Collision Demo")) {
		ImGui::Text("Static objects: %zu", physicsWorld_.GetStaticObjectCount());
		ImGui::Text("Dynamic objects: %zu", physicsWorld_.GetDynamicObjectCount());
		ImGui::Text("Simulation: %s", isPaused_ ? "Paused" : "Playing");
		ImGui::Text("Auto restart: %s | Time: %.2f / %.2f", isAutoResetEnabled_ ? "On" : "Off", demoTime_, parameters_.demoDuration);
		if (ImGui::Button("Restart demo", ImVec2(-1.0f, 0.0f))) {
			resetRequested_ = true;
		}
		if (ImGui::Button(isAutoResetEnabled_ ? "Disable auto restart" : "Enable auto restart", ImVec2(-1.0f, 0.0f))) {
			toggleAutoResetRequested_ = true;
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Lane z=-3: equal mass / elastic collision");
		ImGui::TextUnformatted("Lane z= 0: mass 1 sphere hits mass 5 sphere");
		ImGui::TextUnformatted("Lane z=+3: equal mass / inelastic collision");
		ImGui::TextUnformatted("Drop test z=+5: mass 2 sphere falls onto the floor");
		ImGui::Separator();

		for (const DemoObject& object : demoObjects_) {
			const std::optional<Physics::DynamicObjectState> state = physicsWorld_.GetState(object.handle);
			if (!state) {
				continue;
			}
			ImGui::TextColored(ToImVec4(object.color), "%s | mass %.1f | restitution %.1f | velocity (%.2f, %.2f, %.2f)", object.name.c_str(), state->mass, state->material.restitution, state->velocity.x, state->velocity.y, state->velocity.z);
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Velocity lines show the current direction and speed.");
		ImGui::TextUnformatted("Space: pause | A: auto restart | R: reload and reset | Escape: title");
		ImGui::TextWrapped("Parameters: %s", parameterStatus_.c_str());
	}
	ImGui::End();
}

PhysicsWorldScene::DemoParameters PhysicsWorldScene::LoadParameters() {
	DemoParameters result{};
	result.gravity = Parameter::GetOr<Vector3>(kParameterFileName, "World/Gravity", result.gravity);
	result.worldBounds.min = Parameter::GetOr<Vector3>(kParameterFileName, "World/BoundsMin", result.worldBounds.min);
	result.worldBounds.max = Parameter::GetOr<Vector3>(kParameterFileName, "World/BoundsMax", result.worldBounds.max);
	if (!Collision::IsValid(result.worldBounds)) {
		result.worldBounds = DemoParameters{}.worldBounds;
	}
	result.substepCount = static_cast<uint32_t>(std::clamp(Parameter::GetOr<int32_t>(kParameterFileName, "World/SubstepCount", 8), 1, 32));
	result.solverIterationCount = static_cast<uint32_t>(std::clamp(Parameter::GetOr<int32_t>(kParameterFileName, "World/SolverIterationCount", 8), 1, 32));
	result.bounceVelocityThreshold = std::max(Parameter::GetOr<float>(kParameterFileName, "World/BounceVelocityThreshold", result.bounceVelocityThreshold), 0.0f);
	result.demoDuration = std::max(Parameter::GetOr<float>(kParameterFileName, "Demo/Duration", result.demoDuration), 0.1f);
	result.floorCenter = Parameter::GetOr<Vector3>(kParameterFileName, "Stage/FloorCenter", result.floorCenter);
	result.floorHalfExtents = Parameter::GetOr<Vector3>(kParameterFileName, "Stage/FloorHalfExtents", result.floorHalfExtents);
	result.wallHalfExtents = Parameter::GetOr<Vector3>(kParameterFileName, "Stage/WallHalfExtents", result.wallHalfExtents);
	result.wallPositionX = std::abs(Parameter::GetOr<float>(kParameterFileName, "Stage/WallPositionX", result.wallPositionX));
	result.floorHalfExtents.x = std::max(std::abs(result.floorHalfExtents.x), 0.05f);
	result.floorHalfExtents.y = std::max(std::abs(result.floorHalfExtents.y), 0.05f);
	result.floorHalfExtents.z = std::max(std::abs(result.floorHalfExtents.z), 0.05f);
	result.wallHalfExtents.x = std::max(std::abs(result.wallHalfExtents.x), 0.05f);
	result.wallHalfExtents.y = std::max(std::abs(result.wallHalfExtents.y), 0.05f);
	result.wallHalfExtents.z = std::max(std::abs(result.wallHalfExtents.z), 0.05f);

	const auto loadBody = [](std::string_view group, const BodyParameters& defaultValue) {
		BodyParameters body = defaultValue;
		const std::string prefix{ group };
		body.position = Parameter::GetOr<Vector3>(kParameterFileName, prefix + "/Position", body.position);
		body.velocity = Parameter::GetOr<Vector3>(kParameterFileName, prefix + "/Velocity", body.velocity);
		body.radius = std::max(Parameter::GetOr<float>(kParameterFileName, prefix + "/Radius", body.radius), 0.05f);
		body.mass = std::max(Parameter::GetOr<float>(kParameterFileName, prefix + "/Mass", body.mass), 0.01f);
		body.restitution = std::clamp(Parameter::GetOr<float>(kParameterFileName, prefix + "/Restitution", body.restitution), 0.0f, 1.0f);
		body.friction = std::clamp(Parameter::GetOr<float>(kParameterFileName, prefix + "/Friction", body.friction), 0.0f, 1.0f);
		return body;
	};

	result.elasticLeft = loadBody("Elastic/Left", result.elasticLeft);
	result.elasticRight = loadBody("Elastic/Right", result.elasticRight);
	result.light = loadBody("Mass/Light", result.light);
	result.heavy = loadBody("Mass/Heavy", result.heavy);
	result.inelasticLeft = loadBody("Inelastic/Left", result.inelasticLeft);
	result.inelasticRight = loadBody("Inelastic/Right", result.inelasticRight);
	result.falling = loadBody("Drop/Falling", result.falling);
	return result;
}

Physics::PhysicsWorldSettings PhysicsWorldScene::MakeWorldSettings(const DemoParameters& parameters) {
	return {
		.gravity = parameters.gravity,
		.worldBounds = parameters.worldBounds,
		.substepCount = parameters.substepCount,
		.solverIterationCount = parameters.solverIterationCount,
		.linearDamping = 0.0f,
		.bounceVelocityThreshold = parameters.bounceVelocityThreshold,
	};
}

void PhysicsWorldScene::ReloadParameters() {
	const ParameterIoResult result = Parameter::Reload(kParameterFileName);
	if (!result) {
		parameterStatus_ = "Reload failed: " + result.message;
		return;
	}
	parameters_ = LoadParameters();
	physicsWorld_.SetSettings(MakeWorldSettings(parameters_));
	parameterStatus_ = "Reloaded Assets/Parameters/PhysicsWorldDemo.json";
	ResetSimulation();
}

void PhysicsWorldScene::ResetSimulation() {
	physicsWorld_.Clear();
	demoObjects_.clear();
	demoObjects_.reserve(7u);
	demoTime_ = 0.0f;
	isPaused_ = false;

	const Collision::OBB floor{ .center = parameters_.floorCenter, .halfExtents = parameters_.floorHalfExtents };
	const Collision::OBB leftWall{ .center = { -parameters_.wallPositionX, parameters_.wallHalfExtents.y, 0.0f }, .halfExtents = parameters_.wallHalfExtents };
	const Collision::OBB rightWall{ .center = { parameters_.wallPositionX, parameters_.wallHalfExtents.y, 0.0f }, .halfExtents = parameters_.wallHalfExtents };
	const Physics::StaticObjectOptions floorOptions{ .material = { .restitution = 0.0f, .friction = 0.0f } };
	const Physics::StaticObjectOptions wallOptions{ .material = { .restitution = 0.8f, .friction = 0.0f } };
	floorObject_ = physicsWorld_.AddStaticObject(floor, floorOptions);
	leftWallObject_ = physicsWorld_.AddStaticObject(leftWall, wallOptions);
	rightWallObject_ = physicsWorld_.AddStaticObject(rightWall, wallOptions);

	AddDemoObject({ .body = parameters_.elasticLeft, .name = "Cyan elastic", .color = Color::Cyan });
	AddDemoObject({ .body = parameters_.elasticRight, .name = "Blue elastic", .color = Color::RoyalBlue });
	AddDemoObject({ .body = parameters_.light, .name = "Gold light", .color = Color::Gold });
	AddDemoObject({ .body = parameters_.heavy, .name = "Magenta heavy", .color = Color::Magenta });
	AddDemoObject({ .body = parameters_.inelasticLeft, .name = "Orange inelastic", .color = Color::Orange });
	AddDemoObject({ .body = parameters_.inelasticRight, .name = "Lime inelastic", .color = Color::Lime });
	AddDemoObject({ .body = parameters_.falling, .name = "Yellow falling", .color = Color::Yellow });
}

void PhysicsWorldScene::AddDemoObject(const DemoObjectDesc& desc) {
	const Physics::DynamicObjectHandle handle = physicsWorld_.AddDynamicObject({
		.shape = Physics::DynamicObjectShape{ Collision::Sphere{ .center = desc.body.position, .radius = desc.body.radius } },
		.velocity = desc.body.velocity,
		.mass = desc.body.mass,
		.material = {
			.restitution = desc.body.restitution,
			.friction = desc.body.friction,
		},
	});

	demoObjects_.push_back({ .handle = handle, .name = desc.name, .color = desc.color });
}
