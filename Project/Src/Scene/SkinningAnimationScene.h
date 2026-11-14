#pragma once

#include <LGF/LGF.h>

#include <string>
#include <vector>

class SkinningAnimationScene {
public:
	SkinningAnimationScene();
	~SkinningAnimationScene();

	void Update();
	void Draw();

private:
	LGF::Model foxModel_;
	LGF::ModelAnimator walkAnimator_;
	LGF::ModelAnimator surveyAnimator_;
	LGF::ModelAnimator runAnimator_;
	LGF::DebugCamera3D camera_;
	LGF::DirectionalLight directionalLight_;
	LGF::Skybox skybox_;
	LGF::Ground3D ground_;
	LGF::Font2D titleFont_;
	LGF::Font2D captionFont_;
	LGF::RenderTargetTexture renderTarget_;
	LGF::Sprite renderTargetPreview_;
	LGF::Camera3D renderPassCamera_;
	std::vector<LGF::ModelInstance3D> walkInstances_;
	std::string animationNames_;
};
