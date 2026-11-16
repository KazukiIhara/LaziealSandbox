#include <LGF/LGF.h>

#include "Scene/PostProcessScene.h"

using namespace LGF;

void Main() {
	PostProcessScene scene;

	while (System::Update()) {
		scene.Update();
		scene.Draw();
	}
}
