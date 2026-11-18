#include <LGF/LGF.h>

#include <cmath>
#include <stdexcept>

#include <imgui.h>

using namespace LGF;

namespace {

	constexpr const char* kEffectPath =
		"Assets/Effects/Effekseer/Laser01.efkefc";

	enum class EffectCommand {
		None,
		Restart,
		Stop,
	};

	Transform3D MakeEffectTransform(const Vector3& basePosition, float scale, float rotationY, float elapsedTime, bool animate) {

		Transform3D transform{};
		const float animatedRotationY = rotationY +
			(animate ? elapsedTime * 45.0f * Math::DEG_TO_RAD : 0.0f);
		transform.scale = { scale, scale, scale };
		transform.rotation = Math::MakeQuaternion({
			0.0f,
			animatedRotationY,
			0.0f,
			});
		transform.translation = basePosition;
		if (animate) {
			transform.translation.x += std::sin(elapsedTime) * 3.0f;
			transform.translation.z += std::cos(elapsedTime) * 3.0f;
		}
		return transform;
	}

	EffectCommand DrawControlPanel(
		Vector3& position,
		float& scale,
		float& rotationY,
		bool& animate,
		bool& autoReplay,
		bool isPlaying) {
		EffectCommand command = EffectCommand::None;
		if (!ImGui::Begin("Effekseer Effect")) {
			ImGui::End();
			return command;
		}

		ImGui::TextUnformatted("Arrow keys: orbit camera   Q / E: distance");
		ImGui::Text("State: %s", isPlaying ? "Playing" : "Stopped");
		ImGui::Separator();

		if (ImGui::Button("Restart")) {
			command = EffectCommand::Restart;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			command = EffectCommand::Stop;
		}

		ImGui::Checkbox("Auto Replay", &autoReplay);
		ImGui::Checkbox("Animate Transform", &animate);
		ImGui::DragFloat3("Position", &position.x, 0.05f);
		ImGui::SliderFloat("Scale", &scale, 0.1f, 3.0f);
		ImGui::SliderAngle("Rotation Y", &rotationY);

		ImGui::End();
		return command;
	}

}

void Main() {
	DebugCamera3D camera(20.0f);
	DirectionalLight directionalLight(
		{ -0.5f, -1.0f, 0.3f },
		Color::White,
		1.5f);
	Ground3D ground(30.0f, 30.0f);
	Grid3D grid(24.0f, 1.0f);

	VFX::EffectWorld effectWorld;
	VFX::Effect effect = effectWorld.Load(kEffectPath);
	if (!effect.IsValid()) {
		throw std::runtime_error("Failed to load the Effekseer sample effect.");
	}

	Vector3 effectPosition{ 0.0f, 0.0f, 0.0f };
	float effectScale = 1.0f;
	float effectRotationY{};
	float elapsedTime{};
	bool animateTransform = true;
	bool autoReplay = true;

	Transform3D effectTransform = MakeEffectTransform(
		effectPosition,
		effectScale,
		effectRotationY,
		elapsedTime,
		animateTransform);
	VFX::EffectHandle effectHandle = effectWorld.Play(effect, effectTransform);
	if (!effectHandle.IsValid()) {
		throw std::runtime_error("Failed to play the Effekseer sample effect.");
	}

	Graphics3D::ResetPostProcessProfile();
	Graphics3D::SetExposure(0.0f);

	while (System::Update()) {
		const float deltaTime = static_cast<float>(System::DeltaTime());
		elapsedTime += deltaTime;

		camera.Update();
		effectWorld.Update(deltaTime);

		const bool isPlaying = effectWorld.IsPlaying(effectHandle);
		const EffectCommand command = DrawControlPanel(
			effectPosition,
			effectScale,
			effectRotationY,
			animateTransform,
			autoReplay,
			isPlaying);

		effectTransform = MakeEffectTransform(
			effectPosition,
			effectScale,
			effectRotationY,
			elapsedTime,
			animateTransform);

		if (command == EffectCommand::Stop) {
			effectWorld.Stop(effectHandle);
			autoReplay = false;
		} else if (command == EffectCommand::Restart) {
			effectWorld.Stop(effectHandle);
			effectHandle = effectWorld.Play(effect, effectTransform);
		} else if (!isPlaying && autoReplay) {
			effectHandle = effectWorld.Play(effect, effectTransform);
		}

		if (effectWorld.IsPlaying(effectHandle)) {
			effectWorld.SetTransform(effectHandle, effectTransform);
		}

		camera.SetScene();
		directionalLight.SetScene();
		ground.Draw(
			Math::MakeTranslateMatrix({ 0.0f, -2.0f, 0.0f }),
			Material3D{
				.color = { 0.12f, 0.14f, 0.18f, 1.0f },
				.metallic = 0.1f,
				.roughness = 0.75f,
			});
		grid.Draw(Math::MakeTranslateMatrix({ 0.0f, -1.98f, 0.0f }));
	}
}
