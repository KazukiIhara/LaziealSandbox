#include "Scene/TitleScene.h"

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kBoxMoveSpeed = 3.0f;
	constexpr float kStickDeadZone = 0.15f;

	Vector2 ApplyStickDeadZone(Vector2 input) {
		const float lengthSq = Math::LengthSq(input);
		if (lengthSq < kStickDeadZone * kStickDeadZone) {
			return {};
		}
		if (lengthSq > 1.0f) {
			return Math::Normalize(input);
		}
		return input;
	}

}

TitleScene::TitleScene(const InitData& init)
	: SampleScene(init),
	spherePos_(Parameter::GetOr<Vector3>("Sphere.json", "Pos", spherePos_)) {
}

void TitleScene::Update() {
	const float deltaTime = static_cast<float>(System::DeltaTime());
	rotation_ += deltaTime;
	Vector2 cameraRotation{};

	if (const std::optional<Gamepad> leftJoyCon = JoyCon::Left()) {
		Vector2 dPad{
			static_cast<float>(leftJoyCon->Button(GamepadButtonCode::DPadRight).Press()) -
				static_cast<float>(leftJoyCon->Button(GamepadButtonCode::DPadLeft).Press()),
			static_cast<float>(leftJoyCon->Button(GamepadButtonCode::DPadUp).Press()) -
				static_cast<float>(leftJoyCon->Button(GamepadButtonCode::DPadDown).Press()),
		};
		if (Math::LengthSq(dPad) > 1.0f) {
			dPad = Math::Normalize(dPad);
		}
		boxPos_.x += dPad.x * kBoxMoveSpeed * deltaTime;
		boxPos_.y += dPad.y * kBoxMoveSpeed * deltaTime;

		const Vector2 leftStick = ApplyStickDeadZone(leftJoyCon->LeftStick());
		boxPos_.x += leftStick.x * kBoxMoveSpeed * deltaTime;
		boxPos_.z += leftStick.y * kBoxMoveSpeed * deltaTime;
	}

	if (const std::optional<Gamepad> rightJoyCon = JoyCon::Right()) {
		cameraRotation = ApplyStickDeadZone(rightJoyCon->RightStick());
	}

	UpdateCamera(cameraRotation);

	if (Key::G.Trigger()) {
		ChangeScene(SceneNames::GamepadTest);
	} else if (Key::P.Trigger()) {
		ChangeScene(SceneNames::PointLight);
	} else if (Key::Enter.Trigger()) {
		ChangeScene(SceneNames::Game);
	}
}

void TitleScene::Draw() const {
	DrawStage();
	const Quaternion rotation = Math::MakeRotateAxisAngle(
		{ 0.0f, 1.0f, 0.0f },
		rotation_);
	box_.Draw(
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f }, rotation, boxPos_),
		Color::RoyalBlue);
	capsule_.Draw(
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f }, rotation, { 0.0f, 1.25f, 0.0f }),
		Color::Gold);
	sphere_.Draw(
		Math::MakeTranslateMatrix(spherePos_),
		Color::Turquoise);
}
