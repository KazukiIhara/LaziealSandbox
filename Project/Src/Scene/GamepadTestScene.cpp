#include "Scene/GamepadTestScene.h"

#include <array>
#include <string>

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	struct ButtonEntry {
		GamepadButtonCode code;
		const char* name;
	};

	constexpr std::array<ButtonEntry, static_cast<std::size_t>(GamepadButtonCode::Count)> kButtons{
		ButtonEntry{ GamepadButtonCode::South, "South" },
		ButtonEntry{ GamepadButtonCode::East, "East" },
		ButtonEntry{ GamepadButtonCode::West, "West" },
		ButtonEntry{ GamepadButtonCode::North, "North" },
		ButtonEntry{ GamepadButtonCode::Back, "Back" },
		ButtonEntry{ GamepadButtonCode::Guide, "Guide" },
		ButtonEntry{ GamepadButtonCode::Start, "Start" },
		ButtonEntry{ GamepadButtonCode::LeftStick, "Left stick" },
		ButtonEntry{ GamepadButtonCode::RightStick, "Right stick" },
		ButtonEntry{ GamepadButtonCode::LeftShoulder, "Left shoulder" },
		ButtonEntry{ GamepadButtonCode::RightShoulder, "Right shoulder" },
		ButtonEntry{ GamepadButtonCode::DPadUp, "D-pad up" },
		ButtonEntry{ GamepadButtonCode::DPadDown, "D-pad down" },
		ButtonEntry{ GamepadButtonCode::DPadLeft, "D-pad left" },
		ButtonEntry{ GamepadButtonCode::DPadRight, "D-pad right" },
		ButtonEntry{ GamepadButtonCode::Misc1, "Misc 1" },
		ButtonEntry{ GamepadButtonCode::RightPaddle1, "Right paddle 1" },
		ButtonEntry{ GamepadButtonCode::LeftPaddle1, "Left paddle 1" },
		ButtonEntry{ GamepadButtonCode::RightPaddle2, "Right paddle 2" },
		ButtonEntry{ GamepadButtonCode::LeftPaddle2, "Left paddle 2" },
		ButtonEntry{ GamepadButtonCode::Touchpad, "Touchpad" },
		ButtonEntry{ GamepadButtonCode::Misc2, "Misc 2" },
		ButtonEntry{ GamepadButtonCode::Misc3, "Misc 3" },
		ButtonEntry{ GamepadButtonCode::Misc4, "Misc 4" },
		ButtonEntry{ GamepadButtonCode::Misc5, "Misc 5" },
		ButtonEntry{ GamepadButtonCode::Misc6, "Misc 6" },
	};

	const char* GetGamepadTypeName(GamepadType type) {
		switch (type) {
		case GamepadType::Standard:
			return "Standard";
		case GamepadType::Xbox360:
			return "Xbox 360";
		case GamepadType::XboxOne:
			return "Xbox One";
		case GamepadType::PlayStation3:
			return "PlayStation 3";
		case GamepadType::PlayStation4:
			return "PlayStation 4";
		case GamepadType::PlayStation5:
			return "PlayStation 5";
		case GamepadType::SwitchPro:
			return "Nintendo Switch Pro";
		case GamepadType::JoyConLeft:
			return "Joy-Con (L)";
		case GamepadType::JoyConRight:
			return "Joy-Con (R)";
		case GamepadType::JoyConPair:
			return "Joy-Con pair";
		case GamepadType::GameCube:
			return "GameCube";
		case GamepadType::Steam:
			return "Steam Controller";
		default:
			return "Unknown";
		}
	}

	void DrawAxis(const char* label, float value) {
		char valueText[32]{};
		sprintf_s(valueText, "%+.3f", value);
		ImGui::TextUnformatted(label);
		ImGui::SameLine(90.0f);
		ImGui::ProgressBar(
			(value + 1.0f) * 0.5f,
			ImVec2(180.0f, 0.0f),
			valueText);
	}

	void DrawVector(const char* label, const Vector3& value, const char* unit) {
		ImGui::Text(
			"%-15s X:%+9.4f  Y:%+9.4f  Z:%+9.4f %s",
			label,
			value.x,
			value.y,
			value.z,
			unit);
	}

	void DrawSensor(
		const Gamepad& gamepad,
		GamepadSensorType sensor,
		const char* label) {
		const bool hasGyroscope = gamepad.HasGyroscope(sensor);
		const bool hasAccelerometer = gamepad.HasAccelerometer(sensor);
		if (!hasGyroscope && !hasAccelerometer) {
			return;
		}

		ImGui::SeparatorText(label);
		if (hasGyroscope) {
			DrawVector("Gyroscope", gamepad.Gyroscope(sensor), "rad/s");
			DrawVector("Rotation delta", gamepad.RotationDelta(sensor), "rad/frame");
		} else {
			ImGui::TextDisabled("Gyroscope: unavailable");
		}
		if (hasAccelerometer) {
			DrawVector("Acceleration", gamepad.Acceleration(sensor), "m/s^2");
		} else {
			ImGui::TextDisabled("Accelerometer: unavailable");
		}
	}

	void DrawButtons(const Gamepad& gamepad) {
		ImGui::SeparatorText("Buttons");
		if (!ImGui::BeginTable("GamepadButtons", 4, ImGuiTableFlags_SizingStretchSame)) {
			return;
		}

		for (std::size_t index = 0; index < kButtons.size(); ++index) {
			const ButtonEntry& entry = kButtons[index];
			const GamepadButton button = gamepad.Button(entry.code);
			ImGui::TableNextColumn();
			ImGui::PushID(static_cast<int>(index));

			ImVec4 color{ 0.25f, 0.25f, 0.28f, 1.0f };
			if (button.Trigger()) {
				color = { 0.95f, 0.65f, 0.10f, 1.0f };
			} else if (button.Press()) {
				color = { 0.15f, 0.65f, 0.30f, 1.0f };
			} else if (button.Release()) {
				color = { 0.75f, 0.20f, 0.20f, 1.0f };
			}

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			ImGui::Button(entry.name, ImVec2(-1.0f, 0.0f));
			ImGui::PopStyleColor();
			if (button.Press()) {
				ImGui::SetItemTooltip("Pressed frames: %u", button.PressedFrames());
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	void DrawGamepad(const Gamepad& gamepad, uint32_t index) {
		const std::string name = gamepad.Name();
		ImGui::Text("Slot: %u", index);
		ImGui::Text("Name: %s", name.empty() ? "(unnamed)" : name.c_str());
		ImGui::Text("Type: %s", GetGamepadTypeName(gamepad.Type()));

		const Vector2 leftStick = gamepad.LeftStick();
		const Vector2 rightStick = gamepad.RightStick();
		ImGui::SeparatorText("Axes");
		DrawAxis("Left X", leftStick.x);
		DrawAxis("Left Y", leftStick.y);
		DrawAxis("Right X", rightStick.x);
		DrawAxis("Right Y", rightStick.y);
		DrawAxis("Left trigger", gamepad.LeftTrigger());
		DrawAxis("Right trigger", gamepad.RightTrigger());

		DrawButtons(gamepad);
		DrawSensor(gamepad, GamepadSensorType::Default, "Default IMU");
		DrawSensor(gamepad, GamepadSensorType::Left, "Left Joy-Con IMU");
		DrawSensor(gamepad, GamepadSensorType::Right, "Right Joy-Con IMU");
	}

}

GamepadTestScene::GamepadTestScene(const InitData& init) :
	SampleScene(init) {
}

void GamepadTestScene::Update() {
	UpdateCamera();

	const std::optional<Gamepad> leftJoyCon = JoyCon::Left();
	const std::optional<Gamepad> rightJoyCon = JoyCon::Right();
	if (leftJoyCon.has_value() != wasLeftConnected_) {
		leftRotation_ = {};
	}
	if (rightJoyCon.has_value() != wasRightConnected_) {
		rightRotation_ = {};
	}
	wasLeftConnected_ = leftJoyCon.has_value();
	wasRightConnected_ = rightJoyCon.has_value();

	if (leftJoyCon) {
		leftRotation_ += leftJoyCon->RotationDelta();
	}
	if (rightJoyCon) {
		rightRotation_ += rightJoyCon->RotationDelta();
	}
	if (Key::R.Trigger()) {
		leftRotation_ = {};
		rightRotation_ = {};
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}
}

void GamepadTestScene::Draw() const {
	DrawStage();
	leftOrientationBox_.Draw(
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },
			Math::MakeQuaternion(leftRotation_),
			{ -2.0f, 2.0f, 0.0f }),
		wasLeftConnected_ ? Color::RoyalBlue : Color::Gray);
	rightOrientationBox_.Draw(
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },
			Math::MakeQuaternion(rightRotation_),
			{ 2.0f, 2.0f, 0.0f }),
		wasRightConnected_ ? Color::Crimson : Color::Gray);

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(720.0f, 650.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Gamepad / Joy-Con Monitor")) {
		ImGui::TextUnformatted("R: reset 3D orientation | Escape: open the original sample scenes");
		ImGui::TextDisabled(
			"Button colors: gray=idle, orange=trigger, green=pressed, red=release");
		ImGui::Text(
			"Joy-Con (L): %s | Joy-Con (R): %s",
			JoyCon::Left() ? "connected" : "disconnected",
			JoyCon::Right() ? "connected" : "disconnected");

		bool isGamepadFound = false;
		if (ImGui::BeginTabBar("ConnectedGamepads")) {
			for (uint32_t index = 0; index < Gamepad::MaxCount; ++index) {
				const Gamepad gamepad{ index };
				if (!gamepad.IsConnected()) {
					continue;
				}
				isGamepadFound = true;
				const std::string label =
					gamepad.Name() + "##gamepad" + std::to_string(index);
				if (ImGui::BeginTabItem(label.c_str())) {
					DrawGamepad(gamepad, index);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}

		if (!isGamepadFound) {
			ImGui::Spacing();
			ImGui::TextColored(
				ImVec4(1.0f, 0.75f, 0.20f, 1.0f),
				"No gamepad detected.");
			ImGui::TextWrapped(
				"Wake the Joy-Con with a button press. Windows Bluetooth must show it as connected.");
		}
	}
	ImGui::End();
}
