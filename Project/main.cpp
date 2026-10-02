#include <LGF/LGF.h>

#include "Scene/SampleScenes.h"

using namespace LGF;

void Main() {
	SampleApp sceneManager;
	RegisterSampleScenes(sceneManager);

	while (System::Update()) {
		sceneManager.Update();
		sceneManager.Draw();
	}
}
