#pragma once

#include "Scene/SampleScene.h"

#include <cstdint>
#include <string>

class SphericalShellCapScene final : public SampleScene {
public:
	explicit SphericalShellCapScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	struct DemoParameters {
		LGF::Vector3 gravity{ 0.0f, -9.8f, 0.0f };
		LGF::Collision::AABB worldBounds{
			.min = { -10.0f, -5.0f, -10.0f },
			.max = { 10.0f, 12.0f, 10.0f },
		};
		uint32_t substepCount = 8u;
		uint32_t solverIterationCount = 8u;
		float linearDamping = 0.6f;
		float particleRadius = 0.18f;
		float particleMass = 1.0f;
		float particleRestitution = 0.03f;
		float particleFriction = 0.55f;
		float outerRadius = 5.1f;
		float innerRadius = 4.75f;
		float shellHeight = 1.45f;
		float shellRestitution = 0.02f;
		float shellFriction = 0.65f;
	};

	static DemoParameters LoadParameters();
	static LGF::Physics::ParticleWorldSettings MakeWorldSettings(
		const DemoParameters& parameters);
	void ReloadParameters();
	LGF::Collision::SphericalShellCap MakeShell() const;
	void ResetSimulation();
	void SpawnParticles();
	std::size_t CountParticlesOutsideCavity() const;

	DemoParameters parameters_;
	LGF::Physics::ParticleWorld particleWorld_;
	LGF::Physics::StaticColliderHandle shellCollider_;
	LGF::Vector3 shellPosition_{};
	uint64_t nextParticleId_ = 0u;
	std::size_t spawnedParticleCount_ = 0u;
	float automaticMoveTime_ = 0.0f;
	std::string parameterStatus_;
	bool isPaused_ = false;
	bool automaticMove_ = false;
};
