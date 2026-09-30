#include <LGF/LGF.h>

#include <algorithm>
#include <cmath>
#include <optional>

using namespace LGF;

namespace {

	constexpr float kMoveSpeed = 3.0f;
	constexpr float kRayLength = 8.0f;
	constexpr Vector4 kIdleColor{ 0.15f, 0.8f, 0.25f, 1.0f };
	constexpr Vector4 kHitColor{ 1.0f, 0.15f, 0.1f, 1.0f };
	constexpr Vector4 kStaticColor{ 0.55f, 0.65f, 0.75f, 1.0f };

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

	void Draw(const Collision::Sphere& sphere, const Vector4& color) {
		Sphere3D(sphere.radius).Draw(Math::MakeTranslateMatrix(sphere.center), color);
	}

	void Draw(const Collision::AABB& box, const Vector4& color) {
		const Vector3 size = box.max - box.min;
		const Vector3 center = (box.min + box.max) * 0.5f;
		Box3D(size.x, size.y, size.z).Draw(Math::MakeTranslateMatrix(center), color);
	}

	void Draw(const Collision::Capsule& capsule, const Vector4& color) {
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

	void DrawContact(const std::optional<Collision::Contact>& contact) {
		if (!contact) {
			return;
		}

		Sphere3D(0.12f).Draw(
			Math::MakeTranslateMatrix(contact->point),
			Color::Yellow);
		Line3D(
			contact->point,
			contact->point + contact->normal * 1.25f).Draw(
			Math::Identity4x4(),
			Color::Cyan);
	}

	void DrawRay(
		const Collision::Ray& ray,
		const std::optional<Collision::RaycastHit>& hit) {
		const Vector3 direction = Math::Normalize(ray.direction);
		const Vector3 end = hit
			? hit->point
			: ray.origin + direction * kRayLength;
		Line3D(ray.origin, end).Draw(
			Math::Identity4x4(),
			hit ? kHitColor : Color::Yellow);

		if (hit) {
			Sphere3D(0.12f).Draw(Math::MakeTranslateMatrix(hit->point), Color::Yellow);
			Line3D(hit->point, hit->point + hit->normal * 1.25f).Draw(
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
		L"Collision Debug | L->R: Sphere/Box, Capsule/Box, Capsule/Capsule, Ray/Capsule | Move: WASD + R/F | Reset: Space | Camera: Arrows + Q/E";
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

	const Collision::AABB sphereTarget{
		.min = { -8.2f, 0.0f, -1.0f },
		.max = { -5.8f, 3.0f, 1.0f },
	};
	const Collision::Sphere movableSphereBase{
		.center = { -7.0f, 1.2f, -3.0f },
		.radius = 0.8f,
	};

	const Collision::AABB capsuleBoxTarget{
		.min = { -3.3f, 0.0f, -1.0f },
		.max = { -0.7f, 2.5f, 1.0f },
	};
	const Collision::Capsule movableCapsuleBoxBase{
		.start = { -2.0f, 0.65f, -3.0f },
		.end = { -2.0f, 1.85f, -3.0f },
		.radius = 0.55f,
	};

	const Collision::Capsule capsuleTarget{
		.start = { 3.0f, 0.6f, 0.0f },
		.end = { 3.0f, 2.0f, 0.0f },
		.radius = 0.6f,
	};
	const Collision::Capsule movableCapsuleBase{
		.start = { 3.0f, 0.6f, -3.0f },
		.end = { 3.0f, 2.0f, -3.0f },
		.radius = 0.6f,
	};

	const Collision::Capsule rayTarget{
		.start = { 8.0f, 0.6f, 0.0f },
		.end = { 8.0f, 2.0f, 0.0f },
		.radius = 0.6f,
	};
	const Collision::Ray rayBase{
		.origin = { 8.0f, 1.3f, -4.0f },
		.direction = { 0.0f, 0.0f, 1.0f },
	};

	Vector3 movementOffset{};
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

		const Collision::Sphere movableSphere =
			Translate(movableSphereBase, movementOffset);
		const Collision::Capsule movableCapsuleBox =
			Translate(movableCapsuleBoxBase, movementOffset);
		const Collision::Capsule movableCapsule =
			Translate(movableCapsuleBase, movementOffset);
		Collision::Ray ray = rayBase;
		ray.origin += movementOffset;

		const std::optional<Collision::Contact> sphereContact =
			Collision::Collide(movableSphere, sphereTarget);
		const std::optional<Collision::Contact> capsuleBoxContact =
			Collision::Collide(movableCapsuleBox, capsuleBoxTarget);
		const std::optional<Collision::Contact> capsuleContact =
			Collision::Collide(movableCapsule, capsuleTarget);
		const std::optional<Collision::RaycastHit> rayHit =
			Collision::Raycast(ray, rayTarget, kRayLength);

		camera.Update();
		camera.SetScene();
		light.SetScene();

		ground.Draw(
			Math::MakeTranslateMatrix({ 0.0f, -0.02f, 0.0f }),
			Color::DarkGray);
		grid.Draw(Math::MakeTranslateMatrix({ 0.0f, 0.01f, 0.0f }));

		Draw(movableSphere, sphereContact ? kHitColor : kIdleColor);
		Draw(sphereTarget, sphereContact ? kHitColor : kStaticColor);
		DrawContact(sphereContact);

		Draw(movableCapsuleBox, capsuleBoxContact ? kHitColor : kIdleColor);
		Draw(capsuleBoxTarget, capsuleBoxContact ? kHitColor : kStaticColor);
		DrawContact(capsuleBoxContact);

		Draw(movableCapsule, capsuleContact ? kHitColor : kIdleColor);
		Draw(capsuleTarget, capsuleContact ? kHitColor : kStaticColor);
		DrawContact(capsuleContact);

		Draw(rayTarget, rayHit ? kHitColor : kStaticColor);
		DrawRay(ray, rayHit);
	}
}
