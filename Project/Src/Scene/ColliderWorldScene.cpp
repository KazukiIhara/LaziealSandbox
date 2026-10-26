#include "Scene/ColliderWorldScene.h"

#include <imgui.h>

#include "Scene/SceneNames.h"

using namespace LGF;

namespace {

	constexpr float kMoveSpeed = 3.0f;
	constexpr Vector4 kMoverColor{ 0.1f, 0.8f, 0.9f, 1.0f };
	constexpr Vector4 kTargetColor{ 0.2f, 0.45f, 0.95f, 1.0f };
	constexpr Vector4 kIgnoredColor{ 0.45f, 0.45f, 0.45f, 1.0f };
	constexpr Vector4 kEnterColor{ 1.0f, 0.85f, 0.0f, 1.0f };
	constexpr Vector4 kStayColor{ 1.0f, 0.15f, 0.1f, 1.0f };
	constexpr Vector4 kExitColor{ 1.0f, 0.0f, 0.75f, 1.0f };

	namespace Tags {

		inline constexpr char Mover[] = "Mover";
		inline constexpr char Target[] = "Target";
		inline constexpr char Ignored[] = "Ignored";

	}

	Collision::Sphere Translate(
		const Collision::Sphere& sphere,
		const Vector3& offset) {
		Collision::Sphere result = sphere;
		result.center += offset;
		return result;
	}

	Vector4 GetEventColor(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle,
		const Vector4& idleColor) {
		const Collision::ColliderEvents events = world.GetEvents(handle);
		if (!events.enter.empty()) {
			return kEnterColor;
		}
		if (!events.stay.empty()) {
			return kStayColor;
		}
		if (!events.exit.empty()) {
			return kExitColor;
		}
		return idleColor;
	}

	const char* GetEventName(const Collision::ColliderEvents& events) {
		if (!events.enter.empty()) {
			return "Enter";
		}
		if (!events.stay.empty()) {
			return "Stay";
		}
		if (!events.exit.empty()) {
			return "Exit";
		}
		return "None";
	}

	void DrawShape(
		const Collision::Sphere& sphere,
		const Vector4& color) {
		Sphere3D(sphere.radius).Draw(
			Math::MakeTranslateMatrix(sphere.center),
			color);
	}

	void DrawShape(
		const Collision::AABB& box,
		const Vector4& color) {
		const Vector3 size = box.max - box.min;
		const Vector3 center = (box.min + box.max) * 0.5f;
		Box3D(size.x, size.y, size.z).Draw(
			Math::MakeTranslateMatrix(center),
			color);
	}

	void DrawContacts(
		const Collision::ColliderWorld& world,
		Collision::ColliderHandle handle) {
		const Collision::ColliderEvents events = world.GetEvents(handle);
		for (const std::span<const Collision::ColliderCollision> collisions : {
			events.enter, events.stay }) {
			for (const Collision::ColliderCollision& collision : collisions) {
				Sphere3D(0.1f).Draw(
					Math::MakeTranslateMatrix(collision.contact.point),
					Color::Yellow);
				Line3D(
					collision.contact.point,
					collision.contact.point + collision.contact.normal).Draw(
						Math::Identity4x4(),
						Color::Cyan);
			}
		}
	}

}

ColliderWorldScene::ColliderWorldScene(const InitData& init) :
	SampleScene(init) {
	world_.RegisterTags({
		Tags::Mover,
		Tags::Target,
		Tags::Ignored,
	});

	oneWayMover_ = world_.Add(oneWayMoverBase_, {
		.tag = Tags::Mover,
		.targetTags = { Tags::Target },
	});
	oneWayTarget_ = world_.Add(oneWayTargetShape_, {
		.tag = Tags::Target,
	});
	twoWayMover_ = world_.Add(twoWayMoverBase_, {
		.tag = Tags::Mover,
		.targetTags = { Tags::Target },
	});
	twoWayTarget_ = world_.Add(twoWayTargetShape_, {
		.tag = Tags::Target,
		.targetTags = { Tags::Mover },
	});
	filteredMover_ = world_.Add(filteredMoverBase_, {
		.tag = Tags::Mover,
		.targetTags = { Tags::Target },
	});
	ignoredTarget_ = world_.Add(ignoredTargetShape_, {
		.tag = Tags::Ignored,
	});
	filteredTarget_ = world_.Add(filteredTargetShape_, {
		.tag = Tags::Target,
	});
	ResetDemo();
}

void ColliderWorldScene::Update() {
	const float deltaTime = static_cast<float>(System::DeltaTime());
	Vector3 movement{
		static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
		static_cast<float>(Key::R.Press()) - static_cast<float>(Key::F.Press()),
		static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
	};
	if (Math::LengthSq(movement) > 1.0f) {
		movement = Math::Normalize(movement);
	}
	movementOffset_ += movement * kMoveSpeed * deltaTime;

	if (Key::Space.Trigger()) {
		ResetDemo();
	}
	if (Key::Escape.Trigger()) {
		ChangeScene(SceneNames::Title);
	}

	world_.SetShape(
		oneWayMover_,
		Translate(oneWayMoverBase_, movementOffset_));
	world_.SetShape(
		twoWayMover_,
		Translate(twoWayMoverBase_, movementOffset_));
	world_.SetShape(
		filteredMover_,
		Translate(filteredMoverBase_, movementOffset_));
	world_.Step();

	UpdateEventCounter(oneWayMover_, oneWayMoverEvents_);
	UpdateEventCounter(oneWayTarget_, oneWayTargetEvents_);
	UpdateEventCounter(twoWayMover_, twoWayMoverEvents_);
	UpdateEventCounter(twoWayTarget_, twoWayTargetEvents_);
	UpdateEventCounter(filteredMover_, filteredMoverEvents_);
	UpdateEventCounter(ignoredTarget_, ignoredTargetEvents_);
	UpdateEventCounter(filteredTarget_, filteredTargetEvents_);
	UpdateCamera();
}

void ColliderWorldScene::Draw() const {
	DrawStage();
	DrawShape(
		Translate(oneWayMoverBase_, movementOffset_),
		GetEventColor(world_, oneWayMover_, kMoverColor));
	DrawShape(
		oneWayTargetShape_,
		GetEventColor(world_, oneWayTarget_, kTargetColor));
	DrawShape(
		Translate(twoWayMoverBase_, movementOffset_),
		GetEventColor(world_, twoWayMover_, kMoverColor));
	DrawShape(
		twoWayTargetShape_,
		GetEventColor(world_, twoWayTarget_, kTargetColor));
	DrawShape(
		Translate(filteredMoverBase_, movementOffset_),
		GetEventColor(world_, filteredMover_, kMoverColor));
	DrawShape(
		ignoredTargetShape_,
		GetEventColor(world_, ignoredTarget_, kIgnoredColor));
	DrawShape(
		filteredTargetShape_,
		GetEventColor(world_, filteredTarget_, kTargetColor));

	DrawContacts(world_, oneWayMover_);
	DrawContacts(world_, oneWayTarget_);
	DrawContacts(world_, twoWayMover_);
	DrawContacts(world_, twoWayTarget_);
	DrawContacts(world_, filteredMover_);
	DrawContacts(world_, ignoredTarget_);
	DrawContacts(world_, filteredTarget_);

	ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("ColliderWorld Tag Demo")) {
		ImGui::TextUnformatted("Registered tags:");
		for (const Collision::CollisionTag& tag : world_.GetTags()) {
			ImGui::BulletText("%s", tag.c_str());
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Left lane: one-way notification");
		ImGui::TextUnformatted("Mover targets Target. Target targets nothing.");
		DrawEventInfo("Mover", oneWayMover_, oneWayMoverEvents_);
		DrawEventInfo("Target", oneWayTarget_, oneWayTargetEvents_);

		ImGui::Separator();
		ImGui::TextUnformatted("Center lane: two-way notification");
		ImGui::TextUnformatted("Both colliders target each other's tag.");
		DrawEventInfo("Mover", twoWayMover_, twoWayMoverEvents_);
		DrawEventInfo("Target", twoWayTarget_, twoWayTargetEvents_);

		ImGui::Separator();
		ImGui::TextUnformatted("Right lane: tag filtering");
		ImGui::TextUnformatted("The gray Ignored box never creates an event.");
		DrawEventInfo("Mover", filteredMover_, filteredMoverEvents_);
		DrawEventInfo("Ignored", ignoredTarget_, ignoredTargetEvents_);
		DrawEventInfo("Target", filteredTarget_, filteredTargetEvents_);

		ImGui::Separator();
		ImGui::TextUnformatted("WASD: move spheres on XZ plane | R/F: vertical");
		ImGui::TextUnformatted("Space: reset | Escape: title");
		ImGui::TextColored(
			ImVec4(1.0f, 0.85f, 0.0f, 1.0f),
			"Yellow: Enter");
		ImGui::TextColored(
			ImVec4(1.0f, 0.15f, 0.1f, 1.0f),
			"Red: Stay");
		ImGui::TextColored(
			ImVec4(1.0f, 0.0f, 0.75f, 1.0f),
			"Magenta: Exit");
	}
	ImGui::End();
}

void ColliderWorldScene::ResetDemo() {
	movementOffset_ = {};
	world_.SetShape(oneWayMover_, oneWayMoverBase_);
	world_.SetShape(twoWayMover_, twoWayMoverBase_);
	world_.SetShape(filteredMover_, filteredMoverBase_);
	// 位置を戻したことで発生するExitも消化し、初期状態から再開する。
	world_.Step();
	world_.Step();

	oneWayMoverEvents_ = {};
	oneWayTargetEvents_ = {};
	twoWayMoverEvents_ = {};
	twoWayTargetEvents_ = {};
	filteredMoverEvents_ = {};
	ignoredTargetEvents_ = {};
	filteredTargetEvents_ = {};
}

void ColliderWorldScene::UpdateEventCounter(
	Collision::ColliderHandle handle,
	EventCounter& counter) {
	const Collision::ColliderEvents events = world_.GetEvents(handle);
	counter.enter += events.enter.size();
	counter.stay += events.stay.size();
	counter.exit += events.exit.size();
}

void ColliderWorldScene::DrawEventInfo(
	const char* label,
	Collision::ColliderHandle handle,
	const EventCounter& counter) const {
	const Collision::ColliderEvents events = world_.GetEvents(handle);
	ImGui::Text(
		"%s: %-5s | Enter %zu  Stay %zu  Exit %zu",
		label,
		GetEventName(events),
		counter.enter,
		counter.stay,
		counter.exit);
}
