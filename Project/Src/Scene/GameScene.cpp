#include "Scene/GameScene.h"

#include <algorithm>
#include <cmath>
#include <variant>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kMoveSpeed = 3.0f;
	constexpr float kRayLength = 8.0f;
	constexpr Vector4 kIdleColor{ 0.15f, 0.8f, 0.25f, 1.0f };
	constexpr Vector4 kEnterColor{ 1.0f, 0.85f, 0.0f, 1.0f };
	constexpr Vector4 kStayColor{ 1.0f, 0.15f, 0.1f, 1.0f };
	constexpr Vector4 kExitColor{ 1.0f, 0.0f, 0.75f, 1.0f };
	constexpr Vector4 kStaticColor{ 0.55f, 0.65f, 0.75f, 1.0f };
	constexpr Vector4 kTriggerColor{ 1.0f, 0.5f, 0.0f, 1.0f };
	constexpr Vector4 kDisabledColor{ 0.25f, 0.25f, 0.3f, 1.0f };

	namespace Layers {

		inline constexpr Collision::CollisionLayerMask SphereMover = 1u << 0u;
		inline constexpr Collision::CollisionLayerMask SphereTarget = 1u << 1u;
		inline constexpr Collision::CollisionLayerMask CapsuleBoxMover = 1u << 2u;
		inline constexpr Collision::CollisionLayerMask CapsuleBoxTarget = 1u << 3u;
		inline constexpr Collision::CollisionLayerMask CapsuleMover = 1u << 4u;
		inline constexpr Collision::CollisionLayerMask CapsuleTarget = 1u << 5u;
		inline constexpr Collision::CollisionLayerMask RayTarget = 1u << 6u;

	}

	Collision::Sphere Translate(
		const Collision::Sphere& sphere,
		const Vector3& offset) {
		Collision::Sphere result = sphere;
		result.center += offset;
		return result;
	}

	Collision::Capsule Translate(
		const Collision::Capsule& capsule,
		const Vector3& offset) {
		Collision::Capsule result = capsule;
		result.start += offset;
		result.end += offset;
		return result;
	}

	Quaternion RotationFromUp(const Vector3& direction) {
		const Vector3 up{ 0.0f, 1.0f, 0.0f };
		const Vector3 normalized = Math::Normalize(direction);
		const float cosine = std::clamp(Math::Dot(up, normalized), -1.0f, 1.0f);
		if (cosine >= 1.0f - Math::EPSILON) {
			return {};
		}
		if (cosine <= -1.0f + Math::EPSILON) {
			return Math::MakeRotateAxisAngle({ 1.0f, 0.0f, 0.0f }, Math::PI);
		}

		return Math::MakeRotateAxisAngle(
			Math::Normalize(Math::Cross(up, normalized)),
			std::acos(cosine));
	}

	void DrawShape(const Collision::Sphere& sphere, const Vector4& color) {
		Sphere3D(sphere.radius).Draw(Math::MakeTranslateMatrix(sphere.center), color);
	}

	void DrawShape(const Collision::AABB& box, const Vector4& color) {
		const Vector3 size = box.max - box.min;
		const Vector3 center = (box.min + box.max) * 0.5f;
		Box3D(size.x, size.y, size.z).Draw(Math::MakeTranslateMatrix(center), color);
	}

	void DrawShape(const Collision::Capsule& capsule, const Vector4& color) {
		const Vector3 axis = capsule.end - capsule.start;
		const float axisLength = Math::Length(axis);
		const Vector3 center = (capsule.start + capsule.end) * 0.5f;
		const Quaternion rotation = axisLength > Math::EPSILON
			? RotationFromUp(axis)
			: Quaternion{};
		Capsule3D(capsule.radius, axisLength + capsule.radius * 2.0f).Draw(
			Math::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotation, center),
			color);
	}

	bool EventContains(
		const Collision::CollisionEvent& event,
		Collision::ColliderHandle handle) {
		return event.first == handle || event.second == handle;
	}

	std::optional<Collision::CollisionPhase> GetPhase(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle) {
		for (const Collision::CollisionEvent& event : world.GetEvents()) {
			if (EventContains(event, handle)) {
				return event.phase;
			}
		}
		return std::nullopt;
	}

	Vector4 GetEventColor(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle,
		const Vector4& idleColor) {
		const std::optional<Collision::CollisionPhase> phase = GetPhase(world, handle);
		if (!phase) {
			return world.IsColliding(handle) ? kStayColor : idleColor;
		}

		switch (*phase) {
		case Collision::CollisionPhase::Enter:
			return kEnterColor;
		case Collision::CollisionPhase::Stay:
			return kStayColor;
		case Collision::CollisionPhase::Exit:
			return kExitColor;
		}
		return idleColor;
	}

	void DrawCollider(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle,
		const Vector4& idleColor) {
		const Collision::ColliderShape* shape = world.GetShape(handle);
		if (!shape) {
			return;
		}

		const Vector4 color = GetEventColor(world, handle, idleColor);
		std::visit([&](const auto& colliderShape) {
			DrawShape(colliderShape, color);
		}, *shape);
	}

	void DrawContacts(const Collision::ColliderWorld& world) {
		for (const Collision::CollisionEvent& event : world.GetEvents()) {
			if (event.phase == Collision::CollisionPhase::Exit) {
				continue;
			}

			Sphere3D(0.12f).Draw(
				Math::MakeTranslateMatrix(event.contact.point),
				event.isTrigger ? Color::Orange : Color::Yellow);
			Line3D(
				event.contact.point,
				event.contact.point + event.contact.normal * 1.25f).Draw(
					Math::Identity4x4(),
					Color::Cyan);
		}
	}

	void DrawRay(
		const Collision::Ray& ray,
		const std::optional<Collision::WorldRaycastHit>& hit) {
		const Vector3 direction = Math::Normalize(ray.direction);
		const Vector3 end = hit
			? hit->hit.point
			: ray.origin + direction * kRayLength;
		Line3D(ray.origin, end).Draw(
			Math::Identity4x4(),
			hit ? kStayColor : Color::Yellow);

		if (hit) {
			Sphere3D(0.12f).Draw(
				Math::MakeTranslateMatrix(hit->hit.point),
				Color::Yellow);
			Line3D(
				hit->hit.point,
				hit->hit.point + hit->hit.normal * 1.25f).Draw(
					Math::Identity4x4(),
					Color::Cyan);
		}
	}

}

GameScene::GameScene(const InitData& init) :
	SampleScene(init) {
	movableSphere_ = world_.Add(movableSphereBase_, {
		.layer = Layers::SphereMover,
		.mask = Layers::SphereTarget,
		.userData = 1u,
	});
	sphereTarget_ = world_.Add(sphereTargetShape_, {
		.layer = Layers::SphereTarget,
		.mask = Layers::SphereMover,
		.isTrigger = true,
		.userData = 2u,
	});
	movableCapsuleBox_ = world_.Add(movableCapsuleBoxBase_, {
		.layer = Layers::CapsuleBoxMover,
		.mask = Layers::CapsuleBoxTarget,
		.userData = 3u,
	});
	capsuleBoxTarget_ = world_.Add(capsuleBoxTargetShape_, {
		.layer = Layers::CapsuleBoxTarget,
		.mask = Layers::CapsuleBoxMover,
		.userData = 4u,
	});
	movableCapsule_ = world_.Add(movableCapsuleBase_, {
		.layer = Layers::CapsuleMover,
		.mask = Layers::CapsuleTarget,
		.userData = 5u,
	});
	capsuleTarget_ = world_.Add(capsuleTargetShape_, {
		.layer = Layers::CapsuleTarget,
		.mask = Layers::CapsuleMover,
		.userData = 6u,
	});
	rayTarget_ = world_.Add(rayTargetShape_, {
		.layer = Layers::RayTarget,
		.mask = 0u,
		.userData = 7u,
	});
}

void GameScene::OnEnter() {
	GetData().collisionCount = 0;
}

void GameScene::Update() {
	const float deltaTime = static_cast<float>(System::DeltaTime());
	Vector3 movement{
		static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
		static_cast<float>(Key::R.Press()) - static_cast<float>(Key::F.Press()),
		static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
	};
	if (Math::LengthSq(movement) > Math::EPSILON) {
		movementOffset_ += Math::Normalize(movement) * kMoveSpeed * deltaTime;
	}
	if (Key::Space.Trigger()) {
		movementOffset_ = {};
	}
	if (Key::T.Trigger()) {
		triggerEnabled_ = !triggerEnabled_;
		world_.SetEnabled(sphereTarget_, triggerEnabled_);
	}

	world_.SetShape(movableSphere_, Translate(movableSphereBase_, movementOffset_));
	world_.SetShape(
		movableCapsuleBox_,
		Translate(movableCapsuleBoxBase_, movementOffset_));
	world_.SetShape(
		movableCapsule_,
		Translate(movableCapsuleBase_, movementOffset_));
	world_.Step();

	for (const Collision::CollisionEvent& event : world_.GetEvents()) {
		if (event.phase == Collision::CollisionPhase::Enter) {
			++GetData().collisionCount;
		}
	}

	ray_ = rayBase_;
	ray_.origin += movementOffset_;
	rayHit_ = world_.RaycastClosest(ray_, kRayLength, Layers::RayTarget);
	UpdateCamera();

	if (Key::Enter.Trigger()) {
		ChangeScene(SceneNames::Result);
	}
}

void GameScene::Draw() const {
	DrawStage();
	DrawCollider(world_, movableSphere_, kIdleColor);
	DrawCollider(
		world_,
		sphereTarget_,
		triggerEnabled_ ? kTriggerColor : kDisabledColor);
	DrawCollider(world_, movableCapsuleBox_, kIdleColor);
	DrawCollider(world_, capsuleBoxTarget_, kStaticColor);
	DrawCollider(world_, movableCapsule_, kIdleColor);
	DrawCollider(world_, capsuleTarget_, kStaticColor);
	DrawCollider(world_, rayTarget_, rayHit_ ? kStayColor : kStaticColor);
	DrawContacts(world_);
	DrawRay(ray_, rayHit_);
}
