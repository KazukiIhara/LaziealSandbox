#include <LGF/LGF.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>

using namespace LGF;

void Main() {
	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	const Font2D titleFont{ kFontPath, 40u };
	const Font2D buttonFont{ kFontPath, 24u };
	const Font2D bodyFont{ kFontPath, 20u };
	const Font2D captionFont{ kFontPath, 16u };

	Input::ClearCommands();
	Input::RegisterCommand("UI.Previous", {
		Key::W,
		Key::Up,
		Gamepad{ 0u }.Button(GamepadButtonCode::DPadUp),
	});
	Input::RegisterCommand("UI.Next", {
		Key::S,
		Key::Down,
		Gamepad{ 0u }.Button(GamepadButtonCode::DPadDown),
	});
	Input::RegisterCommand("UI.Submit", {
		Key::Enter,
		Key::Space,
		Gamepad{ 0u }.Button(GamepadButtonCode::South),
	});
	Input::RegisterCommand("UI.PointerSubmit", {
		Mouse::Left,
		Gamepad{ 0u }.Button(GamepadButtonCode::RightShoulder),
	});
	Input::RegisterCommand("UI.DirectButton", {
		Key::Num1,
		Gamepad{ 0u }.Button(GamepadButtonCode::East),
	});
	Input::RegisterCommand("UI.ToggleHitboxes", {
		Key::F,
		Gamepad{ 0u }.Button(GamepadButtonCode::North),
	});

	const InputCommand previousCommand{ "UI.Previous" };
	const InputCommand nextCommand{ "UI.Next" };
	const InputCommand submitCommand{ "UI.Submit" };
	const InputCommand pointerSubmitCommand{ "UI.PointerSubmit" };
	const InputCommand directButtonCommand{ "UI.DirectButton" };
	const InputCommand toggleHitboxesCommand{ "UI.ToggleHitboxes" };

	UIWorld uiWorld;
	std::string triggerMessage = "Trigger: -";
	std::string holdMessage = "Hold: -";
	std::string releaseMessage = "Release: -";
	uint32_t triggerCount = 0u;
	uint32_t holdCount = 0u;
	uint32_t releaseCount = 0u;

	const auto addButton = [
		&uiWorld,
		&buttonFont,
		&triggerMessage,
		&holdMessage,
		&releaseMessage,
		&triggerCount,
		&holdCount,
		&releaseCount](
			std::string text,
			float positionY,
			bool cursorEnabled,
			bool navigationEnabled,
			std::function<bool()> isDirectPressed = {}) {
		UIButtonDesc desc{};
		desc.transform = {
			.position = { 0.0f, positionY },
			.size = { 440.0f, 58.0f },
			.anchor = { 0.5f, 0.5f },
			.pivot = { 0.5f, 0.5f },
		};
		desc.callbacks = {
			.onTrigger = [&, text] {
				++triggerCount;
				triggerMessage = "Trigger: " + text +
					" (" + std::to_string(triggerCount) + ")";
			},
			.onHold = [&, text] {
				++holdCount;
				holdMessage = "Hold: " + text +
					" (" + std::to_string(holdCount) + ")";
			},
			.onRelease = [&, text] {
				++releaseCount;
				releaseMessage = "Release: " + text +
					" (" + std::to_string(releaseCount) + ")";
			},
		};
		desc.draw = [&buttonFont, text](const UIButtonDrawContext& context) {
			Vector4 color{ 0.08f, 0.10f, 0.16f, 0.96f };
			if (context.state.isHovered) {
				color = Color::RoyalBlue;
			}
			if (context.state.isSelected) {
				color = Color::DarkOrange;
			}
			if (context.state.isHeld) {
				color = Color::HotPink;
			}

			context.rect.Draw(color);
			buttonFont.Draw(
				text,
				context.rect.leftTop + context.rect.size * 0.5f,
				{
					.color = Color::White,
					.anchor = { 0.5f, 0.5f },
				});
		};
		desc.isDirectPressed = std::move(isDirectPressed);
		desc.isCursorEnabled = cursorEnabled;
		desc.isNavigationEnabled = navigationEnabled;
		return uiWorld.AddButton(desc);
	};

	addButton("Cursor + Navigation", -108.0f, true, true);
	addButton("Cursor Only", -36.0f, true, false);
	addButton("Navigation Only", 36.0f, false, true);
	addButton(
		"Direct Input: 1 / Pad East",
		108.0f,
		false,
		false,
		[&directButtonCommand] {
			return directButtonCommand.Press();
		});

	const Window::Size initialWindowSize = Window::GetSize();
	Vector2 cursorPosition{
		static_cast<float>(initialWindowSize.width) * 0.5f,
		static_cast<float>(initialWindowSize.height) * 0.5f,
	};
	bool isPadCursor = false;
	bool areHitboxesVisible = false;

	while (System::Update()) {
		const Gamepad gamepad{ 0u };
		const Vector2 stick = gamepad.LeftStick();
		constexpr float kStickDeadZone = 0.18f;
		constexpr float kCursorSpeed = 720.0f;
		if (std::abs(stick.x) >= kStickDeadZone ||
			std::abs(stick.y) >= kStickDeadZone) {
			isPadCursor = true;
			const float deltaTime = static_cast<float>(System::DeltaTime());
			cursorPosition += Vector2{ stick.x, -stick.y } *
				(kCursorSpeed * deltaTime);
		}

		const Vector2 mouseCursorDelta = Cursor::Delta();
		if (mouseCursorDelta.x != 0.0f || mouseCursorDelta.y != 0.0f) {
			isPadCursor = false;
		}
		if (!isPadCursor) {
			cursorPosition = Cursor::Pos();
		}

		const Window::Size windowSize = Window::GetSize();
		cursorPosition.x = std::clamp(
			cursorPosition.x,
			0.0f,
			static_cast<float>(windowSize.width));
		cursorPosition.y = std::clamp(
			cursorPosition.y,
			0.0f,
			static_cast<float>(windowSize.height));

		int32_t navigationOffset = 0;
		if (previousCommand.Trigger()) {
			navigationOffset = -1;
		} else if (nextCommand.Trigger()) {
			navigationOffset = 1;
		}
		if (toggleHitboxesCommand.Trigger()) {
			areHitboxesVisible = !areHitboxesVisible;
		}

		uiWorld.Update({
			.cursorPosition = cursorPosition,
			.isCursorPressed = pointerSubmitCommand.Press(),
			.navigationOffset = navigationOffset,
			.isNavigationPressed = submitCommand.Press(),
		});

		Rect2D{
			0.0f,
			0.0f,
			static_cast<float>(windowSize.width),
			static_cast<float>(windowSize.height),
		}.Draw(Vector4{ 0.015f, 0.02f, 0.04f, 1.0f });

		titleFont.Draw(
			"InputCommand + UIWorld Demo",
			{ static_cast<float>(windowSize.width) * 0.5f, 34.0f },
			{
				.color = Color::AliceBlue,
				.anchor = { 0.5f, 0.0f },
			});

		captionFont.Draw(
			"Mouse: Move / Left Click    Pad Cursor: Left Stick / Right Shoulder",
			{ 24.0f, 94.0f },
			{ .color = Color::LightGray });
		captionFont.Draw(
			"Selection: W/S, Up/Down, Enter/Space    Pad: D-Pad / South",
			{ 24.0f, 120.0f },
			{ .color = Color::LightGray });
		captionFont.Draw(
			"Direct: 1 / Pad East    Hitbox: F / Pad North",
			{ 24.0f, 146.0f },
			{ .color = Color::LightGray });

		uiWorld.Draw();
		if (areHitboxesVisible) {
			uiWorld.DrawHitboxes({ 0.0f, 1.0f, 0.0f, 0.22f });
		}

		const float statusTop = static_cast<float>(windowSize.height) - 128.0f;
		Rect2D{ 24.0f, statusTop, 660.0f, 104.0f }
			.Draw(Vector4{ 0.04f, 0.06f, 0.1f, 0.94f });
		bodyFont.Draw(triggerMessage, { 42.0f, statusTop + 12.0f });
		bodyFont.Draw(holdMessage, { 42.0f, statusTop + 40.0f });
		bodyFont.Draw(releaseMessage, { 42.0f, statusTop + 68.0f });

		const Vector4 cursorColor = isPadCursor ? Color::HotPink : Color::Cyan;
		Rect2D{
			cursorPosition - Vector2{ 7.0f, 7.0f },
			{ 14.0f, 14.0f },
		}.Draw(cursorColor);
	}
}
