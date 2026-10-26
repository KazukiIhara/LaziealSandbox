#include "Scene/SampleScenes.h"

#include "Scene/ColliderWorldScene.h"
#include "Scene/GameScene.h"
#include "Scene/GamepadTestScene.h"
#include "Scene/OBBCollisionScene.h"
#include "Scene/ParticleWorldScene.h"
#include "Scene/PhysicsWorldScene.h"
#include "Scene/PointLightScene.h"
#include "Scene/ResultScene.h"
#include "Scene/SceneNames.h"
#include "Scene/SphericalShellCapScene.h"
#include "Scene/TitleScene.h"

void RegisterSampleScenes(SampleApp& sceneManager) {
	sceneManager
		.Register<PhysicsWorldScene>(SceneNames::PhysicsWorld)
		.Register<ColliderWorldScene>(SceneNames::ColliderWorld)
		.Register<OBBCollisionScene>(SceneNames::OBBCollision)
		.Register<SphericalShellCapScene>(SceneNames::SphericalShellCap)
		.Register<ParticleWorldScene>(SceneNames::ParticleWorld)
		.Register<GamepadTestScene>(SceneNames::GamepadTest)
		.Register<PointLightScene>(SceneNames::PointLight)
		.Register<TitleScene>(SceneNames::Title)
		.Register<GameScene>(SceneNames::Game)
		.Register<ResultScene>(SceneNames::Result);
}
