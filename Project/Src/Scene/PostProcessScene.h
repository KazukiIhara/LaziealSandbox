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

	bool grayscaleEnabled_ = false;
	float grayscaleIntensity_ = 1.0f;

	bool posterizationEnabled_ = false;
	float posterizationColorSteps_ = 8.0f;
	float posterizationIntensity_ = 1.0f;

	bool sepiaEnabled_ = false;
	float sepiaIntensity_ = 1.0f;

	bool colorInversionEnabled_ = false;
	float colorInversionIntensity_ = 1.0f;

	bool lensDistortionEnabled_ = false;
	float lensDistortion_ = 0.15f;
	float lensDistortionScale_ = 1.0f;
	LGF::Vector2 lensDistortionCenter_ = { 0.5f, 0.5f };

	bool radialBlurEnabled_ = false;
	LGF::Vector2 radialBlurCenter_ = { 0.5f, 0.5f };
	float radialBlurStrength_ = 0.08f;
	float radialBlurSampleCount_ = 12.0f;

	bool pixelationEnabled_ = false;
	float pixelSize_ = 8.0f;

	bool orderedDitheringEnabled_ = false;
	float orderedDitheringColorSteps_ = 8.0f;
	float orderedDitheringIntensity_ = 1.0f;
	float orderedDitheringScale_ = 1.0f;

	bool chromaticAberrationEnabled_ = false;
	float chromaticAberrationIntensity_ = 2.0f;

	bool vignetteEnabled_ = true;
	float vignetteIntensity_ = 0.25f;
	float vignetteSmoothness_ = 0.5f;
	float vignetteRoundness_ = 1.0f;
	LGF::Vector4 vignetteColor_ = LGF::Color::Black;

	bool fxaaEnabled_ = true;
	float fxaaEdgeThreshold_ = 0.125f;
	float fxaaMinimumEdgeThreshold_ = 0.0312f;
	float fxaaBlendStrength_ = 0.75f;

	bool sharpenEnabled_ = false;
	float sharpenIntensity_ = 0.5f;
	float sharpenThreshold_ = 0.02f;

	float exposureEV_ = 0.0f;
};
