#include <LGF/LGF.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

using namespace LGF;

void Main() {
	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	const Font2D titleFont{ kFontPath, 40u };
	const Font2D buttonFont2D{ kFontPath, 20u };
	const Font2D bodyFont{ kFontPath, 20u };
	const Font2D captionFont{ kFontPath, 16u };
	const Font3D buttonFont3D{ kFontPath, 32u };

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

	DebugCamera3D camera{ 8.0f };
	const DirectionalLight light{
		{ 0.3f, -1.0f, 0.5f },
		Color::White,
		1.5f,
	};
	const Grid3D grid{ 16.0f, 1.0f };
	const UIPanel3D buttonPanel{};

	UIWorld uiWorld;
	std::string triggerMessage = "Trigger: -";
	std::string holdMessage = "Hold: -";
	std::string releaseMessage = "Release: -";
	uint32_t triggerCount = 0u;
	uint32_t holdCount = 0u;
	uint32_t releaseCount = 0u;
	std::vector<std::pair<UIButtonHandle, std::string>> buttons;

	const auto addButton3D = [
		&uiWorld,
		&buttons,
		&buttonPanel,
		&buttonFont3D](
			std::string text,
			float positionY,
			bool cursorEnabled,
			bool navigationEnabled,
			std::function<bool()> isDirectPressed = {}) {
		UIButtonDesc3D desc{};
		desc.transform = {
			.position = { 0.0f, positionY, 0.0f },
			.size = { 3.6f, 0.65f },
		};
		desc.draw = [&buttonPanel, &buttonFont3D, text](
			const UIButtonDrawContext3D& context) {
			Vector4 color{ 0.12f, 0.18f, 0.32f, 1.0f };
			if (context.state.isSelected) {
				color = { 0.95f, 0.42f, 0.04f, 1.0f };
			}
			if (context.state.isHovered) {
				color = { 0.05f, 0.42f, 1.0f, 1.0f };
			}
			if (context.state.isHeld) {
				color = { 1.0f, 0.08f, 0.48f, 1.0f };
			}

			if (context.state.isSelected || context.state.isHovered ||
				context.state.isHeld) {
				Vector4 outlineColor = context.state.isSelected
					? Color::Yellow
					: Color::Cyan;
				if (context.state.isHovered) {
					outlineColor = Color::Cyan;
				}
				if (context.state.isHeld) {
					outlineColor = Color::White;
				}

				const Matrix4x4 outlineMatrix = Math::MakeAffineMatrix(
					{ 1.04f, 1.18f, 1.0f },
					{},
					{ 0.0f, 0.0f, 0.01f }) * context.worldMatrix;
				buttonPanel.Draw(outlineMatrix, outlineColor);
			}

			buttonPanel.Draw(context.worldMatrix, color);
			buttonFont3D.Draw(
				text,
				Math::Transform({ 0.0f, 0.0f, -0.01f }, context.worldMatrix),
				{
					.color = Color::White,
					.anchor = { 0.5f, 0.5f },
					.worldUnitsPerPixel = 0.008f,
					.mode = Font3DMode::World,
				});
		};
		desc.isDirectPressed = std::move(isDirectPressed);
		desc.isCursorEnabled = cursorEnabled;
		desc.isNavigationEnabled = navigationEnabled;
		const UIButtonHandle handle = uiWorld.AddButton3D(desc);
		buttons.emplace_back(handle, std::move(text));
		return handle;
	};

	addButton3D("Cursor + Navigation", 1.2f, true, true);
	addButton3D("Cursor Only", 0.4f, true, false);
	addButton3D("Navigation Only", -0.4f, false, true);
	addButton3D(
		"Direct Input: 1 / Pad East",
		-1.2f,
		false,
		false,
		[&directButtonCommand] {
			return directButtonCommand.Press();
		});

	UIButtonDesc overlayButton{};
	overlayButton.transform = {
		.position = { 24.0f, 184.0f },
		.size = { 240.0f, 48.0f },
	};
	overlayButton.draw = [&buttonFont2D](const UIButtonDrawContext& context) {
		const Vector4 color = context.state.isHeld
			? Color::HotPink
			: context.state.isHovered
				? Color::RoyalBlue
				: Vector4{ 0.05f, 0.08f, 0.16f, 0.94f };
		context.rect.Draw(color);
		buttonFont2D.Draw(
			"2D Overlay Button",
			context.rect.leftTop + context.rect.size * 0.5f,
			{
				.color = Color::White,
				.anchor = { 0.5f, 0.5f },
			});
	};
	buttons.emplace_back(
		uiWorld.AddButton(overlayButton),
		"2D Overlay Button");

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
			.cursor3D = MakeUICursor3D(camera, cursorPosition, 100.0f),
		});

		for (const auto& [handle, text] : buttons) {
			const UIButtonEvents events = uiWorld.GetEvents(handle);
			if (events.triggered) {
				++triggerCount;
				triggerMessage = "Trigger: " + text +
					" (" + std::to_string(triggerCount) + ")";
			}
			if (events.held) {
				++holdCount;
				holdMessage = "Hold: " + text +
					" (" + std::to_string(holdCount) + ")";
			}
			if (events.released) {
				++releaseCount;
				releaseMessage = "Release: " + text +
					" (" + std::to_string(releaseCount) + ")";
			}
		}

		camera.SetScene();
		light.SetScene();
		grid.Draw();
		uiWorld.Draw3D();
		if (areHitboxesVisible) {
			uiWorld.DrawHitboxes3D(Color::Lime);
		}

		titleFont.Draw(
			"InputCommand + 2D / 3D UIWorld Demo",
			{ static_cast<float>(windowSize.width) * 0.5f, 28.0f },
			{
				.color = Color::AliceBlue,
				.anchor = { 0.5f, 0.0f },
			});
		captionFont.Draw(
			"Mouse: Move / Left Click    Pad Cursor: Left Stick / Right Shoulder",
			{ 24.0f, 88.0f },
			{ .color = Color::LightGray });
		captionFont.Draw(
			"Selection: W/S, Up/Down, Enter/Space    Pad: D-Pad / South",
			{ 24.0f, 114.0f },
			{ .color = Color::LightGray });
		captionFont.Draw(
			"Direct: 1 / Pad East    Hitbox: F / Pad North",
			{ 24.0f, 140.0f },
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
