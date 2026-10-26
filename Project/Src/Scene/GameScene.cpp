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
	constexpr Vector4 kSphereTargetColor{ 1.0f, 0.5f, 0.0f, 1.0f };
	constexpr Vector4 kDisabledColor{ 0.25f, 0.25f, 0.3f, 1.0f };

	namespace Tags {

		inline constexpr char SphereMover[] = "SphereMover";
		inline constexpr char SphereTarget[] = "SphereTarget";
		inline constexpr char CapsuleBoxMover[] = "CapsuleBoxMover";
		inline constexpr char CapsuleBoxTarget[] = "CapsuleBoxTarget";
		inline constexpr char CapsuleMover[] = "CapsuleMover";
		inline constexpr char CapsuleTarget[] = "CapsuleTarget";
		inline constexpr char RayTarget[] = "RayTarget";

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

	Vector4 GetEventColor(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle,
		const Vector4& idleColor) {
		const Collision::ColliderEvents events = world.GetEvents(handle);
		if (!events.enter.empty()) {
			return kEnterColor;
		}
		if (!events.stay.empty()) {
			return kStayColor;
		}
		if (!events.exit.empty()) {
			return kExitColor;
		}
		return idleColor;
	}

	void DrawCollider(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle,
		const Vector4& idleColor) {
		const std::optional<Collision::ColliderState> collider =
			world.GetCollider(handle);
		if (!collider) {
			return;
		}

		const Vector4 color = GetEventColor(world, handle, idleColor);
		std::visit([&](const auto& colliderShape) {
			DrawShape(colliderShape, color);
		}, collider->shape);
	}

	void DrawContacts(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle) {
		const Collision::ColliderEvents events = world.GetEvents(handle);
		for (const std::span<const Collision::ColliderCollision> collisions : {
			events.enter, events.stay }) {
			for (const Collision::ColliderCollision& collision : collisions) {
				Sphere3D(0.12f).Draw(
					Math::MakeTranslateMatrix(collision.contact.point),
					Color::Yellow);
				Line3D(
					collision.contact.point,
					collision.contact.point + collision.contact.normal * 1.25f).Draw(
						Math::Identity4x4(),
						Color::Cyan);
			}
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
	world_.RegisterTags({
		Tags::SphereMover,
		Tags::SphereTarget,
		Tags::CapsuleBoxMover,
		Tags::CapsuleBoxTarget,
		Tags::CapsuleMover,
		Tags::CapsuleTarget,
		Tags::RayTarget,
	});

	movableSphere_ = world_.Add(movableSphereBase_, {
		.tag = Tags::SphereMover,
		.targetTags = { Tags::SphereTarget },
	});
	sphereTarget_ = world_.Add(sphereTargetShape_, {
		.tag = Tags::SphereTarget,
	});
	movableCapsuleBox_ = world_.Add(movableCapsuleBoxBase_, {
		.tag = Tags::CapsuleBoxMover,
		.targetTags = { Tags::CapsuleBoxTarget },
	});
	capsuleBoxTarget_ = world_.Add(capsuleBoxTargetShape_, {
		.tag = Tags::CapsuleBoxTarget,
	});
	movableCapsule_ = world_.Add(movableCapsuleBase_, {
		.tag = Tags::CapsuleMover,
		.targetTags = { Tags::CapsuleTarget },
	});
	capsuleTarget_ = world_.Add(capsuleTargetShape_, {
		.tag = Tags::CapsuleTarget,
	});
	rayTarget_ = world_.Add(rayTargetShape_, {
		.tag = Tags::RayTarget,
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
		sphereTargetEnabled_ = !sphereTargetEnabled_;
		world_.SetEnabled(sphereTarget_, sphereTargetEnabled_);
	}

	world_.SetShape(movableSphere_, Translate(movableSphereBase_, movementOffset_));
	world_.SetShape(
		movableCapsuleBox_,
		Translate(movableCapsuleBoxBase_, movementOffset_));
	world_.SetShape(
		movableCapsule_,
		Translate(movableCapsuleBase_, movementOffset_));
	world_.Step();

	for (const Collision::ColliderHandle handle : {
		movableSphere_, movableCapsuleBox_, movableCapsule_ }) {
		GetData().collisionCount += static_cast<int32_t>(
			world_.GetEvents(handle).enter.size());
	}

	ray_ = rayBase_;
	ray_.origin += movementOffset_;
	rayHit_ = world_.RaycastClosest(ray_, kRayLength, { Tags::RayTarget });
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
		sphereTargetEnabled_ ? kSphereTargetColor : kDisabledColor);
	DrawCollider(world_, movableCapsuleBox_, kIdleColor);
	DrawCollider(world_, capsuleBoxTarget_, kStaticColor);
	DrawCollider(world_, movableCapsule_, kIdleColor);
	DrawCollider(world_, capsuleTarget_, kStaticColor);
	DrawCollider(world_, rayTarget_, rayHit_ ? kStayColor : kStaticColor);
	DrawContacts(world_, movableSphere_);
	DrawContacts(world_, movableCapsuleBox_);
	DrawContacts(world_, movableCapsule_);
	DrawRay(ray_, rayHit_);
}
