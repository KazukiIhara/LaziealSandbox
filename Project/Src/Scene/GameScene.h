#pragma once

#include "Scene/SampleScene.h"

#include <optional>

class GameScene final : public SampleScene {
public:
	explicit GameScene(const InitData& init);

	void OnEnter() override;
	void Update() override;
	void Draw() const override;

private:
	const LGF::Collision::AABB sphereTargetShape_{
		.min = { -8.2f, 0.0f, -1.0f },
		.max = { -5.8f, 3.0f, 1.0f },
	};
	const LGF::Collision::Sphere movableSphereBase_{
		.center = { -7.0f, 1.2f, -3.0f },
		.radius = 0.8f,
	};
	const LGF::Collision::AABB capsuleBoxTargetShape_{
		.min = { -3.3f, 0.0f, -1.0f },
		.max = { -0.7f, 2.5f, 1.0f },
	};
	const LGF::Collision::Capsule movableCapsuleBoxBase_{
		.start = { -2.0f, 0.65f, -3.0f },
		.end = { -2.0f, 1.85f, -3.0f },
		.radius = 0.55f,
	};
	const LGF::Collision::Capsule capsuleTargetShape_{
		.start = { 3.0f, 0.6f, 0.0f },
		.end = { 3.0f, 2.0f, 0.0f },
		.radius = 0.6f,
	};
	const LGF::Collision::Capsule movableCapsuleBase_{
		.start = { 3.0f, 0.6f, -3.0f },
		.end = { 3.0f, 2.0f, -3.0f },
		.radius = 0.6f,
	};
	const LGF::Collision::Capsule rayTargetShape_{
		.start = { 8.0f, 0.6f, 0.0f },
		.end = { 8.0f, 2.0f, 0.0f },
		.radius = 0.6f,
	};
	const LGF::Collision::Ray rayBase_{
		.origin = { 8.0f, 1.3f, -4.0f },
		.direction = { 0.0f, 0.0f, 1.0f },
	};

	LGF::Collision::ColliderWorld world_;
	LGF::Collision::ColliderHandle movableSphere_{};
	LGF::Collision::ColliderHandle sphereTarget_{};
	LGF::Collision::ColliderHandle movableCapsuleBox_{};
	LGF::Collision::ColliderHandle capsuleBoxTarget_{};
	LGF::Collision::ColliderHandle movableCapsule_{};
	LGF::Collision::ColliderHandle capsuleTarget_{};
	LGF::Collision::ColliderHandle rayTarget_{};
	LGF::Vector3 movementOffset_{};
	LGF::Collision::Ray ray_ = rayBase_;
	std::optional<LGF::Collision::WorldRaycastHit> rayHit_;
	bool triggerEnabled_ = true;
};
