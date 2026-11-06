#pragma once

#include "Scene/SampleScene.h"

#include <cstdint>

class PhysicsParticleScene final : public SampleScene {
public:
	explicit PhysicsParticleScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	void ResetSimulation();
	void SpawnParticles();
	void ScatterParticles();

	LGF::Physics::PhysicsWorld physicsWorld_;
	uint64_t nextParticleId_ = 0u;
	bool isPaused_ = false;
};

