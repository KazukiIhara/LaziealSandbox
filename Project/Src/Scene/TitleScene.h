#pragma once

#include "Scene/SampleScene.h"

class TitleScene final : public SampleScene {
public:
	using SampleScene::SampleScene;

	void Update() override;
	void Draw() const override;

private:
	float rotation_ = 0.0f;
	LGF::Box3D box_{ 1.8f };
	LGF::Capsule3D capsule_{ 0.65f, 2.5f };
	LGF::Sphere3D sphere_{ 1.0f };
};
