#pragma once

#include <LGF/LGF.h>

#include <vector>

/// <summary>
/// Lazieal Frameworkの主要機能を一つの画面で確認するショーケース。
/// 各機能の有効状態や代表的なパラメータはImGuiから変更できます。
/// </summary>
class ShowcaseScene {
public:
	ShowcaseScene();
	~ShowcaseScene();

	void Update();
	void Draw();

private:
	void InitializeInput();
	void InitializeUI();
	void InitializeCollision();
	void InitializePhysics();
	void InitializePostProcess();

	void UpdateUI();
	void UpdateCollision();
	void UpdatePhysics(float deltaTime);
	void UpdateVFX(float deltaTime);

	void DrawWorld();
	void DrawRenderTarget();
	void DrawPhysics() const;
	void DrawCollision() const;
	void DrawOverlay2D();
	void DrawControlPanel();
	void DrawFeatureList() const;

	void SpawnPhysicsBall();
	void ResetPhysicsBalls();
	void SpawnPhysicsParticles();
	void PlayEffect();
	void ApplyPostProcessSettings();

private:
	// 3Dシーンの基本要素。DebugCamera3Dは矢印キーとQ/Eで操作できます。
	LGF::DebugCamera3D camera_;
	LGF::Camera3D renderTargetCamera_;
	LGF::DirectionalLight directionalLight_;
	LGF::PointLight pointLight_;
	LGF::ToonLighting toonLighting_;
	LGF::Skybox skybox_;
	LGF::Ground3D ground_;
	LGF::Grid3D grid_;
	LGF::Sphere3D sphere_;
	LGF::Box3D box_;
	LGF::Capsule3D capsule_;
	LGF::Line3D collisionLink_;

	// glTFモデルとスキニングアニメーション。
	LGF::Model foxModel_;
	LGF::ModelAnimator foxAnimator_;
	std::vector<LGF::ModelInstance3D> foxInstances_;

	// 画面外へ3D描画してから、Spriteとして画面へ合成します。
	LGF::RenderTargetTexture renderTarget_;
	LGF::Sprite renderTargetPreview_;

	// 2D / 3Dフォント、画像、ボタンUI。
	LGF::Font2D titleFont_;
	LGF::Font2D uiFont_;
	LGF::Font3D worldFont_;
	LGF::Texture checkerTexture_;
	LGF::Sprite checkerSprite_;
	LGF::UIWorld uiWorld_;
	LGF::UIPanel3D uiPanel3D_;
	LGF::UIButtonHandle spawnButton_{};
	LGF::UIButtonHandle effectButton_{};
	LGF::UIButtonHandle worldButton_{};

	// CPUパーティクルとEffekseerを同じシーンで管理します。
	LGF::VFX::ParticleWorld particleWorld_;
	LGF::VFX::ParticleEmitterHandle particleEmitter_{};
	LGF::VFX::GpuParticleWorld gpuParticleWorld_;
	LGF::VFX::ParticleEmitterHandle gpuParticleEmitter_{};
	LGF::VFX::EffectWorld effectWorld_;
	LGF::VFX::Effect laserEffect_;
	LGF::VFX::EffectHandle effectHandle_{};

	// 軽量な当たり判定と、剛体シミュレーションは別ワールドです。
	LGF::Collision::ColliderWorld colliderWorld_;
	LGF::Collision::ColliderHandle fixedCollider_{};
	LGF::Collision::ColliderHandle movingCollider_{};
	LGF::Physics::PhysicsWorld physicsWorld_;
	LGF::Physics::StaticObjectHandle physicsGround_{};
	LGF::Physics::KinematicObjectHandle kinematicPlatform_{};
	LGF::Physics::PhysicsParticleRenderGroupHandle physicsParticleRenderGroup_{};
	std::vector<LGF::Physics::DynamicObjectHandle> physicsBalls_;

	LGF::PostProcessProfileHandle postProcessProfile_{};

	float elapsedTime_ = 0.0f;
	float animationSpeed_ = 1.0f;
	float pointLightIntensity_ = 24.0f;
	float particleEmissionRate_ = 120.0f;
	float exposureEV_ = 0.0f;
	float bloomIntensity_ = 0.18f;
	float vignetteIntensity_ = 0.2f;
	float grayscaleIntensity_ = 1.0f;
	float chromaticAberrationIntensity_ = 2.0f;
	float pixelSize_ = 8.0f;
	float sharpenIntensity_ = 0.5f;
	float sharpenThreshold_ = 0.02f;
	float physicsParticleRadius_ = 0.12f;
	float kinematicPlatformHeight_ = 1.0f;
	int physicsSpawnCount_ = 8;
	int physicsParticleSpawnCount_ = 3000;

	bool showModel_ = true;
	bool showMeshletColors_ = false;
	bool showMeshletBounds_ = false;
	bool showPrimitives_ = true;
	bool showGrid_ = true;
	bool show2DOverlay_ = true;
	bool showUIHitboxes_ = false;
	bool showCameraDebug_ = false;
	bool particlesEnabled_ = true;
	bool gpuParticlesEnabled_ = true;
	bool physicsEnabled_ = true;
	bool bloomEnabled_ = true;
	bool vignetteEnabled_ = false;
	bool grayscaleEnabled_ = false;
	bool chromaticAberrationEnabled_ = false;
	bool pixelationEnabled_ = false;
	bool sharpenEnabled_ = false;
	bool collisionDetected_ = false;
};
