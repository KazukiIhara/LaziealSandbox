#include <LGF/LGF.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <variant>

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

	Collision::Sphere Translate(const Collision::Sphere& sphere, const Vector3& offset) {
		Collision::Sphere result = sphere;
		result.center += offset;
		return result;
	}

	Collision::Capsule Translate(const Collision::Capsule& capsule, const Vector3& offset) {
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

RuntimeConfig ConfigureRuntime() {
	RuntimeConfig config{};
	config.assetRoot =
		"../Dependencies/LaziealRuntime/Dependencies/LaziealGraphicsFramework/Project/Assets";
	config.window.title =
		L"ColliderWorld | Enter: Yellow Stay: Red Exit: Magenta Trigger: Orange | Move: WASD+R/F Reset: Space Toggle Trigger: T | Camera: Arrows+Q/E";
	config.window.width = 1280;
	config.window.height = 720;
	return config;
}

void Main() {
	Camera3D::Param cameraParam{};
	cameraParam.farClip = 100.0f;
	DebugCamera3D camera(25.0f, cameraParam);
	camera.pitch = 22.0f * Math::DEG_TO_RAD;

	DirectionalLight light({ 0.5f, -1.0f, 0.35f }, Color::White, 1.5f);
	const Ground3D ground(30.0f);
	const Grid3D grid(30.0f, 1.0f);

	const Collision::AABB sphereTargetShape{
		.min = { -8.2f, 0.0f, -1.0f },
		.max = { -5.8f, 3.0f, 1.0f },
	};
	const Collision::Sphere movableSphereBase{
		.center = { -7.0f, 1.2f, -3.0f },
		.radius = 0.8f,
	};
	const Collision::AABB capsuleBoxTargetShape{
		.min = { -3.3f, 0.0f, -1.0f },
		.max = { -0.7f, 2.5f, 1.0f },
	};
	const Collision::Capsule movableCapsuleBoxBase{
		.start = { -2.0f, 0.65f, -3.0f },
		.end = { -2.0f, 1.85f, -3.0f },
		.radius = 0.55f,
	};
	const Collision::Capsule capsuleTargetShape{
		.start = { 3.0f, 0.6f, 0.0f },
		.end = { 3.0f, 2.0f, 0.0f },
		.radius = 0.6f,
	};
	const Collision::Capsule movableCapsuleBase{
		.start = { 3.0f, 0.6f, -3.0f },
		.end = { 3.0f, 2.0f, -3.0f },
		.radius = 0.6f,
	};
	const Collision::Capsule rayTargetShape{
		.start = { 8.0f, 0.6f, 0.0f },
		.end = { 8.0f, 2.0f, 0.0f },
		.radius = 0.6f,
	};
	const Collision::Ray rayBase{
		.origin = { 8.0f, 1.3f, -4.0f },
		.direction = { 0.0f, 0.0f, 1.0f },
	};

	Collision::ColliderWorld world;
	const Collision::ColliderHandle movableSphere = world.Add(movableSphereBase, {
		.layer = Layers::SphereMover,
		.mask = Layers::SphereTarget,
		.userData = 1u,
	});
	const Collision::ColliderHandle sphereTarget = world.Add(sphereTargetShape, {
		.layer = Layers::SphereTarget,
		.mask = Layers::SphereMover,
		.isTrigger = true,
		.userData = 2u,
	});
	const Collision::ColliderHandle movableCapsuleBox = world.Add(movableCapsuleBoxBase, {
		.layer = Layers::CapsuleBoxMover,
		.mask = Layers::CapsuleBoxTarget,
		.userData = 3u,
	});
	const Collision::ColliderHandle capsuleBoxTarget = world.Add(capsuleBoxTargetShape, {
		.layer = Layers::CapsuleBoxTarget,
		.mask = Layers::CapsuleBoxMover,
		.userData = 4u,
	});
	const Collision::ColliderHandle movableCapsule = world.Add(movableCapsuleBase, {
		.layer = Layers::CapsuleMover,
		.mask = Layers::CapsuleTarget,
		.userData = 5u,
	});
	const Collision::ColliderHandle capsuleTarget = world.Add(capsuleTargetShape, {
		.layer = Layers::CapsuleTarget,
		.mask = Layers::CapsuleMover,
		.userData = 6u,
	});
	const Collision::ColliderHandle rayTarget = world.Add(rayTargetShape, {
		.layer = Layers::RayTarget,
		.mask = 0u,
		.userData = 7u,
	});

	Vector3 movementOffset{};
	bool triggerEnabled = true;
	while (System::Update()) {
		const float deltaTime = static_cast<float>(System::DeltaTime());
		Vector3 movement{
			static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
			static_cast<float>(Key::R.Press()) - static_cast<float>(Key::F.Press()),
			static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
		};
		if (Math::LengthSq(movement) > Math::EPSILON) {
			movementOffset += Math::Normalize(movement) * kMoveSpeed * deltaTime;
		}
		if (Key::Space.Trigger()) {
			movementOffset = {};
		}
		if (Key::T.Trigger()) {
			triggerEnabled = !triggerEnabled;
			world.SetEnabled(sphereTarget, triggerEnabled);
		}

		world.SetShape(movableSphere, Translate(movableSphereBase, movementOffset));
		world.SetShape(
			movableCapsuleBox,
			Translate(movableCapsuleBoxBase, movementOffset));
		world.SetShape(movableCapsule, Translate(movableCapsuleBase, movementOffset));
		world.Step();

		Collision::Ray ray = rayBase;
		ray.origin += movementOffset;
		const std::optional<Collision::WorldRaycastHit> rayHit =
			world.RaycastClosest(ray, kRayLength, Layers::RayTarget);

		camera.Update();
		camera.SetScene();
		light.SetScene();
		ground.Draw(
			Math::MakeTranslateMatrix({ 0.0f, -0.02f, 0.0f }),
			Color::DarkGray);
		grid.Draw(Math::MakeTranslateMatrix({ 0.0f, 0.01f, 0.0f }));

		DrawCollider(world, movableSphere, kIdleColor);
		DrawCollider(
			world,
			sphereTarget,
			triggerEnabled ? kTriggerColor : kDisabledColor);
		DrawCollider(world, movableCapsuleBox, kIdleColor);
		DrawCollider(world, capsuleBoxTarget, kStaticColor);
		DrawCollider(world, movableCapsule, kIdleColor);
		DrawCollider(world, capsuleTarget, kStaticColor);
		DrawCollider(world, rayTarget, rayHit ? kStayColor : kStaticColor);
		DrawContacts(world);
		DrawRay(ray, rayHit);
	}
}
