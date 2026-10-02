#pragma once

#include "Scene/SampleScenes.h"

class SampleScene : public SampleApp::Scene {
public:
	explicit SampleScene(const InitData& init);

protected:
	void UpdateCamera();
	void DrawStage() const;

private:
	LGF::DebugCamera3D camera_;
	LGF::DirectionalLight light_;
	LGF::Ground3D ground_;
	LGF::Grid3D grid_;
};
