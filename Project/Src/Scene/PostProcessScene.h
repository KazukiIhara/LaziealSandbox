#pragma once

#include <LGF/LGF.h>

class PostProcessScene {
public:
	PostProcessScene();
	~PostProcessScene();

	void Update();
	void Draw();

private:
	void DrawControlPanel();
	void ApplyPostProcessSettings();
	void ResetPostProcessSettings();

private:
	LGF::DebugCamera3D camera_;
	LGF::DirectionalLight directionalLight_;
	LGF::PointLight warmLight_;
	LGF::PointLight coolLight_;
	LGF::PointLight accentLight_;
	LGF::Skybox skybox_;
	LGF::Ground3D ground_;
	LGF::Grid3D grid_;
	LGF::Sphere3D sphere_;
	LGF::Box3D box_;
	LGF::PostProcessProfileHandle postProcessProfile_{};

	bool bloomEnabled_ = true;
	float bloomThreshold_ = 1.0f;
	float bloomSoftKnee_ = 0.5f;
	float bloomIntensity_ = 0.15f;
	float bloomRadius_ = 0.7f;

	bool colorAdjustmentEnabled_ = true;
	float brightness_ = 0.0f;
	float contrast_ = 1.0f;
	float saturation_ = 1.0f;
	float colorAdjustmentIntensity_ = 1.0f;
	LGF::Vector4 tint_ = LGF::Color::White;

	bool vignetteEnabled_ = true;
	float vignetteIntensity_ = 0.25f;
	float vignetteSmoothness_ = 0.5f;
	float vignetteRoundness_ = 1.0f;
	LGF::Vector4 vignetteColor_ = LGF::Color::Black;
	float exposureEV_ = 0.0f;
};
