#include <LGF/LGF.h>

using namespace LGF;

void Main() {
	constexpr const char* kFontPath = "Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	const Font2D titleFont{ kFontPath, 48u };
	const Font2D bodyFont{ kFontPath, 28u };
	const Font2D captionFont{ kFontPath, 20u };
	const Font3D worldFont{ kFontPath, 64u };

	Camera3D camera{ { 0.0f, 1.5f, -8.0f } };
	const Grid3D grid{ 12.0f, 1.0f };
	const Font3DDrawOptions billboardOptions{
		.color = Color::AliceBlue,
		.anchor = { 0.5f, 0.5f },
		.worldUnitsPerPixel = 0.008f,
		.mode = Font3DMode::Billboard,
	};
	const Font3DDrawOptions worldOptions{
		.color = Color::Gold,
		.anchor = { 0.5f, 0.5f },
		.worldUnitsPerPixel = 0.008f,
		.mode = Font3DMode::World,
		.rotation = Math::MakeQuaternion({ 0.0f, -0.45f, 0.0f }),
	};

	while (System::Update()) {
		camera.SetScene();
		grid.Draw();
		worldFont.Draw(
			"Billboard Font3D / 常にカメラを向く",
			{ -4.0f, 2.2f, 2.0f },
			billboardOptions);
		worldFont.Draw(
			"World Font3D / 空間に固定",
			{ -3.6f, 0.8f, 2.0f },
			worldOptions);

		Rect2D{ 24.0f, 24.0f, 650.0f, 180.0f }
			.Draw(Vector4{ 0.025f, 0.035f, 0.06f, 0.94f });

		titleFont.Draw(
			"Font2D + Font3D Demo",
			{ 48.0f, 42.0f },
			{ .color = Color::AliceBlue });

		bodyFont.Draw(
			"2Dテキストは画面上、3Dテキストはワールド上に描画",
			{ 48.0f, 108.0f },
			{ .color = Color::White });

		captionFont.Draw(
			"Billboard: カメラ追従 / World: Quaternionで向きを固定",
			{ 48.0f, 154.0f },
			{ .color = Color::LightGray });
	}
}
