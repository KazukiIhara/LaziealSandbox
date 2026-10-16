#pragma once

#include "Scene/SampleScene.h"

#include <cstdint>

class ParticleWorldScene final : public SampleScene {
public:
	explicit ParticleWorldScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	void ResetSimulation();
	void SpawnParticles();
	void ScatterParticles();

	LGF::Physics::ParticleWorld particleWorld_;
	uint64_t nextParticleId_ = 0u;
	bool isPaused_ = false;
};
