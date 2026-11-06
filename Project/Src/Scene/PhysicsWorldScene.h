#pragma once

#include "Scene/SampleScene.h"

#include <cstdint>
#include <string>
#include <vector>

class PhysicsWorldScene final : public SampleScene {
public:
	explicit PhysicsWorldScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	struct BodyParameters {
		LGF::Vector3 position{};
		LGF::Vector3 velocity{};
		float radius = 0.55f;
		float mass = 1.0f;
		float restitution = 1.0f;
		float friction{};
	};

	struct DemoParameters {
		LGF::Vector3 gravity{ 0.0f, -9.8f, 0.0f };
		LGF::Collision::AABB worldBounds{
			.min = { -10.0f, -2.0f, -7.0f },
			.max = { 10.0f, 8.0f, 7.0f },
		};
		LGF::Vector3 floorCenter{ 0.0f, -0.25f, 0.0f };
		LGF::Vector3 floorHalfExtents{ 6.5f, 0.25f, 6.0f };
		LGF::Vector3 wallHalfExtents{ 0.25f, 2.5f, 6.0f };
		BodyParameters elasticLeft{
			.position = { -3.0f, 0.55f, -3.0f },
			.velocity = { 2.0f, 0.0f, 0.0f },
		};
		BodyParameters elasticRight{
			.position = { 3.0f, 0.55f, -3.0f },
			.velocity = { -2.0f, 0.0f, 0.0f },
		};
		BodyParameters light{
			.position = { -3.0f, 0.55f, 0.0f },
			.velocity = { 3.0f, 0.0f, 0.0f },
			.mass = 1.0f,
		};
		BodyParameters heavy{
			.position = { 1.0f, 0.55f, 0.0f },
			.mass = 5.0f,
		};
		BodyParameters inelasticLeft{
			.position = { -3.0f, 0.55f, 3.0f },
			.velocity = { 2.0f, 0.0f, 0.0f },
			.restitution = 0.0f,
		};
		BodyParameters inelasticRight{
			.position = { 3.0f, 0.55f, 3.0f },
			.velocity = { -2.0f, 0.0f, 0.0f },
			.restitution = 0.0f,
		};
		BodyParameters falling{
			.position = { 0.0f, 5.0f, 5.0f },
			.radius = 0.55f,
			.mass = 2.0f,
			.restitution = 0.85f,
			.friction = 0.1f,
		};
		uint32_t substepCount = 8u;
		uint32_t solverIterationCount = 8u;
		float demoDuration = 5.0f;
		float wallPositionX = 6.5f;
		float bounceVelocityThreshold = 0.8f;
	};

	struct DemoObjectDesc {
		BodyParameters body{};
		std::string name;
		LGF::Vector4 color{};
	};

	struct DemoObject {
		LGF::Physics::DynamicObjectHandle handle{};
		std::string name;
		LGF::Vector4 color{};
	};

	static DemoParameters LoadParameters();
	static LGF::Physics::PhysicsWorldSettings MakeWorldSettings(const DemoParameters& parameters);
	void ReloadParameters();
	void ResetSimulation();
	void AddDemoObject(const DemoObjectDesc& desc);

	DemoParameters parameters_;
	LGF::Physics::PhysicsWorld physicsWorld_;
	LGF::Physics::StaticObjectHandle floorObject_{};
	LGF::Physics::StaticObjectHandle leftWallObject_{};
	LGF::Physics::StaticObjectHandle rightWallObject_{};
	std::vector<DemoObject> demoObjects_;
	float demoTime_{};
	std::string parameterStatus_;
	bool isPaused_ = false;
	bool isAutoResetEnabled_ = true;
	mutable bool resetRequested_ = false;
	mutable bool toggleAutoResetRequested_ = false;
};
