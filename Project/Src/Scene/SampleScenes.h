#pragma once

#include <LGF/LGF.h>

#include <string>

struct SceneData {
	int collisionCount = 0;
};

using SampleApp = LGF::SceneManager<std::string, SceneData>;

void RegisterSampleScenes(SampleApp& sceneManager);
