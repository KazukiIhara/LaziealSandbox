#pragma once

#include "Scene/SampleScene.h"

class PointLightScene final : public SampleScene {
public:
	using SampleScene::SampleScene;

	void Update() override;
	void Draw() const override;

private:
	float lightAngle_ = 0.0f;
	bool isPaused_ = false;
	LGF::DirectionalLight fillLight_{ { 0.5f, -1.0f, 0.35f }, LGF::Color::White, 0.08f };
	LGF::PointLight pointLight_{ {}, LGF::Color::Orange, 8.0f, 8.0f };
	LGF::Sphere3D lightMarker_{ 0.25f };
	LGF::Sphere3D sphere_{ 0.8f };
	LGF::Box3D box_{ 1.4f };
};
