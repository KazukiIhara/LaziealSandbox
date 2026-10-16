#pragma once

#include "Scene/SampleScene.h"

#include <cstdint>

class ParticleWorldScene final : public SampleScene {
public:
	explicit ParticleWorldScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	enum class BoundaryType : uint8_t {
		Box,
		Sphere,
		Cylinder,
	};

	void ResetSimulation();
	void TossParticles();

	LGF::Physics::ParticleWorld particleWorld_;
	BoundaryType boundaryType_ = BoundaryType::Sphere;
	bool isPaused_ = false;
};
