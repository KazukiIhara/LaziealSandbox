#pragma once

#include "Scene/SampleScene.h"

#include <cstddef>
#include <cstdint>
#include <vector>

/// <summary>
/// PhysicsWorldの動的・静的・キネマティックオブジェクトと物理粒子を確認するシーン。
/// </summary>
class PhysicsWorldDemoScene final : public SampleScene {
public:
	explicit PhysicsWorldDemoScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	void Reset();
	void SpawnSpheres(std::size_t count);
	void SpawnParticles(std::size_t count);
	void LaunchSpheres();
	void UpdatePlayer();
	void UpdateKinematicPlatform(float deltaTime);
	void DrawDebugWindow();
	std::size_t GetFallingSphereCount() const;

	LGF::Physics::PhysicsWorld physicsWorld_;
	LGF::Physics::DynamicObjectHandle playerHandle_{};
	LGF::Physics::KinematicObjectHandle platformHandle_{};
	LGF::Physics::PhysicsParticleRenderGroupHandle particleRenderGroup_{};
	std::vector<LGF::Physics::StaticObjectState> staticObjectStates_;
	std::vector<LGF::Physics::KinematicObjectState> kinematicObjectStates_;
	std::vector<LGF::Physics::DynamicObjectState> dynamicObjectStates_;
	uint32_t nextObjectIndex_ = 100u;
	float spawnTimer_ = 0.0f;
	float platformTime_ = 0.0f;
	bool autoSpawn_ = true;
	bool isPaused_ = false;
};
