#pragma once

#include "Scene/SampleScene.h"

#include <cstddef>

class ColliderWorldScene final : public SampleScene {
public:
	explicit ColliderWorldScene(const InitData& init);

	void Update() override;
	void Draw() const override;

private:
	struct EventCounter {
		std::size_t enter{};
		std::size_t stay{};
		std::size_t exit{};
	};

	void ResetDemo();
	void UpdateEventCounter(
		LGF::Collision::ColliderHandle handle,
		EventCounter& counter);
	void DrawEventInfo(
		const char* label,
		LGF::Collision::ColliderHandle handle,
		const EventCounter& counter) const;

	const LGF::Collision::Sphere oneWayMoverBase_{
		.center = { -5.0f, 0.8f, -4.0f },
		.radius = 0.65f,
	};
	const LGF::Collision::AABB oneWayTargetShape_{
		.min = { -6.0f, 0.0f, -0.8f },
		.max = { -4.0f, 1.6f, 0.8f },
	};
	const LGF::Collision::Sphere twoWayMoverBase_{
		.center = { 0.0f, 0.8f, -4.0f },
		.radius = 0.65f,
	};
	const LGF::Collision::AABB twoWayTargetShape_{
		.min = { -1.0f, 0.0f, -0.8f },
		.max = { 1.0f, 1.6f, 0.8f },
	};
	const LGF::Collision::Sphere filteredMoverBase_{
		.center = { 5.0f, 0.8f, -4.0f },
		.radius = 0.65f,
	};
	const LGF::Collision::AABB ignoredTargetShape_{
		.min = { 4.0f, 0.0f, -2.1f },
		.max = { 6.0f, 1.6f, -0.9f },
	};
	const LGF::Collision::AABB filteredTargetShape_{
		.min = { 4.0f, 0.0f, 1.2f },
		.max = { 6.0f, 1.6f, 2.8f },
	};

	LGF::Collision::ColliderWorld world_;
	LGF::Collision::ColliderHandle oneWayMover_{};
	LGF::Collision::ColliderHandle oneWayTarget_{};
	LGF::Collision::ColliderHandle twoWayMover_{};
	LGF::Collision::ColliderHandle twoWayTarget_{};
	LGF::Collision::ColliderHandle filteredMover_{};
	LGF::Collision::ColliderHandle ignoredTarget_{};
	LGF::Collision::ColliderHandle filteredTarget_{};
	LGF::Vector3 movementOffset_{};
	EventCounter oneWayMoverEvents_{};
	EventCounter oneWayTargetEvents_{};
	EventCounter twoWayMoverEvents_{};
	EventCounter twoWayTargetEvents_{};
	EventCounter filteredMoverEvents_{};
	EventCounter ignoredTargetEvents_{};
	EventCounter filteredTargetEvents_{};
};
