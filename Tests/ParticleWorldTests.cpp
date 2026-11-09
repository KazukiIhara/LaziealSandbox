#include <LGF/VFX/Particle/ParticleWorld.h>
#include <LGF/Parameter.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
	void Require(bool condition, const char* message) {
		if (!condition) {
			throw std::runtime_error(message);
		}
	}

	LGF::VFX::ParticleEmitterDesc MakeEmitter() {
		LGF::VFX::ParticleEmitterDesc desc;
		desc.minimumVelocity = {};
		desc.maximumVelocity = {};
		desc.acceleration = {};
		desc.minimumLifetime = 1.0f;
		desc.maximumLifetime = 1.0f;
		desc.emissionRate = 0.0f;
		return desc;
	}
}

int main() {
	try {
		using namespace LGF;
		using namespace LGF::VFX;
		ParticleWorldSettings settings;
		settings.maximumParticleCount = 3u;
		settings.maximumEmitterCount = 2u;
		settings.maximumDeltaTime = 1.0f;
		settings.worldBounds = { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } };
		ParticleWorld world{ settings };
		ParticleWorld other{ settings };
		auto desc = MakeEmitter();
		auto emitter = world.CreateEmitter(desc);
		Require(world.IsValid(emitter), "Emitter creation");
		Require(!other.Emit(emitter, 1u), "Cross-world handle rejection");
		Require(world.Emit(emitter, 10u), "Burst request");
		world.Update(0.0f);
		Require(world.GetParticleCount() == 3u, "Particle capacity");
		Require(world.GetLastStepStatistics().rejectedCount == 7u, "Rejected particle statistics");
		Require(world.GetLastStepStatistics().isAvailable &&
			world.GetLastStepStatistics().latencyFrames == 0u, "CPU statistics are immediate");
		world.Update(1.0f);
		Require(world.GetParticleCount() == 0u, "Lifetime cleanup");
		Require(world.GetLastStepStatistics().expiredCount == 3u, "Expired statistics");
		world.Emit(emitter, 3u);
		world.Update(0.0f);
		Require(world.GetParticleCount() == 3u, "Free list reuse");
		Require(world.RemoveEmitter(emitter), "Remove emitter");
		Require(world.GetParticleCount() == 3u, "Emitter removal keeps existing particles");
		const auto replacement = world.CreateEmitter(desc);
		Require(!world.Emit(emitter, 1u) && world.IsValid(replacement), "Generation check after reuse");
		world.Clear();
		Require(!world.IsValid(replacement) && world.GetParticleCount() == 0u, "Clear invalidates handles");

		// AABBの6面、境界上、padding、範囲判定無効化を検証する。
		for (int axis = 0; axis < 3; ++axis) {
			for (float direction : { -1.0f, 1.0f }) {
				world.Clear();
				desc = MakeEmitter();
				if (axis == 0) desc.position.x = direction * 1.01f;
				if (axis == 1) desc.position.y = direction * 1.01f;
				if (axis == 2) desc.position.z = direction * 1.01f;
				emitter = world.CreateEmitter(desc);
				world.Emit(emitter, 1u);
				world.Update(0.0f);
				Require(world.GetLastStepStatistics().outOfBoundsCount == 1u, "AABB face cleanup");
			}
		}
		world.Clear();
		desc = MakeEmitter();
		desc.position = { 1.0f, -1.0f, 1.0f };
		emitter = world.CreateEmitter(desc);
		world.Emit(emitter, 1u);
		world.Update(0.0f);
		Require(world.GetParticleCount() == 1u, "AABB includes its boundary");
		settings.boundsPadding = 0.1f;
		ParticleWorld padded{ settings };
		desc.position.x = 1.05f;
		padded.Emit(padded.CreateEmitter(desc), 1u);
		padded.Update(0.0f);
		Require(padded.GetParticleCount() == 1u, "Bounds padding");
		settings.removeOutOfBounds = false;
		ParticleWorld unbounded{ settings };
		desc.position.x = 1000.0f;
		unbounded.Emit(unbounded.CreateEmitter(desc), 1u);
		unbounded.Update(0.0f);
		Require(unbounded.GetParticleCount() == 1u, "Disabled bounds check");

		world.Clear();
		desc = MakeEmitter();
		desc.minimumVelocity = desc.maximumVelocity = { 2.0f, 0.0f, 0.0f };
		emitter = world.CreateEmitter(desc);
		world.Emit(emitter, 1u);
		world.Update(0.75f);
		Require(world.GetLastStepStatistics().outOfBoundsCount == 1u, "Velocity integration");
		world.Clear();
		desc = MakeEmitter();
		desc.acceleration = { 8.0f, 0.0f, 0.0f };
		emitter = world.CreateEmitter(desc);
		world.Emit(emitter, 1u);
		world.Update(0.75f);
		Require(world.GetLastStepStatistics().outOfBoundsCount == 1u, "Acceleration integration");

		world.Clear();
		desc = MakeEmitter();
		desc.emissionRate = 4.0f;
		emitter = world.CreateEmitter(desc);
		world.Update(0.125f);
		Require(world.GetParticleCount() == 0u, "Fractional emission first half");
		world.Update(0.125f);
		Require(world.GetParticleCount() == 1u, "Fractional emission second half");
		world.SetEmitterEnabled(emitter, false);
		world.Update(0.25f);
		Require(world.GetParticleCount() == 1u, "Disabled continuous emission");
		world.Emit(emitter, 1u);
		world.Update(0.0f);
		Require(world.GetParticleCount() == 2u, "Explicit emission while disabled");
		world.Update(std::numeric_limits<float>::quiet_NaN());
		world.Update(-1.0f);
		Require(world.GetParticleCount() == 2u, "Invalid delta time ignored");
		desc.minimumLifetime = -1.0f;
		Require(!world.CreateEmitter(desc).IsValid(), "Invalid lifetime rejected");
		desc = MakeEmitter();
		desc.minimumVelocity.x = 2.0f;
		Require(!world.SetEmitterDesc(emitter, desc), "Invalid velocity range rejected");
		Require(world.GetEmitterDesc(emitter)->emissionRate == 4.0f, "Failed update preserves configuration");
		Require(!world.SetEmitterPosition(emitter, { INFINITY, 0.0f, 0.0f }), "Invalid position rejected");
		world.CreateEmitter(MakeEmitter());
		Require(!world.CreateEmitter(MakeEmitter()).IsValid(), "Emitter capacity");
		world.Clear();
		emitter = world.CreateEmitter(MakeEmitter());
		world.Emit(emitter, std::numeric_limits<uint32_t>::max());
		world.Update(0.0f);
		Require(world.GetParticleCount() == 3u, "Huge burst is bounded");

		const auto loadedSettings = LoadParticleWorldSettings("Project/Assets/Parameters/ParticleWorld.json");
		const auto loadedEmitter = LoadParticleEmitterDesc("Project/Assets/Parameters/ParticleEmitter.json");
		ParticleWorld configured{ loadedSettings };
		Require(configured.CreateEmitter(loadedEmitter).IsValid(), "Sample JSON configuration");
		Require(loadedEmitter.emissionRate == 300.0f, "JSON emission rate");
		std::cout << "ParticleWorld tests passed.\n";
		return 0;
	}
	catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
}
