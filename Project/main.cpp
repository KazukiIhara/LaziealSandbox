#include <LGF/LGF.h>

#include <cmath>
#include <memory>

using namespace LGF;

namespace {

	constexpr const char* kFontPath =
		"Assets/Fonts/NotoSansJP/NotoSansJP-wght.ttf";
	constexpr const char* kFadeBlackPath =
		"../Dependencies/LaziealRuntime/Dependencies/LaziealGraphicsFramework/Project/Assets/"
		"JsonData/SceneTransition/FadeBlack.json";
	constexpr const char* kHorizontalWipePath =
		"../Dependencies/LaziealRuntime/Dependencies/LaziealGraphicsFramework/Project/Assets/"
		"JsonData/SceneTransition/HorizontalWipe.json";

	enum class DemoSceneState {
		Blue,
		Orange,
	};

	struct DemoSceneData {
		Font2D titleFont{ kFontPath, 64u };
		Font2D textFont{ kFontPath, 24u };
	};

	using DemoScene = IScene<DemoSceneState, DemoSceneData>;

	void DrawInstructions(const DemoSceneData& data, const Vector2& screenCenter, const Vector4& color) {
		data.textFont.Draw(
			"SPACE : FadeBlack",
			{ screenCenter.x, screenCenter.y + 100.0f },
			Font2DDrawOptions{
				.color = color,
				.anchor = { 0.5f, 0.5f },
			});
		data.textFont.Draw(
			"ENTER : HorizontalWipe",
			{ screenCenter.x, screenCenter.y + 138.0f },
			Font2DDrawOptions{
				.color = color,
				.anchor = { 0.5f, 0.5f },
			});
	}

	class BlueScene final : public DemoScene {
	public:
		using DemoScene::DemoScene;

		void Update() override {
			elapsedTime_ += static_cast<float>(System::DeltaTime());
			if (Key::Space.Trigger()) {
				ChangeScene(DemoSceneState::Orange, "FadeBlack");
			} else if (Key::Enter.Trigger()) {
				ChangeScene(DemoSceneState::Orange, "HorizontalWipe");
			}
		}

		void Draw() const override {
			const Vector2 screenSize{
				static_cast<float>(Graphics2D::GetWindowWidth()),
				static_cast<float>(Graphics2D::GetWindowHeight()),
			};
			const Vector2 screenCenter = screenSize * 0.5f;
			Rect2D{ {}, screenSize }.Draw(Vector4{ 0.03f, 0.12f, 0.2f, 1.0f });

			const float orbit = elapsedTime_ * 1.5f;
			Circle2D{
				{
					screenCenter.x + std::cos(orbit) * 230.0f,
					screenCenter.y + std::sin(orbit) * 90.0f,
				},
				72.0f,
			}.Draw(Vector4{ 0.1f, 0.75f, 1.0f, 0.75f });
			Circle2D{
				{
					screenCenter.x + std::cos(orbit + Math::PI) * 230.0f,
					screenCenter.y + std::sin(orbit + Math::PI) * 90.0f,
				},
				48.0f,
			}.Draw(Vector4{ 0.35f, 1.0f, 0.8f, 0.65f });

			GetData().titleFont.Draw(
				"SCENE BLUE",
				screenCenter,
				Font2DDrawOptions{
					.color = Color::White,
					.anchor = { 0.5f, 0.5f },
				});
			DrawInstructions(GetData(), screenCenter, { 0.65f, 0.9f, 1.0f, 1.0f });
		}

	private:
		float elapsedTime_ = 0.0f;
	};

	class OrangeScene final : public DemoScene {
	public:
		using DemoScene::DemoScene;

		void Update() override {
			elapsedTime_ += static_cast<float>(System::DeltaTime());
			if (Key::Space.Trigger()) {
				ChangeScene(DemoSceneState::Blue, "FadeBlack");
			} else if (Key::Enter.Trigger()) {
				ChangeScene(DemoSceneState::Blue, "HorizontalWipe");
			}
		}

		void Draw() const override {
			const Vector2 screenSize{
				static_cast<float>(Graphics2D::GetWindowWidth()),
				static_cast<float>(Graphics2D::GetWindowHeight()),
			};
			const Vector2 screenCenter = screenSize * 0.5f;
			Rect2D{ {}, screenSize }.Draw(Vector4{ 0.24f, 0.06f, 0.03f, 1.0f });

			const float movement = std::sin(elapsedTime_ * 2.0f) * 120.0f;
			RoundRect2D{
				{ screenCenter.x - 310.0f + movement, screenCenter.y - 190.0f },
				{ 180.0f, 90.0f },
				24.0f,
			}.Draw(Vector4{ 1.0f, 0.35f, 0.08f, 0.8f });
			RoundRect2D{
				{ screenCenter.x + 130.0f - movement, screenCenter.y + 100.0f },
				{ 180.0f, 90.0f },
				24.0f,
			}.Draw(Vector4{ 1.0f, 0.7f, 0.12f, 0.75f });

			GetData().titleFont.Draw(
				"SCENE ORANGE",
				screenCenter,
				Font2DDrawOptions{
					.color = Color::White,
					.anchor = { 0.5f, 0.5f },
				});
			DrawInstructions(GetData(), screenCenter, { 1.0f, 0.82f, 0.58f, 1.0f });
		}

	private:
		float elapsedTime_ = 0.0f;
	};

}

void Main() {
	auto data = std::make_shared<DemoSceneData>();
	SceneManager<DemoSceneState, DemoSceneData> sceneManager(data);
	sceneManager
		.Register<BlueScene>(DemoSceneState::Blue)
		.Register<OrangeScene>(DemoSceneState::Orange);
	sceneManager.RegisterTransition("FadeBlack", LoadSceneTransition(kFadeBlackPath));
	sceneManager.RegisterTransition("HorizontalWipe", LoadSceneTransition(kHorizontalWipePath));

	while (System::Update()) {
		sceneManager.Update();
		sceneManager.Draw();
	}
}
