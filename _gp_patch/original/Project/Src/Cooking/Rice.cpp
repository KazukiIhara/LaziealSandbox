#include "Rice.h"

#include "LGF/Graphics/Sphere3D.h"
#include "LGF/LGFMath.h"

using namespace LGF;

namespace {

	constexpr float kRiceRadius = 0.07f;

}

Rice::Rice(
	Physics::PhysicsWorld& physicsWorld,
	const Vector3& position,
	ObjectHandle objectHandle) :
	physicsWorld_(&physicsWorld) {
	physicsHandle_ = physicsWorld_->AddParticle({
		.position = position,
		.radius = kRiceRadius,
		.mass = 0.02f,
		.material = { .restitution = 0.08f, .friction = 0.65f },
		.objectHandle = objectHandle,
	});
}

Rice::~Rice() {
	if (physicsWorld_ != nullptr) {
		physicsWorld_->RemoveParticle(physicsHandle_);
	}
}

void Rice::Draw() const {
	if (physicsWorld_ == nullptr) {
		return;
	}

	const auto state = physicsWorld_->GetParticleState(physicsHandle_);
	if (!state.has_value()) {
		return;
	}

	Sphere3D(state->radius).Draw(
		Math::MakeTranslateMatrix(state->position),
		Vector4{ 0.95f, 0.92f, 0.78f, 1.0f });
}
