#pragma once

#include <array>

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
	std::array<LGF::PointLight, 3> pointLights_{
		LGF::PointLight{ {}, LGF::Color::Tomato, 6.0f, 7.0f },
		LGF::PointLight{ {}, LGF::Color::Lime, 6.0f, 7.0f },
		LGF::PointLight{ {}, LGF::Color::DodgerBlue, 6.0f, 7.0f },
	};
	LGF::Sphere3D lightMarker_{ 0.25f };
	LGF::Sphere3D sphere_{ 0.8f };
	LGF::Box3D box_{ 1.4f };
};
