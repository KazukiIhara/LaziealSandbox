#pragma once

#include <LGF/LGF.h>

class ToonLightingScene {
public:
	ToonLightingScene();
	~ToonLightingScene();

	void Update();
	void Draw();

private:
	void DrawControlPanel();
	void ResetLightingSettings();

private:
	LGF::DebugCamera3D camera_;
	LGF::DirectionalLight directionalLight_;
	LGF::ToonLighting toonLighting_;
	LGF::ToonMaterial3D warmToonMaterial_;
	LGF::ToonMaterial3D coolToonMaterial_;
	LGF::Ground3D ground_;
	LGF::Sphere3D sphere_;
	LGF::Capsule3D capsule_;
	LGF::Cylinder3D cylinder_;
	LGF::Cone3D cone_;
	LGF::Box3D box_;
};
