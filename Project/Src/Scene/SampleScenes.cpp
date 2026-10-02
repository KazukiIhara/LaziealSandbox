#include "Scene/SampleScenes.h"

#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/SceneNames.h"
#include "Scene/TitleScene.h"

void RegisterSampleScenes(SampleApp& sceneManager) {
	sceneManager
		.Register<TitleScene>(SceneNames::Title)
		.Register<GameScene>(SceneNames::Game)
		.Register<ResultScene>(SceneNames::Result);
}
