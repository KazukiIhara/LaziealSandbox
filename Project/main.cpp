#include <LGF/LGF.h>

#include <string>
#include <vector>

using namespace LGF;

void Main() {
	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	const Font2D titleFont{ kFontPath, 40u };
	const Font2D buttonFont{ kFontPath, 24u };
	const Font2D captionFont{ kFontPath, 18u };

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

	const InputCommand previousCommand{ "UI.Previous" };
	const InputCommand nextCommand{ "UI.Next" };
	const InputCommand submitCommand{ "UI.Submit" };

	UIWorld uiWorld;
	std::vector<UIButtonHandle> buttons;
	buttons.push_back(uiWorld.AddButton({
		.text = "Start Game",
		.transform = {
			.position = { 0.0f, -120.0f },
			.size = { 320.0f, 64.0f },
			.anchor = { 0.5f, 0.5f },
			.pivot = { 0.5f, 0.5f },
		},
		.isCursorEnabled = false,
		.isNavigationEnabled = true,
		}));
	buttons.push_back(uiWorld.AddButton({
		.text = "Options",
		.transform = {
			.position = { 0.0f, -40.0f },
			.size = { 320.0f, 64.0f },
			.anchor = { 0.5f, 0.5f },
			.pivot = { 0.5f, 0.5f },
		},
		.isCursorEnabled = false,
		.isNavigationEnabled = true,
		}));
	buttons.push_back(uiWorld.AddButton({
		.text = "Credits",
		.transform = {
			.position = { 0.0f, 40.0f },
			.size = { 320.0f, 64.0f },
			.anchor = { 0.5f, 0.5f },
			.pivot = { 0.5f, 0.5f },
		},
		.isCursorEnabled = false,
		.isNavigationEnabled = true,
		}));
	buttons.push_back(uiWorld.AddButton({
		.text = "Exit",
		.transform = {
			.position = { 0.0f, 120.0f },
			.size = { 320.0f, 64.0f },
			.anchor = { 0.5f, 0.5f },
			.pivot = { 0.5f, 0.5f },
		},
		.isCursorEnabled = false,
		.isNavigationEnabled = true,
		}));

	std::string message = "Select a button";

	while (System::Update()) {
		int32_t navigationOffset = 0;
		if (previousCommand.Trigger()) {
			navigationOffset = -1;
		} else if (nextCommand.Trigger()) {
			navigationOffset = 1;
		}

		uiWorld.Update({
			.navigationOffset = navigationOffset,
			.isNavigationPressed = submitCommand.Press(),
			.isCursorEnabled = false,
			});

		for (const UIButtonHandle button : buttons) {
			if (uiWorld.Trigger(button)) {
				message = uiWorld.GetText(button) + " selected";
			}
		}

		const Window::Size windowSize = Window::GetSize();
		titleFont.Draw(
			"Simple UIWorld Demo",
			{ static_cast<float>(windowSize.width) * 0.5f, 48.0f },
			{
				.color = Color::White,
				.anchor = { 0.5f, 0.0f },
			});
			captionFont.Draw(
				"W / S, Arrow Keys, D-Pad: Select    Enter / Space / Pad South: Submit",
				{ static_cast<float>(windowSize.width) * 0.5f, 104.0f },
			{
				.color = Color::LightGray,
				.anchor = { 0.5f, 0.0f },
			});

			uiWorld.Draw(buttonFont);

			captionFont.Draw(
				message,
				{
					static_cast<float>(windowSize.width) * 0.5f,
					static_cast<float>(windowSize.height) - 72.0f,
				},
			{
				.color = Color::Yellow,
				.anchor = { 0.5f, 0.0f },
			});
	}
}
