#pragma once

#include "Scene/SampleScene.h"

class ResultScene final : public SampleScene {
public:
	using SampleScene::SampleScene;

	void Update() override;
	void Draw() const override;

private:
	float rotation_ = 0.0f;
	LGF::Box3D pedestal_{ 5.0f, 0.7f, 5.0f };
	LGF::Sphere3D marker_{ 0.55f };
};
