#pragma once

#include "Scene/SampleScene.h"

class TitleScene final : public SampleScene {
public:
	TitleScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	float rotation_ = 0.0f;
	LGF::Box3D box_{ 1.8f };
	LGF::Vector3 boxPos_{ -2.5f, 1.0f, 0.0f };
	LGF::Capsule3D capsule_{ 0.65f, 2.5f };
	LGF::Sphere3D sphere_{ 1.0f };
	LGF::Vector3 spherePos_{ 2.5f, 1.0f, 0.0f };
};
