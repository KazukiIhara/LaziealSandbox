#include <LGF/LGF.h>

#include <array>
#include <cstddef>

using namespace LGF;

RuntimeConfig ConfigureRuntime() {
	RuntimeConfig config{};
	config.assetRoot = "../Dependencies/LaziealRuntime/Dependencies/LaziealGraphicsFramework/Project/Assets";
	config.window.title = L"Lazieal Graphics Framework";
	config.window.width = 800;
	config.window.height = 600;
	return config;
}

void Main() {
	Model flightHelmet("Assets/Models/FlightHelmet/FlightHelmet.gltf");
	Transform3D flightHelmetTransform{};
	flightHelmetTransform.scale = { 5.0f, 5.0f, 5.0f };
	flightHelmetTransform.translation = { -1.5f, -1.75f, 0.0f };

	Model waterBottle("Assets/Models/WaterBottle/WaterBottle.glb");
	Transform3D waterBottleTransform{};
	waterBottleTransform.scale = { 10.0f, 10.0f, 10.0f };
	waterBottleTransform.translation = { 1.5f, -0.45f, 0.0f };

	const Ground3D ground(100.0f);
	const Matrix4x4 groundWorldMatrix =
		Math::MakeTranslateMatrix({ 0.0f, -1.75f, 0.0f });
	const Material3D groundMaterial{
		.color = Color::Gray,
	};

	constexpr float kModelMoveSpeed = 2.0f;
	const ModelDrawOptions modelDrawOptions{
		.enableNormalMap = true,
		.normalScale = 1.0f,
	};

	Camera3D::Param sceneCameraParam{};
	sceneCameraParam.farClip = 80.0f;
	Camera3D sceneCamera(
		{ 0.0f, 4.0f, -40.0f },
		0.0f,
		5.0f * Math::DEG_TO_RAD,
		sceneCameraParam);

	DebugCamera3D debugCamera(85.0f);
	debugCamera.yaw = 45.0f * Math::DEG_TO_RAD;
	debugCamera.pitch = 30.0f * Math::DEG_TO_RAD;

	constexpr std::array<float, 4> kModelRowZ{
		-35.0f,
		-29.0f,
		-17.0f,
		15.0f,
	};
	constexpr std::array<float, 4> kModelColumnSpacing{
		1.0f,
		2.0f,
		4.0f,
		7.0f,
	};
	constexpr std::size_t kModelsPerRow = 5u;
	DirectionalLight directionalLight({ 1.0f, -1.0f, 0.5f }, Color::White, 1.5f);
	Skybox skybox("Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.dds");

	while (System::Update()) {
		Vector3 moveDirection{
			static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
			0.0f,
			static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
		};
		if (Math::LengthSq(moveDirection) > Math::EPSILON) {
			moveDirection = Math::Normalize(moveDirection);
			flightHelmetTransform.translation +=
				moveDirection * kModelMoveSpeed * static_cast<float>(System::DeltaTime());
		}

		debugCamera.Update();
		debugCamera.SetScene();

		sceneCamera.SetShadowScene();
		sceneCamera.DrawDebug({
			.showFrustum = true,
			.showShadowCascades = true,
		});
		directionalLight.SetScene();
		skybox.SetScene();

		ground.Draw(groundWorldMatrix, groundMaterial);
		flightHelmet.Draw(Math::MakeAffineMatrix(
			flightHelmetTransform.scale,
			flightHelmetTransform.rotation,
			flightHelmetTransform.translation),
			modelDrawOptions);
		waterBottle.Draw(Math::MakeAffineMatrix(
			waterBottleTransform.scale,
			waterBottleTransform.rotation,
			waterBottleTransform.translation),
			modelDrawOptions);

		for (std::size_t rowIndex = 0; rowIndex < kModelRowZ.size(); ++rowIndex) {
			for (std::size_t columnIndex = 0; columnIndex < kModelsPerRow; ++columnIndex) {
				const float x =
					(static_cast<float>(columnIndex) - 2.0f) * kModelColumnSpacing[rowIndex];
				if ((rowIndex + columnIndex) % 2u == 0u) {
					const Vector3 translation{ x, -1.75f, kModelRowZ[rowIndex] };
					flightHelmet.Draw(Math::MakeAffineMatrix(
						flightHelmetTransform.scale,
						flightHelmetTransform.rotation,
						translation),
						modelDrawOptions);
				} else {
					const Vector3 translation{ x, -0.45f, kModelRowZ[rowIndex] };
					waterBottle.Draw(Math::MakeAffineMatrix(
						waterBottleTransform.scale,
						waterBottleTransform.rotation,
						translation),
						modelDrawOptions);
				}
			}
		}
	}
}
