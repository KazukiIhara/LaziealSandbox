#include "Scene/ParticleWorldScene.h"

#include <array>
#include <cmath>
#include <vector>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kParticleRadius = 0.18f;
	constexpr float kCuttingHeight = 4.2f;
	constexpr uint32_t kCircleDivision = 32u;
	constexpr Vector4 kBoundaryColor{ 0.2f, 0.8f, 1.0f, 1.0f };
	constexpr Vector4 kCuttingPlaneColor{ 1.0f, 0.25f, 0.2f, 1.0f };

	const char* GetBoundaryName(uint8_t boundaryType) {
		switch (boundaryType) {
		case 0u:
			return "Box";
		case 1u:
			return "Sphere";
		case 2u:
			return "Cylinder";
		default:
			return "Unknown";
		}
	}

	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
		Line3D(start, end).Draw(Math::Identity4x4(), color);
	}

	void DrawCircle(float radius, float height, const Vector4& color) {
		for (uint32_t index = 0u; index < kCircleDivision; ++index) {
			const float firstAngle = Math::PI * 2.0f *
				static_cast<float>(index) / static_cast<float>(kCircleDivision);
			const float secondAngle = Math::PI * 2.0f *
				static_cast<float>(index + 1u) / static_cast<float>(kCircleDivision);
			DrawLine(
				{ std::cos(firstAngle) * radius, height, std::sin(firstAngle) * radius },
				{ std::cos(secondAngle) * radius, height, std::sin(secondAngle) * radius },
				color);
		}
	}

	void DrawBoxBoundary() {
		constexpr float radius = 3.0f;
		constexpr float bottom = 0.2f;
		const std::array<Vector3, 4> bottomCorners{
			Vector3{ -radius, bottom, -radius },
			Vector3{  radius, bottom, -radius },
			Vector3{  radius, bottom,  radius },
			Vector3{ -radius, bottom,  radius },
		};
		const std::array<Vector3, 4> cutCorners{
			Vector3{ -radius, kCuttingHeight, -radius },
			Vector3{  radius, kCuttingHeight, -radius },
			Vector3{  radius, kCuttingHeight,  radius },
			Vector3{ -radius, kCuttingHeight,  radius },
		};

		for (std::size_t index = 0u; index < bottomCorners.size(); ++index) {
			const std::size_t next = (index + 1u) % bottomCorners.size();
			DrawLine(bottomCorners[index], bottomCorners[next], kBoundaryColor);
			DrawLine(bottomCorners[index], cutCorners[index], kBoundaryColor);
			DrawLine(cutCorners[index], cutCorners[next], kCuttingPlaneColor);
		}
	}

	void DrawSphereBoundary() {
		constexpr Vector3 center{ 0.0f, 3.0f, 0.0f };
		constexpr float radius = 3.0f;
		constexpr uint32_t latitudeDivision = 8u;

		for (uint32_t latitude = 1u; latitude <= latitudeDivision; ++latitude) {
			const float height = center.y - radius +
				(kCuttingHeight - (center.y - radius)) *
				static_cast<float>(latitude) / static_cast<float>(latitudeDivision);
			const float offset = height - center.y;
			const float circleRadius = std::sqrt(radius * radius - offset * offset);
			DrawCircle(
				circleRadius,
				height,
				latitude == latitudeDivision ? kCuttingPlaneColor : kBoundaryColor);
		}

		constexpr uint32_t longitudeDivision = 12u;
		constexpr uint32_t segmentDivision = 12u;
		for (uint32_t longitude = 0u; longitude < longitudeDivision; ++longitude) {
			const float angle = Math::PI * 2.0f *
				static_cast<float>(longitude) / static_cast<float>(longitudeDivision);
			for (uint32_t segment = 0u; segment < segmentDivision; ++segment) {
				const float firstHeight = center.y - radius +
					(kCuttingHeight - (center.y - radius)) *
					static_cast<float>(segment) / static_cast<float>(segmentDivision);
				const float secondHeight = center.y - radius +
					(kCuttingHeight - (center.y - radius)) *
					static_cast<float>(segment + 1u) / static_cast<float>(segmentDivision);
				const float firstRadius = std::sqrt(
					radius * radius - (firstHeight - center.y) * (firstHeight - center.y));
				const float secondRadius = std::sqrt(
					radius * radius - (secondHeight - center.y) * (secondHeight - center.y));
				DrawLine(
					{ std::cos(angle) * firstRadius, firstHeight, std::sin(angle) * firstRadius },
					{ std::cos(angle) * secondRadius, secondHeight, std::sin(angle) * secondRadius },
					kBoundaryColor);
			}
		}
	}

	void DrawCylinderBoundary() {
		constexpr float radius = 3.0f;
		constexpr float bottom = 0.2f;
		DrawCircle(radius, bottom, kBoundaryColor);
		DrawCircle(radius, kCuttingHeight, kCuttingPlaneColor);

		constexpr uint32_t sideDivision = 12u;
		for (uint32_t index = 0u; index < sideDivision; ++index) {
			const float angle = Math::PI * 2.0f *
				static_cast<float>(index) / static_cast<float>(sideDivision);
			const float x = std::cos(angle) * radius;
			const float z = std::sin(angle) * radius;
			DrawLine({ x, bottom, z }, { x, kCuttingHeight, z }, kBoundaryColor);
		}
	}

}

ParticleWorldScene::ParticleWorldScene(const InitData& init) :
	SampleScene(init),
	particleWorld_({
		.gravity = { 0.0f, -9.8f, 0.0f },
		.substepCount = 4u,
		.solverIterationCount = 6u,
		.linearDamping = 0.08f,
	}) {
	ResetSimulation();
}

void ParticleWorldScene::Update() {
	if (Key::Num1.Trigger()) {
		boundaryType_ = BoundaryType::Box;
		ResetSimulation();
	} else if (Key::Num2.Trigger()) {
		boundaryType_ = BoundaryType::Sphere;
		ResetSimulation();
	} else if (Key::Num3.Trigger()) {
		boundaryType_ = BoundaryType::Cylinder;
		ResetSimulation();
	}

	if (Key::Space.Trigger()) {
		isPaused_ = !isPaused_;
	}
	if (Key::R.Trigger()) {
		ResetSimulation();
	}
	if (Key::T.Trigger()) {
		TossParticles();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	if (!isPaused_) {
		particleWorld_.Step(static_cast<float>(System::DeltaTime()));
	}
	UpdateCamera();
}

void ParticleWorldScene::Draw() const {
	DrawStage();

	switch (boundaryType_) {
	case BoundaryType::Box:
		DrawBoxBoundary();
		break;
	case BoundaryType::Sphere:
		DrawSphereBoundary();
		break;
	case BoundaryType::Cylinder:
		DrawCylinderBoundary();
		break;
	}

	std::vector<Physics::ParticleState> states;
	particleWorld_.GetStates(states);
	for (const Physics::ParticleState& state : states) {
		const Vector4 color = state.userData % 3u == 0u
			? Color::Gold
			: state.userData % 3u == 1u ? Color::Orange : Color::Yellow;
		Sphere3D(state.radius).Draw(Math::MakeTranslateMatrix(state.position), color);
	}

	DrawLine(
		{ 0.0f, kCuttingHeight, 0.0f },
		{ 0.0f, kCuttingHeight + 1.5f, 0.0f },
		kCuttingPlaneColor);

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("ParticleWorld Demo")) {
		ImGui::Text("Boundary: %s", GetBoundaryName(static_cast<uint8_t>(boundaryType_)));
		ImGui::Text("Particles: %zu", particleWorld_.GetParticleCount());
		ImGui::Text("Simulation: %s", isPaused_ ? "Paused" : "Playing");
		ImGui::Separator();
		ImGui::TextUnformatted("1 / 2 / 3: Box / Sphere / Cylinder");
		ImGui::TextUnformatted("T: toss particles | Space: pause | R: reset");
		ImGui::TextUnformatted("Escape: open the original sample scenes");
		ImGui::Separator();
		ImGui::TextColored(
			ImVec4(1.0f, 0.25f, 0.2f, 1.0f),
			"Red edge: cutting plane (normal points upward)");
		ImGui::TextWrapped("The surface above the plane is removed, so tossed particles can leave the container.");
	}
	ImGui::End();
}

void ParticleWorldScene::ResetSimulation() {
	particleWorld_.Clear();
	isPaused_ = false;

	const Physics::ParticleBoundaryOptions boundaryOptions{
		.collisionParameter = {
			.restitution = 0.35f,
			.friction = 0.3f,
		},
		.cuttingPlane = Collision::Plane{
			.normal = { 0.0f, 1.0f, 0.0f },
			.distance = -kCuttingHeight,
		},
	};

	switch (boundaryType_) {
	case BoundaryType::Box:
		particleWorld_.AddBoundary(
			Collision::AABB{
				.min = { -3.0f, 0.2f, -3.0f },
				.max = { 3.0f, 6.0f, 3.0f },
			},
			boundaryOptions);
		break;
	case BoundaryType::Sphere:
		particleWorld_.AddBoundary(
			Collision::Sphere{
				.center = { 0.0f, 3.0f, 0.0f },
				.radius = 3.0f,
			},
			boundaryOptions);
		break;
	case BoundaryType::Cylinder:
		particleWorld_.AddBoundary(
			Collision::Cylinder{
				.start = { 0.0f, 0.2f, 0.0f },
				.end = { 0.0f, 6.0f, 0.0f },
				.radius = 3.0f,
			},
			boundaryOptions);
		break;
	}

	constexpr uint32_t columnCount = 4u;
	constexpr uint32_t layerCount = 4u;
	for (uint32_t layer = 0u; layer < layerCount; ++layer) {
		for (uint32_t z = 0u; z < columnCount; ++z) {
			for (uint32_t x = 0u; x < columnCount; ++x) {
				const uint64_t particleIndex =
					layer * columnCount * columnCount + z * columnCount + x;
				particleWorld_.Add({
					.position = {
						(static_cast<float>(x) - 1.5f) * 0.48f,
						2.0f + static_cast<float>(layer) * 0.48f,
						(static_cast<float>(z) - 1.5f) * 0.48f,
					},
					.velocity = {
						static_cast<float>(static_cast<int32_t>(z) - 1) * 0.12f,
						0.0f,
						static_cast<float>(static_cast<int32_t>(x) - 1) * 0.12f,
					},
					.radius = kParticleRadius,
					.collisionParameter = {
						.restitution = 0.25f,
						.friction = 0.25f,
					},
					.userData = particleIndex,
				});
			}
		}
	}
}

void ParticleWorldScene::TossParticles() {
	std::vector<Physics::ParticleState> states;
	particleWorld_.GetStates(states);
	for (const Physics::ParticleState& state : states) {
		const float angle = static_cast<float>(state.userData) * 2.399963f;
		particleWorld_.AddImpulse(
			state.handle,
			{
				std::cos(angle) * 2.0f,
				6.5f + static_cast<float>(state.userData % 5u) * 0.25f,
				std::sin(angle) * 2.0f,
			});
	}
}
