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
		LGF::ObjectHandle objectHandle,
		LGF::Physics::PhysicsParticleRenderGroupHandle renderGroup);

	~Rice();

	Rice(const Rice&) = delete;
	Rice& operator=(const Rice&) = delete;

private:
	LGF::Physics::PhysicsWorld* physicsWorld_ = nullptr;
	LGF::Physics::PhysicsParticleHandle physicsHandle_{};
};
