#pragma once

#include "Scene/SampleScene.h"

class OBBCollisionScene final : public SampleScene {
public:
	explicit OBBCollisionScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	void ResetDemo();

	LGF::Collision::OBB fixedBox_{};
	LGF::Collision::OBB movingBox_{};
	LGF::Collision::Sphere testSphere_{};
	LGF::Collision::Ray testRay_{};
	LGF::Vector3 movingRotation_{};
};
