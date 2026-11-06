#pragma once

#include "Scene/SampleScene.h"

#include <cstdint>
#include <string>

class PhysicsShowcaseScene final : public SampleScene {
public:
	explicit PhysicsShowcaseScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	struct DemoParameters {
		LGF::Vector3 gravity{ 0.0f, -9.8f, 0.0f };
		LGF::Collision::AABB worldBounds{
			.min = { -12.0f, -5.0f, -12.0f },
			.max = { 12.0f, 12.0f, 12.0f },
		};
		LGF::Collision::AABB floor{
			.min = { -10.0f, -0.5f, -10.0f },
			.max = { 10.0f, 0.0f, 10.0f },
		};
		LGF::Vector3 shellPosition{ 3.0f, 0.05f, 0.0f };
		LGF::Vector3 shellShakeAmplitude{ 2.7f, 0.0f, 1.3f };
		LGF::Vector3 shellShakeAngularSpeed{ 2.2f, 0.0f, 1.35f };
		LGF::Vector3 particleSpawnCenter{ 3.0f, 3.0f, 0.0f };
		LGF::Vector3 playerPosition{ -5.0f, 0.6f, 0.0f };
		LGF::Vector3 wallCenter{ -1.0f, 1.25f, -3.0f };
		LGF::Vector3 wallHalfExtents{ 4.0f, 1.25f, 0.25f };
		LGF::Vector3 wallRotationDegrees{};
		LGF::Vector3 rampCenter{ -2.5f, 0.325f, 3.0f };
		LGF::Vector3 rampHalfExtents{ 2.5f, 0.2f, 1.5f };
		LGF::Vector3 rampRotationDegrees{ 0.0f, 0.0f, 12.0f };
		uint32_t substepCount = 8u;
		uint32_t solverIterationCount = 8u;
		uint32_t particleColumnCount = 5u;
		uint32_t particleLayerCount = 2u;
		float linearDamping = 0.6f;
		float shellOuterRadius = 4.2f;
		float shellInnerRadius = 3.85f;
		float shellHeight = 1.6f;
		float shellRestitution = 0.02f;
		float shellFriction = 0.65f;
		float particleRadius = 0.18f;
		float particleMass = 1.0f;
		float particleRestitution = 0.03f;
		float particleFriction = 0.55f;
		float particleSpacing = 0.48f;
		float playerRadius = 0.5f;
		float playerMoveSpeed = 5.0f;
		float playerJumpSpeed = 6.5f;
	};

	static DemoParameters LoadParameters();
	static LGF::Physics::PhysicsWorldSettings MakeWorldSettings(const DemoParameters& parameters);
	void ReloadParameters();
	LGF::Collision::SphericalShellCap MakeShell() const;
	LGF::Collision::OBB MakeWall() const;
	LGF::Collision::OBB MakeRamp() const;
	void ResetSimulation();
	void SpawnParticles();
	void UpdateShell(float deltaTime);
	void UpdatePlayer();
	bool IsPlayerGrounded() const;

	DemoParameters parameters_;
	LGF::Physics::PhysicsWorld physicsWorld_;
	LGF::Physics::StaticObjectHandle floorObject_{};
	LGF::Physics::KinematicObjectHandle shellObject_{};
	LGF::Physics::StaticObjectHandle wallObject_{};
	LGF::Physics::StaticObjectHandle rampObject_{};
	LGF::Physics::DynamicObjectHandle playerObject_{};
	LGF::Vector3 shellPosition_{};
	float shellShakeTime_ = 0.0f;
	std::string parameterStatus_;
	bool isPaused_ = false;
	bool isShellShaking_ = false;
	bool isPlayerGrounded_ = false;
	mutable bool toggleShellShakeRequested_ = false;
};
