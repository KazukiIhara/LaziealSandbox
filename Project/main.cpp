#include <LGF/LGF.h>

#include "Scene/ToonLightingScene.h"

using namespace LGF;

void Main() {
	ToonLightingScene scene;

	while (System::Update()) {
		scene.Update();
		scene.Draw();
	}
}
