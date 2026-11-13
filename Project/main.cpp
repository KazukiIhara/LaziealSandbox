#include <LGF/LGF.h>

#include "Scene/SkinningAnimationScene.h"

using namespace LGF;

void Main() {
	SkinningAnimationScene scene;

	while (System::Update()) {
		scene.Update();
		scene.Draw();
	}
}
