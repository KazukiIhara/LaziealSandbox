#pragma once

#include "LGF/Physics.h"

// =========================================================
// 米の物理パーティクル
// =========================================================
class Rice
{
public:
	Rice(
		LGF::Physics::PhysicsWorld& physicsWorld,
		const LGF::Vector3& position,
		LGF::ObjectHandle objectHandle);

	~Rice();

	Rice(const Rice&) = delete;
	Rice& operator=(const Rice&) = delete;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() const;

private:
	LGF::Physics::PhysicsWorld* physicsWorld_ = nullptr;
	LGF::Physics::PhysicsParticleHandle physicsHandle_{};
};
