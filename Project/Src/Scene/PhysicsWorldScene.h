#pragma once

#include "Scene/SampleScene.h"

class PhysicsWorldScene final : public SampleScene {
public:
	explicit PhysicsWorldScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	void ResetSimulation();

	LGF::Physics::PhysicsWorld physicsWorld_;
	LGF::Physics::StaticObjectHandle floorHandle_{};
	LGF::Physics::DynamicObjectHandle sphereHandle_{};
	bool isPaused_ = false;
};
