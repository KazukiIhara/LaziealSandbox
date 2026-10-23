#include "Scene/OBBCollisionScene.h"

#include <optional>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kMoveSpeed = 3.0f;
	constexpr float kRotationSpeed = 1.4f;
	constexpr float kRayLength = 20.0f;

	void DrawOBB(const Collision::OBB& box, const Vector4& color) {
		Box3D(
			box.halfExtents.x * 2.0f,
			box.halfExtents.y * 2.0f,
			box.halfExtents.z * 2.0f).Draw(
			Math::MakeAffineMatrix(
				{ 1.0f, 1.0f, 1.0f },
				box.rotation,
				box.center),
			color);
	}

	void DrawLine(
		const Vector3& start,
		const Vector3& end,
		const Vector4& color) {
		Line3D(start, end).Draw(Math::Identity4x4(), color);
	}

	void DrawContact(const Collision::Contact& contact, const Vector4& color) {
		Sphere3D(0.09f).Draw(
			Math::MakeTranslateMatrix(contact.point),
			color);
		DrawLine(
			contact.point,
			contact.point + contact.normal,
			color);
	}

}

OBBCollisionScene::OBBCollisionScene(const InitData& init) :
	SampleScene(init) {
	ResetDemo();
}

void OBBCollisionScene::Update() {
	const float deltaTime = static_cast<float>(System::DeltaTime());
	Vector3 movement{
		static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
		static_cast<float>(Key::R.Press()) - static_cast<float>(Key::F.Press()),
		static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
	};
	if (Math::LengthSq(movement) > 1.0f) {
		movement = Math::Normalize(movement);
	}
	movingBox_.center += movement * kMoveSpeed * deltaTime;

	const Vector3 rotationInput{
		static_cast<float>(Key::I.Press()) - static_cast<float>(Key::K.Press()),
		static_cast<float>(Key::L.Press()) - static_cast<float>(Key::J.Press()),
		static_cast<float>(Key::O.Press()) - static_cast<float>(Key::U.Press()),
	};
	movingRotation_ += rotationInput * kRotationSpeed * deltaTime;
	movingBox_.rotation = Math::MakeQuaternion(movingRotation_);

	if (Key::Space.Trigger()) {
		ResetDemo();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	UpdateCamera();
}

void OBBCollisionScene::Draw() const {
	DrawStage();

	const std::optional<Collision::Contact> boxContact =
		Collision::Collide(fixedBox_, movingBox_);
	const std::optional<Collision::Contact> sphereContact =
		Collision::Collide(testSphere_, movingBox_);
	const std::optional<Collision::RaycastHit> rayHit =
		Collision::Raycast(testRay_, movingBox_, kRayLength);
	const Vector3 closestPoint = Collision::ClosestPoint(
		testSphere_.center,
		movingBox_);
	const bool containsSphereCenter = Collision::Contains(
		movingBox_,
		testSphere_.center);

	DrawOBB(
		fixedBox_,
		boxContact ? Color::OrangeRed : Color::RoyalBlue);
	DrawOBB(
		movingBox_,
		boxContact || sphereContact ? Color::Red : Color::Turquoise);
	Sphere3D(testSphere_.radius).Draw(
		Math::MakeTranslateMatrix(testSphere_.center),
		sphereContact ? Color::OrangeRed : Color::Gold);

	// 球の中心からOBB上の最近接点までを緑色の線で表示する。
	DrawLine(testSphere_.center, closestPoint, Color::Lime);
	Sphere3D(0.07f).Draw(
		Math::MakeTranslateMatrix(closestPoint),
		Color::Lime);

	const Vector3 rayDirection = Math::Normalize(testRay_.direction);
	const Vector3 rayEnd = rayHit
		? rayHit->point
		: testRay_.origin + rayDirection * kRayLength;
	DrawLine(testRay_.origin, rayEnd, rayHit ? Color::Cyan : Color::Gray);
	if (rayHit) {
		Sphere3D(0.09f).Draw(
			Math::MakeTranslateMatrix(rayHit->point),
			Color::Cyan);
		DrawLine(
			rayHit->point,
			rayHit->point + rayHit->normal,
			Color::Cyan);
	}

	if (boxContact) {
		DrawContact(*boxContact, Color::Yellow);
	}
	if (sphereContact) {
		DrawContact(*sphereContact, Color::Magenta);
	}

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("OBB Collision Demo")) {
		ImGui::Text(
			"OBB vs OBB: %s",
			boxContact ? "Colliding" : "Separated");
		ImGui::Text(
			"Sphere vs OBB: %s",
			sphereContact ? "Colliding" : "Separated");
		ImGui::Text(
			"Sphere center in OBB: %s",
			containsSphereCenter ? "Yes" : "No");
		ImGui::Text(
			"Closest-point distance: %.3f",
			Math::Length(closestPoint - testSphere_.center));
		ImGui::Text(
			"Raycast: %s",
			rayHit ? "Hit" : "Miss");
		if (boxContact) {
			ImGui::Text("OBB penetration: %.3f", boxContact->penetrationDepth);
		}
		if (sphereContact) {
			ImGui::Text("Sphere penetration: %.3f", sphereContact->penetrationDepth);
		}
		if (rayHit) {
			ImGui::Text("Ray distance: %.3f", rayHit->distance);
		}

		ImGui::Separator();
		ImGui::TextUnformatted("WASD: move on XZ plane | R/F: move vertically");
		ImGui::TextUnformatted("I/K: pitch | J/L: yaw | U/O: roll");
		ImGui::TextUnformatted("Space: reset | Escape: title");
		ImGui::Separator();
		ImGui::TextColored(
			ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
			"Yellow: OBB contact point and normal");
		ImGui::TextColored(
			ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
			"Magenta: Sphere contact point and normal");
		ImGui::TextColored(
			ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
			"Green: closest point on the moving OBB");
		ImGui::TextColored(
			ImVec4(0.0f, 1.0f, 1.0f, 1.0f),
			"Cyan: ray and hit normal");
	}
	ImGui::End();
}

void OBBCollisionScene::ResetDemo() {
	fixedBox_ = {
		.center = { -1.2f, 1.3f, 0.0f },
		.halfExtents = { 1.8f, 0.65f, 1.15f },
		.rotation = Math::MakeQuaternion({ 0.2f, 0.45f, 0.15f }),
	};
	movingRotation_ = { -0.15f, -0.35f, 0.2f };
	movingBox_ = {
		.center = { 2.1f, 1.45f, 0.25f },
		.halfExtents = { 1.1f, 0.8f, 0.7f },
		.rotation = Math::MakeQuaternion(movingRotation_),
	};
	testSphere_ = {
		.center = { 4.2f, 1.35f, 1.4f },
		.radius = 0.75f,
	};
	testRay_ = {
		.origin = { -6.0f, 3.0f, -5.0f },
		.direction = movingBox_.center - Vector3{ -6.0f, 3.0f, -5.0f },
	};
}
