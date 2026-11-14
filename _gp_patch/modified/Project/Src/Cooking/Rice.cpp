#include "Rice.h"

using namespace LGF;

namespace {

	constexpr float kRiceRadius = 0.07f;

}

Rice::Rice(
	Physics::PhysicsWorld& physicsWorld,
	const Vector3& position,
	ObjectHandle objectHandle,
	Physics::PhysicsParticleRenderGroupHandle renderGroup) :
	physicsWorld_(&physicsWorld) {
	physicsHandle_ = physicsWorld_->AddParticle({
		.position = position,
		.radius = kRiceRadius,
		.mass = 0.02f,
		.material = { .restitution = 0.08f, .friction = 0.65f },
		.objectHandle = objectHandle,
		.renderGroup = renderGroup,
	});
}

Rice::~Rice() {
	if (physicsWorld_ != nullptr) {
		physicsWorld_->RemoveParticle(physicsHandle_);
	}
}
