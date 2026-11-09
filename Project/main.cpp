#include <LGF/LGF.h>
#include <imgui.h>

using namespace LGF;

void Main() {
	VFX::ParticleWorld world{ VFX::LoadParticleWorldSettings("Assets/Parameters/ParticleWorld.json") };
	const auto emitter = world.CreateEmitter(VFX::LoadParticleEmitterDesc("Assets/Parameters/ParticleEmitter.json"));
	Camera3D camera{ { 0.0f, 2.0f, -8.0f } };
	bool continuous = true;
	while (System::Update()) {
		camera.SetScene();
		if (ImGui::Begin("Particle Demo")) {
			if (ImGui::Button("Burst (200)")) {
				world.Emit(emitter, 200u);
			}
			if (ImGui::Checkbox("Continuous emission", &continuous)) {
				world.SetEmitterEnabled(emitter, continuous);
			}
		}
		ImGui::End();
		world.Update(static_cast<float>(System::DeltaTime()));
		world.Draw();
		world.DrawDebug();
	}
}
